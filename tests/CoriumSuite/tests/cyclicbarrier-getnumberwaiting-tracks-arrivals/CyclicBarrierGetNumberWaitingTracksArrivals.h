#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumFactory.h>
#include <CoriumDiagnostics.h>
#include <CoriumChrono.h>
#include <CoriumRuntime.h>
#include <AtomicVariable.h>
#include <ThreadUtils.h>

#include <thread>

class CyclicBarrierGetNumberWaitingTracksArrivals final
	: public Hades::Runtime::IFixture<CyclicBarrierGetNumberWaitingTracksArrivals, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::CyclicBarrier m_barrier{ 3 };
	Corium::Core::Factory::DefaultThreadFactory m_factory{};
	Corium::Core::Atomic::AtomicValue32<bool> m_t1Arrived{ false };
	Corium::Core::ThreadHandle m_t1{};
	Corium::Core::ThreadHandle m_t2{};
	bool m_join1Success{ false };
	bool m_join2Success{ false };

public:
	explicit CyclicBarrierGetNumberWaitingTracksArrivals(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_t1Arrived.store(false, Corium::Core::Atomics::MemoryOrder::RELAXED);
		m_t1 = {};
		m_t2 = {};
		m_join1Success = false;
		m_join2Success = false;
	}

	void executeImpl() noexcept {
		using namespace Corium::Core::Chrono::Literals;

		CORIUM_ASSERT(m_barrier.getParties() == 3);
		CORIUM_ASSERT(m_barrier.getNumberWaiting() == 0);

		m_t1 = m_factory.createAndStart(Corium::Core::createClosure<void()>([this]() {
			m_t1Arrived.store(true, Corium::Core::Atomics::MemoryOrder::RELEASE);
			m_barrier.await();
		}), "BarrierParty1");

		m_t2 = m_factory.createAndStart(Corium::Core::createClosure<void()>([this]() {
			while (!m_t1Arrived.load(Corium::Core::Atomics::MemoryOrder::ACQUIRE)) {
				std::this_thread::yield();
			}

			uint32_t waiting = 0;
			auto deadline = Corium::Core::Chrono::until(1000_ms);
			while (!deadline.isExpired()) {
				waiting = m_barrier.getNumberWaiting();
				if (waiting == 1) {
					break;
				}
				std::this_thread::yield();
			}

			CORIUM_ASSERT(waiting == 1);

			m_barrier.await();
		}), "BarrierParty2");

		m_barrier.await();

		m_join1Success = Corium::Core::NativeThread::joinThread(m_t1);
		m_join2Success = Corium::Core::NativeThread::joinThread(m_t2);

		CORIUM_ASSERT(m_join1Success);
		CORIUM_ASSERT(m_join2Success);
		CORIUM_ASSERT(m_barrier.getNumberWaiting() == 0);
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

