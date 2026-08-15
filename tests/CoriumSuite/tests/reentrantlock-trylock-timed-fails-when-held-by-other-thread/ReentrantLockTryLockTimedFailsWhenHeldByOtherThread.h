#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumFactory.h>
#include <CoriumThread.h>
#include <CoriumDiagnostics.h>
#include <CoriumChrono.h>
#include <AtomicVariable.h>
#include <thread>

#include <CoriumRuntime.h>

class ReentrantLockTryLockTimedFailsWhenHeldByOtherThread final
	: public Hades::Runtime::IFixture<ReentrantLockTryLockTimedFailsWhenHeldByOtherThread, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::ReentrantLock m_Lock;
	Corium::Core::Atomic::AtomicValue32<bool> m_TryLockResult{ true };
	Corium::Core::Atomic::AtomicValue32<bool> m_ThreadStarted{ false };
	Corium::Core::Factory::DefaultThreadFactory m_Factory;
	Corium::Core::ThreadHandle m_Handle{};
	bool m_JoinSuccess{ false };
	bool m_SpawnedTryLockSuccess{ false };

public:
	explicit ReentrantLockTryLockTimedFailsWhenHeldByOtherThread(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_TryLockResult.store(true, Corium::Core::Atomics::MemoryOrder::RELAXED);
		m_ThreadStarted.store(false, Corium::Core::Atomics::MemoryOrder::RELAXED);
	}

	void executeImpl() noexcept {
		using namespace Corium::Core::Chrono::Literals;

		m_Lock.lock();

		m_Handle = m_Factory.createAndStart(Corium::Core::createClosure<void()>([this]() {
			m_ThreadStarted.store(true, Corium::Core::Atomics::MemoryOrder::RELEASE);
			bool res = m_Lock.tryLock(Corium::Core::Chrono::until(1_ms));
			m_TryLockResult.store(res, Corium::Core::Atomics::MemoryOrder::RELEASE);
			if (res) {
				m_Lock.unlock();
			}
		}), "TryLockTimedWorker");

		while (!m_ThreadStarted.load(Corium::Core::Atomics::MemoryOrder::ACQUIRE)) {
			std::this_thread::yield();
		}

		m_JoinSuccess = Corium::Core::NativeThread::joinThread(m_Handle);
		CORIUM_ASSERT(m_JoinSuccess);

		m_SpawnedTryLockSuccess = m_TryLockResult.load(Corium::Core::Atomics::MemoryOrder::ACQUIRE);
		CORIUM_ASSERT(!m_SpawnedTryLockSuccess);

		m_Lock.unlock();
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


