#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumEnvironment.h>

class SemaphoreAvailablePermitsTracksAcquireRelease final
	: public Hades::Runtime::IFixture<SemaphoreAvailablePermitsTracksAcquireRelease, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::Semaphore m_Sem{ 5 };

public:
	explicit SemaphoreAvailablePermitsTracksAcquireRelease(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
	}

	void executeImpl() noexcept {
		CORIUM_ASSERT(m_Sem.availablePermits() == 5 && "Initial permits count should be 5");

		m_Sem.acquire(2);
		CORIUM_ASSERT(m_Sem.availablePermits() == 3 && "Available permits count after acquiring 2 should be 3");

		m_Sem.release(2);
		CORIUM_ASSERT(m_Sem.availablePermits() == 5 && "Available permits count after releasing 2 should be 5");
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

