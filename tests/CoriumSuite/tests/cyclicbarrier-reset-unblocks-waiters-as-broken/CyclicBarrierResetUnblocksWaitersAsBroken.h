#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumFactory.h>
#include <CoriumDiagnostics.h>
#include <CoriumRuntime.h>
#include <AtomicVariable.h>

class CyclicBarrierResetUnblocksWaitersAsBroken final
	: public Hades::Runtime::IFixture<CyclicBarrierResetUnblocksWaitersAsBroken, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::CyclicBarrier m_barrier{ 2 };
	Corium::Core::Atomic::AtomicValue32<bool> m_threadStarted{ false };
	Corium::Core::Atomic::AtomicValue32<bool> m_threadReturned{ false };
	Corium::Core::Factory::DefaultThreadFactory m_factory{};
	Corium::Core::ThreadHandle m_t1{};
	bool m_joinSuccess{ false };

public:
	explicit CyclicBarrierResetUnblocksWaitersAsBroken(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_threadStarted.store(false, Corium::Core::Atomics::MemoryOrder::RELAXED);
		m_threadReturned.store(false, Corium::Core::Atomics::MemoryOrder::RELAXED);
		m_t1 = {};
		m_joinSuccess = false;
	}

	void executeImpl() noexcept {
		m_t1 = m_factory.createAndStart(Corium::Core::createClosure<void()>([this]() {
			m_threadStarted.store(true, Corium::Core::Atomics::MemoryOrder::RELEASE);
			m_barrier.await();
			m_threadReturned.store(true, Corium::Core::Atomics::MemoryOrder::RELEASE);
		}), "BarrierResetWaitThread");

		// Bounded poll until spawned thread has started and called await()
		while (!m_threadStarted.load(Corium::Core::Atomics::MemoryOrder::ACQUIRE)) {
			Corium::Intrinsic::Pause();
		}
		while (m_barrier.getNumberWaiting() < 1) {
			Corium::Intrinsic::Pause();
		}

		m_barrier.reset();

		m_joinSuccess = Corium::Core::NativeThread::joinThread(m_t1);

		CORIUM_ASSERT(m_joinSuccess);
		CORIUM_ASSERT(m_threadReturned.load(Corium::Core::Atomics::MemoryOrder::ACQUIRE));
		CORIUM_ASSERT(m_barrier.isBroken());
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

