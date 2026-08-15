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
#include <memory>

class ReadWriteLockAllowsConcurrentReaders final
	: public Hades::Runtime::IFixture<ReadWriteLockAllowsConcurrentReaders, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit ReadWriteLockAllowsConcurrentReaders(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();

		m_RwLock = std::make_unique<Corium::Runtime::Sync::ReadWriteLock>();
		m_ThreadAcquiredRead = std::make_unique<std::atomic<bool>>(false);
		m_Factory = std::make_unique<Corium::Core::Factory::DefaultThreadFactory>();
	}

	void executeImpl() noexcept {
		// Main thread acquires read lock first
		m_RwLock->lockRead();

		// Spawned thread attempts to acquire read lock concurrently
		Corium::Core::ThreadHandle handle = m_Factory->createAndStart(
			Corium::Core::createClosure<void()>([this]() {
				bool success = m_RwLock->tryLockRead();
				if (success) {
					m_ThreadAcquiredRead->store(true, std::memory_order_release);
					m_RwLock->unlockRead();
				}
			}),
			"ReadLockReader"
		);

		bool joined = Corium::Core::NativeThread::joinThread(handle);
		CORIUM_ASSERT(joined && "NativeThread::joinThread failed");
		CORIUM_ASSERT(m_ThreadAcquiredRead->load(std::memory_order_acquire) && "Spawned thread failed to acquire concurrent read lock");

		// Verify count while main thread still holds read lock (after spawned thread unlocked)
		CORIUM_ASSERT(m_RwLock->getReadLockCount() == 1 && "Expected read lock count to be 1 after spawned thread unlocked");

		m_RwLock->unlockRead();

		CORIUM_ASSERT(m_RwLock->getReadLockCount() == 0 && "Expected read lock count to be 0 after main thread unlocked");
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
	}

	void teardownImpl() noexcept {
	}

	uint64_t getDeterminismHashImpl() noexcept {
		return 0;
	}

private:
	std::unique_ptr<Corium::Runtime::Sync::ReadWriteLock> m_RwLock;
	std::unique_ptr<std::atomic<bool>> m_ThreadAcquiredRead;
	std::unique_ptr<Corium::Core::Factory::DefaultThreadFactory> m_Factory;
};

