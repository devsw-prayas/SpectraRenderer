#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumChrono.h>
#include <CoriumDiagnostics.h>

class SemaphoreTryAcquireTimesOutWhenExhausted final
	: public Hades::Runtime::IFixture<SemaphoreTryAcquireTimesOutWhenExhausted, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Runtime::Sync::Semaphore m_Sem{ 0 };
	bool m_Result{ false };

public:
	explicit SemaphoreTryAcquireTimesOutWhenExhausted(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		m_Result = false;
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;
		using namespace Corium::Core::Chrono::Literals;

		m_Result = m_Sem.tryAcquire(Chrono::until(1_ms));
		CORIUM_ASSERT(!m_Result && "tryAcquire on exhausted semaphore should have returned false");
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


