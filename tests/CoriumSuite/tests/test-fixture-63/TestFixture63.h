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
#include <atomic>

class TestFixture63 final
	: public Hades::Runtime::IFixture<TestFixture63, Hades::Runtime::NullDeviceAdapter> {
private:
	struct PoisonOnDestroy {
		uint64_t m_Value;
		~PoisonOnDestroy() { m_Value = 0xDEADDEADDEADDEADull; }
	};

	static inline std::atomic<uint64_t> s_Seen{ 0 };

	static void runOn(Corium::Core::Frame::FrameHandle* p_Frame) {
		using namespace Corium::Core;
		Factory::DefaultThreadFactory factory{};
		CORIUM_UNUSED(NativeThread::joinThread(factory.createAndStart(makeClosure<void()>([p_Frame]() {
			Frame::this_frame::captureFrame(p_Frame);
		}), "CaptureCheck")));
	}

public:
	explicit TestFixture63(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;

		Frame::FrameStackDesc desc{};
		Frame::init(desc);
		CORIUM_ASSERT(Frame::validate(desc));

		volatile uint64_t va = 0x1111222233334444ull, vb = 0x5555666677778888ull;
		const uint64_t a = va, b = vb;
		s_Seen.store(0);

		auto named = Utils::makeClosure<void()>([a, b]() { s_Seen.store(a ^ b); }, 0);
		runOn(Frame::NativeFrame::createFrame(std::move(named), desc).value());
		CORIUM_ASSERT(s_Seen.exchange(0) == (a ^ b) && "Named closure");

		auto plain = Frame::NativeFrame::createFrame(Utils::makeClosure<void()>([a, b]() { s_Seen.store(a ^ b); }, 0), desc);
		runOn(plain.value());
		CORIUM_ASSERT(s_Seen.exchange(0) == (a ^ b) && "A temporary closure must outlive its statement");

		auto withDtor = Frame::NativeFrame::createFrame(Utils::makeClosure<void()>([p = PoisonOnDestroy{ a }]() { s_Seen.store(p.m_Value); }, 0), desc);
		runOn(withDtor.value());
		CORIUM_ASSERT(s_Seen.exchange(0) == a && "A closure with a destructor must not be destroyed before the frame runs");
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
