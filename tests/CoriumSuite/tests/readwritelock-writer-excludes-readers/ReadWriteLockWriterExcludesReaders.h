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

class ReadWriteLockWriterExcludesReaders final
	: public Hades::Runtime::IFixture<ReadWriteLockWriterExcludesReaders, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit ReadWriteLockWriterExcludesReaders(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();

		m_RwLock = std::make_unique<Corium::Runtime::Sync::ReadWriteLock>();
		m_ReaderAcquired = std::make_unique<std::atomic<bool>>(false);
		m_Factory = std::make_unique<Corium::Core::Factory::DefaultThreadFactory>();
	}

	void executeImpl() noexcept {
		// Main thread acquires write lock
		m_RwLock->lockWrite();

		// Assert main thread holds write lock
		CORIUM_ASSERT(m_RwLock->isWriteLocked() && "isWriteLocked should be true when write lock is held");

		// Spawned thread attempts to acquire read lock while write lock is held
		Corium::Core::ThreadHandle handle = m_Factory->createAndStart(
			Corium::Core::createClosure<void()>([this]() {
				bool success = m_RwLock->tryLockRead();
				if (success) {
					m_ReaderAcquired->store(true, std::memory_order_release);
					m_RwLock->unlockRead();
				}
			}),
			"ReaderExclusionCheck"
		);

		bool joined = Corium::Core::NativeThread::joinThread(handle);
		CORIUM_ASSERT(joined && "NativeThread::joinThread failed");

		// Assert that the spawned thread failed to acquire the read lock
		CORIUM_ASSERT(!m_ReaderAcquired->load(std::memory_order_acquire) && "Spawned thread acquired read lock while write lock was held");

		// Unlock write lock
		m_RwLock->unlockWrite();

		CORIUM_ASSERT(!m_RwLock->isWriteLocked() && "isWriteLocked should be false after unlocking write lock");
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
	std::unique_ptr<std::atomic<bool>> m_ReaderAcquired;
	std::unique_ptr<Corium::Core::Factory::DefaultThreadFactory> m_Factory;
};
