#pragma once
#include <thread>
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumFactory.h>
#include <CoriumDiagnostics.h>
#include <CoriumRuntime.h>
#include <AtomicVariable.h>

class CriticalSectionTryLockFailsWhileHeldByOtherThread final
	: public Hades::Runtime::IFixture<CriticalSectionTryLockFailsWhileHeldByOtherThread, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::CriticalSection m_CS;
	Corium::Core::Atomic::AtomicValue32<bool> m_TryLockResult{ true };
	Corium::Core::Atomic::AtomicValue32<bool> m_ThreadStarted{ false };
	Corium::Core::Factory::DefaultThreadFactory m_Factory;
	Corium::Core::ThreadHandle m_Handle{};
	bool m_JoinSuccess{ false };
	bool m_SpawnedTryLockSuccess{ false };

public:
	explicit CriticalSectionTryLockFailsWhileHeldByOtherThread(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_TryLockResult.store(true, Corium::Core::Atomics::MemoryOrder::RELEASE);
		m_ThreadStarted.store(false, Corium::Core::Atomics::MemoryOrder::RELEASE);
		m_CS.lock();
	}

	void executeImpl() noexcept {
		m_Handle = m_Factory.createAndStart(Corium::Core::createClosure<void()>([this]() {
			m_ThreadStarted.store(true, Corium::Core::Atomics::MemoryOrder::RELEASE);
			bool res = m_CS.tryLock();
			m_TryLockResult.store(res, Corium::Core::Atomics::MemoryOrder::RELEASE);
			if (res) {
				m_CS.unlock();
			}
		}), "TryLockWorker");

		while (!m_ThreadStarted.load(Corium::Core::Atomics::MemoryOrder::ACQUIRE)) {
			std::this_thread::yield();
		}

		m_JoinSuccess = Corium::Core::NativeThread::joinThread(m_Handle);
		CORIUM_ASSERT(m_JoinSuccess);

		m_SpawnedTryLockSuccess = m_TryLockResult.load(Corium::Core::Atomics::MemoryOrder::ACQUIRE);
		CORIUM_ASSERT(!m_SpawnedTryLockSuccess);

		m_CS.unlock();
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

