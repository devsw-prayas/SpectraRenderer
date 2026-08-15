#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumDiagnostics.h>

class CountDownLatchAwaitReleasesAfterZero final
	: public Hades::Runtime::IFixture<CountDownLatchAwaitReleasesAfterZero, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::CountDownLatch m_Latch{ 2 };

public:
	explicit CountDownLatchAwaitReleasesAfterZero(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_Latch.countDown();
		m_Latch.countDown();
	}

	void executeImpl() noexcept {
		m_Latch.await();

		CORIUM_ASSERT(m_Latch.getCount() == 0);
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

