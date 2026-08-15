#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumDiagnostics.h>
#include <CoriumRuntime.h>
#include <atomic>

class ConditionSignalWakesSingleAwaitingThread final
	: public Hades::Runtime::IFixture<ConditionSignalWakesSingleAwaitingThread, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::ReentrantLock m_Lock;
	Corium::Runtime::Sync::Condition m_Cond{ m_Lock };
	std::atomic<bool> m_ThreadStarted{ false };
	std::atomic<bool> m_Awakened{ false };
	Corium::Core::Factory::DefaultThreadFactory m_Factory;
	Corium::Core::ThreadHandle m_Handle{};
	bool m_Joined{ false };

public:
	explicit ConditionSignalWakesSingleAwaitingThread(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();

		m_ThreadStarted.store(false, std::memory_order_relaxed);
		m_Awakened.store(false, std::memory_order_relaxed);

		m_Handle = m_Factory.createAndStart(
			Corium::Core::createClosure<void()>([this]() {
				m_Lock.lock();
				m_ThreadStarted.store(true, std::memory_order_release);
				m_Cond.await();
				m_Awakened.store(true, std::memory_order_release);
				m_Lock.unlock();
			}),
			"ConditionWaiter"
		);

		while (!m_ThreadStarted.load(std::memory_order_acquire)) {
			// Wait for spawned thread to start and acquire lock
		}
	}

	void executeImpl() noexcept {
		// Acquiring lock ensures spawned thread has entered cond.await() (which releases lock)
		m_Lock.lock();
		m_Cond.signal();
		m_Lock.unlock();

		m_Joined = Corium::Core::NativeThread::joinThread(m_Handle);
		CORIUM_ASSERT(m_Joined && "NativeThread::joinThread failed");
		CORIUM_ASSERT(m_Awakened.load(std::memory_order_acquire) && "Spawned thread was not awakened by signal()");
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

