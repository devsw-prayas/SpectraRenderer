#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumDiagnostics.h>

class SemaphoreTryAcquireSucceedsWhenPermitsAvailable final
	: public Hades::Runtime::IFixture<SemaphoreTryAcquireSucceedsWhenPermitsAvailable, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::Semaphore m_Sem{ 1 };
	bool m_Acquired{ false };

public:
	explicit SemaphoreTryAcquireSucceedsWhenPermitsAvailable(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_Acquired = false;
	}

	void executeImpl() noexcept {
		m_Acquired = m_Sem.tryAcquire();
		CORIUM_ASSERT(m_Acquired && "tryAcquire() should return true when a permit is available");
		CORIUM_ASSERT(m_Sem.availablePermits() == 0 && "availablePermits() should be 0 after tryAcquire()");
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


