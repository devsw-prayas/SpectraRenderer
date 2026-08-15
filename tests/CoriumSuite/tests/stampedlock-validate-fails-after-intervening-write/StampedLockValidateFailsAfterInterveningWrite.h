#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumEnvironment.h>

class StampedLockValidateFailsAfterInterveningWrite final
	: public Hades::Runtime::IFixture<StampedLockValidateFailsAfterInterveningWrite, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::StampedLock m_Lock;
	uint64_t m_Stamp{ 0 };
	uint64_t m_WriteStamp{ 0 };

public:
	explicit StampedLockValidateFailsAfterInterveningWrite(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		// TODO: PCIe host->device transfers (GPU fixtures) belong here.
	}

	void executeImpl() noexcept {
		m_Stamp = m_Lock.tryOptimisticRead();
		CORIUM_ASSERT(m_Stamp != 0);

		m_WriteStamp = m_Lock.writeLock();
		CORIUM_ASSERT(m_WriteStamp != 0);
		m_Lock.unlockWrite(m_WriteStamp);

		CORIUM_ASSERT(!m_Lock.validate(m_Stamp));
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


