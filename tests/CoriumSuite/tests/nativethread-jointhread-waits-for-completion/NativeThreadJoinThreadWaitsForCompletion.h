#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumRuntime.h>
#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumDiagnostics.h>
#include <CoriumChrono.h>
#include <AtomicVariable.h>
#include <thread>
#include <chrono>

class NativeThreadJoinThreadWaitsForCompletion final
	: public Hades::Runtime::IFixture<NativeThreadJoinThreadWaitsForCompletion, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Core::Atomic::AtomicValue32<bool> m_Flag{ false };
	Corium::Core::Factory::DefaultThreadFactory m_Factory{};
	Corium::Core::ThreadHandle m_Handle{};
	bool m_Joined{ false };

public:
	explicit NativeThreadJoinThreadWaitsForCompletion(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_Flag.store(false);
		m_Joined = false;
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;

		m_Handle = m_Factory.createAndStart(
			createClosure<void()>([this]() {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				m_Flag.store(true, Atomics::MemoryOrder::RELEASE);
			}),
			"JoinWaitThread"
		);

		m_Joined = NativeThread::joinThread(m_Handle);
		CORIUM_ASSERT(m_Joined && "NativeThread::joinThread failed");
		CORIUM_ASSERT(m_Flag.load(Atomics::MemoryOrder::ACQUIRE) && "Flag was not set before joinThread returned");
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

