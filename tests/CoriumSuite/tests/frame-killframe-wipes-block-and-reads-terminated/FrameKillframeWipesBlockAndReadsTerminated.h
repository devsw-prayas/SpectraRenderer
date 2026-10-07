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

// killFrame on a suspended frame wipes the whole block (guard page included), never resumes it, and leaves a TERMINATED handle.
class FrameKillframeWipesBlockAndReadsTerminated final
	: public Hades::Runtime::IFixture<FrameKillframeWipesBlockAndReadsTerminated, Hades::Runtime::NullDeviceAdapter> {
private:
#define FRAME_KILL_CHECK(c) do { if (!(c)) { std::printf("FRAME CHECK FAILED line %d: %s\n", __LINE__, #c); ++fails; } } while (0)

	static int run() noexcept {
		using namespace Corium::Core;
		using Frame::FrameState;
		using Frame::NativeFrame;

		int fails = 0;
		Frame::FrameHandle* frame = nullptr;
		int stage = 0;

		auto closure = makeClosure<void()>([&]() {
			stage = 1;
			// Leave real data on the frame's stack so the wipe has something to remove.
			CORIUM_ALIGNAS(16) volatile char scratch[2048];
			for (size_t i = 0; i < sizeof(scratch); ++i) scratch[i] = static_cast<char>(0xA5);
			NativeFrame::yieldToNative(frame, false);
			stage = 2;   // must never run: the frame is killed while suspended
		});

		Frame::FrameStackDesc desc;
		Frame::init(desc);
		Frame::enableGuardPages(desc, true);
		FRAME_KILL_CHECK(Frame::validate(desc));

		auto created = NativeFrame::createFrame(std::move(closure), desc);
		FRAME_KILL_CHECK(created.has_value());
		if (!created) return fails + 1;
		frame = *created;

		Frame::FrameHandle* native = std::addressof(Frame::this_thread::nativeContext());
		NativeFrame::switchTo(native, frame, false);
		FRAME_KILL_CHECK(stage == 1);
		FRAME_KILL_CHECK(frame->m_State == FrameState::SUSPENDED);

		const auto* base = reinterpret_cast<const uint8_t*>(frame);
		const size_t blockSize = frame->m_StackSize;

		NativeFrame::killFrame(frame);

		size_t blockNonzero = 0;
		for (size_t i = sizeof(Frame::FrameHandle); i < blockSize; ++i) blockNonzero += base[i] != 0;
		size_t handleNonzero = 0;
		for (size_t i = 0; i < sizeof(Frame::FrameHandle); ++i) handleNonzero += base[i] != 0;

		FRAME_KILL_CHECK(blockNonzero == 0);
		FRAME_KILL_CHECK(handleNonzero == 1);                  // only m_State, which now reads TERMINATED
		FRAME_KILL_CHECK(frame->m_State == FrameState::TERMINATED);
		FRAME_KILL_CHECK(stage == 1);
		FRAME_KILL_CHECK(Frame::this_thread::currentFrame() == native);
		return fails;
	}
#undef FRAME_KILL_CHECK

public:
	explicit FrameKillframeWipesBlockAndReadsTerminated(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		int fails = 0;
		Corium::Core::Factory::DefaultThreadFactory threadFactory;
		Corium::Core::ThreadHandle worker = threadFactory.createAndStart(
			Corium::Core::makeClosure<void()>([&fails]() { fails = run(); }), "FrameKillWorker");
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
