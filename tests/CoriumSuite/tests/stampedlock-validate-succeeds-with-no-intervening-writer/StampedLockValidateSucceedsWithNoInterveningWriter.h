#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumDiagnostics.h>

class StampedLockValidateSucceedsWithNoInterveningWriter final
	: public Hades::Runtime::IFixture<StampedLockValidateSucceedsWithNoInterveningWriter, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::StampedLock m_Lock;
	uint64_t m_Stamp{ 0 };
	bool m_Valid{ false };

public:
	explicit StampedLockValidateSucceedsWithNoInterveningWriter(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_Stamp = 0;
		m_Valid = false;
	}

	void executeImpl() noexcept {
		m_Stamp = m_Lock.tryOptimisticRead();
		CORIUM_ASSERT(m_Stamp != 0ULL && "tryOptimisticRead returned 0 stamp on freshly constructed StampedLock");

		m_Valid = m_Lock.validate(m_Stamp);
		CORIUM_ASSERT(m_Valid && "validate(stamp) returned false with no intervening writer");
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


