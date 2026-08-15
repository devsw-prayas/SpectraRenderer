#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumFactory.h>
#include <CoriumDiagnostics.h>
#include <AtomicVariable.h>
#include <CoriumRuntime.h>

class CyclicBarrierAwaitReleasesAllParties final
	: public Hades::Runtime::IFixture<CyclicBarrierAwaitReleasesAllParties, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::CyclicBarrier m_Barrier{ 2 };
	Corium::Core::Atomic::AtomicValue32<uint32_t> m_ArrivedCount{ 0 };
	Corium::Core::Factory::DefaultThreadFactory m_Factory;
	Corium::Core::ThreadHandle m_T1{};
	Corium::Core::ThreadHandle m_T2{};
	bool m_Join1Success{ false };
	bool m_Join2Success{ false };

public:
	explicit CyclicBarrierAwaitReleasesAllParties(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_ArrivedCount.store(0, Corium::Core::Atomics::MemoryOrder::RELAXED);
	}

	void executeImpl() noexcept {
		m_T1 = m_Factory.createAndStart(Corium::Core::createClosure<void()>([this]() {
			m_ArrivedCount.fetchAdd(1, Corium::Core::Atomics::MemoryOrder::RELAXED);
			m_Barrier.await();
		}), "BarrierParty1");

		m_T2 = m_Factory.createAndStart(Corium::Core::createClosure<void()>([this]() {
			m_ArrivedCount.fetchAdd(1, Corium::Core::Atomics::MemoryOrder::RELAXED);
			m_Barrier.await();
		}), "BarrierParty2");

		m_Join1Success = Corium::Core::NativeThread::joinThread(m_T1);
		m_Join2Success = Corium::Core::NativeThread::joinThread(m_T2);

		CORIUM_ASSERT(m_Join1Success);
		CORIUM_ASSERT(m_Join2Success);
		CORIUM_ASSERT(m_ArrivedCount.load(Corium::Core::Atomics::MemoryOrder::RELAXED) == 2);
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

