#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumEnvironment.h>

class StampedLockTryConvertToWriteLockSucceedsWhenUncontended final
	: public Hades::Runtime::IFixture<StampedLockTryConvertToWriteLockSucceedsWhenUncontended, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::StampedLock m_Lock;
	uint64_t m_Rs{ 0 };
	uint64_t m_Ws{ 0 };

public:
	explicit StampedLockTryConvertToWriteLockSucceedsWhenUncontended(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		// TODO: PCIe host->device transfers (GPU fixtures) belong here.
	}

	void executeImpl() noexcept {
		m_Rs = m_Lock.readLock();
		CORIUM_ASSERT(m_Rs != 0);

		m_Ws = m_Lock.tryConvertToWriteLock(m_Rs);
		CORIUM_ASSERT(m_Ws != 0);

		m_Lock.unlockWrite(m_Ws);
	}

	void resetImpl(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept {
		(void)ro_Adapter;
		// TODO: state reset between slice iterations.
	}

	void teardownImpl() noexcept {
		// TODO: teardown.
	}

	uint64_t getDeterminismHashImpl() noexcept {
		// TODO: return a semantically meaningful hash of output state.
		return 0;
	}
};


