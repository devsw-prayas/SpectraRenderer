#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumFrame.h>
#include <CoriumDiagnostics.h>
#include <CoriumRuntime.h>
#include <Atomics.h>
#include <atomic>
#include <thread>
#include <intrin.h>

class TestFixture62 final
	: public Hades::Runtime::IFixture<TestFixture62, Hades::Runtime::NullDeviceAdapter> {
private:
	alignas(64) static inline std::byte s_Stack[256 * 1024];
	static inline std::atomic<int> s_Yielded{ 0 };
	static inline std::atomic<int> s_Migrated{ -1 };
	static inline std::atomic<int> s_CanaryOk{ -1 };
	static inline std::atomic<int> s_Terminated{ 0 };

public:
	explicit TestFixture62(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;

		s_Yielded.store(0);
		s_Migrated.store(-1);
		s_CanaryOk.store(-1);
		s_Terminated.store(0);

		Frame::FrameStackDesc desc{};
		Frame::init(desc);
		Frame::setProvenance(desc, Frame::Provenance::CALLEE_OWNED);
		Frame::setMemoryLocation(desc, s_Stack);
		Frame::setStackSize(desc, sizeof(s_Stack));
		CORIUM_ASSERT(Frame::validate(desc));

		auto entry = Utils::makeClosure<void()>([]() {
			const std::thread::id before = std::this_thread::get_id();
			volatile uint64_t canary = 0x5EC74A11C0DEF00Dull;
			Frame::NativeFrame::yieldToNative(Frame::this_frame::currentFrame(), false);
			s_Migrated.store(std::this_thread::get_id() != before ? 1 : 0);
			s_CanaryOk.store(canary == 0x5EC74A11C0DEF00Dull ? 1 : 0);
		}, 0);
		auto created = Frame::NativeFrame::createFrame(std::move(entry), desc);
		CORIUM_ASSERT(created.has_value());
		Frame::FrameHandle* frame = created.value();

		Factory::DefaultThreadFactory factory{};
		const ThreadHandle a = factory.createAndStart(makeClosure<void()>([frame]() {
			Frame::this_frame::captureFrame(frame);
			s_Yielded.store(1, std::memory_order_release);
		}), "MigrateA");
		const ThreadHandle b = factory.createAndStart(makeClosure<void()>([frame]() {
			while (s_Yielded.load(std::memory_order_acquire) == 0) std::this_thread::yield();
			while (!Frame::this_frame::tryCaptureFrame(frame)) _mm_pause();
			const Frame::FrameState state = Corium::Atomics::load<Corium::Atomics::MemoryOrder::ACQUIRE>(&frame->m_State);
			s_Terminated.store(state == Frame::FrameState::TERMINATED ? 1 : 0);
		}), "MigrateB");

		CORIUM_ASSERT(NativeThread::joinThread(a) && NativeThread::joinThread(b));
		CORIUM_ASSERT(s_Migrated.load() == 1 && "The frame must resume on a different thread");
		CORIUM_ASSERT(s_CanaryOk.load() == 1 && "The frame's stack must survive the hop");
		CORIUM_ASSERT(s_Terminated.load() == 1 && "The frame must read TERMINATED after running to completion");
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return 0;
	}
};
