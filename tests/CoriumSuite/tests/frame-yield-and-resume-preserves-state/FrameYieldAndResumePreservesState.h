#pragma once
// TODO: not run yet.
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumFactory.h>
#include <CoriumFrame.h>
#include <CoriumRuntime.h>
#include <CoriumThread.h>

#include <cstdio>
#include <xmmintrin.h>

// A frame that yields mid-body must resume at the same instruction with its registers, stack and MXCSR intact.
class FrameYieldAndResumePreservesState final
	: public Hades::Runtime::IFixture<FrameYieldAndResumePreservesState, Hades::Runtime::NullDeviceAdapter> {
private:
#define FRAME_YIELD_CHECK(c) do { if (!(c)) { std::printf("FRAME CHECK FAILED line %d: %s\n", __LINE__, #c); ++fails; } } while (0)

	// Touches about 30 KiB of the frame's own stack, so a wrong stack top or bounds shows up as a fault.
	static CORIUM_NOINLINE bool deepStack(int v_Depth) noexcept {
		CORIUM_ALIGNAS(16) volatile char pad[512];
		pad[0] = static_cast<char>(v_Depth);
		pad[511] = static_cast<char>(v_Depth + 1);
		const bool intact = pad[0] == static_cast<char>(v_Depth) && pad[511] == static_cast<char>(v_Depth + 1);
		return v_Depth == 0 ? intact : intact && deepStack(v_Depth - 1);
	}

	static int run() noexcept {
		using namespace Corium::Core;
		using Frame::FrameState;
		using Frame::NativeFrame;

		int fails = 0;
		Frame::FrameHandle* frame = nullptr;
		int stage = 0;
		bool currentInside = false;
		unsigned mxcsrInside = 0;
		float sum = 0.0f;
		bool deepOk = false;

		auto closure = makeClosure<void()>([&]() {
			stage = 1;
			currentInside = Frame::this_thread::currentFrame() == frame;
			mxcsrInside = _mm_getcsr();

			CORIUM_ALIGNAS(16) float values[4] = { 1, 2, 3, 4 };
			__m128 squares = _mm_mul_ps(_mm_load_ps(values), _mm_load_ps(values));

			NativeFrame::yieldToNative(frame, false);

			stage = 2;
			CORIUM_ALIGNAS(16) float out[4];
			_mm_store_ps(out, squares);
			sum = out[0] + out[1] + out[2] + out[3];
			deepOk = deepStack(60);
		});

		Frame::FrameStackDesc desc;
		Frame::init(desc);
		Frame::enableGuardPages(desc, true);
		FRAME_YIELD_CHECK(Frame::validate(desc));

		auto created = NativeFrame::createFrame(std::move(closure), desc);
		FRAME_YIELD_CHECK(created.has_value());
		if (!created) return fails + 1;
		frame = *created;

		Frame::FrameHandle* native = std::addressof(Frame::this_thread::nativeContext());
		FRAME_YIELD_CHECK(frame->m_State == FrameState::READY);

		NativeFrame::switchTo(native, frame, false);
		FRAME_YIELD_CHECK(stage == 1);
		FRAME_YIELD_CHECK(currentInside);
		FRAME_YIELD_CHECK(mxcsrInside == 0x1F80);
		FRAME_YIELD_CHECK(Frame::this_thread::currentFrame() == native);
		FRAME_YIELD_CHECK(frame->m_State == FrameState::SUSPENDED);

		NativeFrame::switchTo(native, frame, false);
		FRAME_YIELD_CHECK(stage == 2);
		FRAME_YIELD_CHECK(sum == 30.0f);
		FRAME_YIELD_CHECK(deepOk);
		FRAME_YIELD_CHECK(Frame::this_thread::currentFrame() == native);
		// A finished pool frame hands its block back, so its handle is invalid from here on and is not read again.
		return fails;
	}
#undef FRAME_YIELD_CHECK

public:
	explicit FrameYieldAndResumePreservesState(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		int fails = 0;
		Corium::Core::Factory::DefaultThreadFactory threadFactory;
		Corium::Core::ThreadHandle worker = threadFactory.createAndStart(
			Corium::Core::makeClosure<void()>([&fails]() { fails = run(); }), "FrameYieldWorker");
		Corium::Core::NativeThread::joinThread(worker);
		if (fails != 0) CORIUM_TRAP();
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		CORIUM_UNUSED(ro_Adapter);
	}

	void teardownImpl() noexcept {
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return 0;
	}
};
