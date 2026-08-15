#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumDiagnostics.h>

class CountDownLatchGetCountDecrementsPerCall final
	: public Hades::Runtime::IFixture<CountDownLatchGetCountDecrementsPerCall, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::CountDownLatch m_Latch{ 3 };

public:
	explicit CountDownLatchGetCountDecrementsPerCall(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
	}

	void executeImpl() noexcept {
		CORIUM_ASSERT(m_Latch.getCount() == 3);

		m_Latch.countDown();
		CORIUM_ASSERT(m_Latch.getCount() == 2);
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

