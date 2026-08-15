#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumSync.h>
#include <CoriumChrono.h>
#include <CoriumDiagnostics.h>
#include <CoriumRuntime.h>

class ExchangerTimedExchangeTimesOutWithNoPartner final
	: public Hades::Runtime::IFixture<ExchangerTimedExchangeTimesOutWithNoPartner, Hades::Runtime::NullDeviceAdapter> {
private:
	int m_result{ 0 };
	bool m_exchanged{ false };

public:
	explicit ExchangerTimedExchangeTimesOutWithNoPartner(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		Corium::CoriumRuntime::initRuntime();
		m_result = 0;
		m_exchanged = false;
	}

	void executeImpl() noexcept {
		using namespace Corium::Core;
		using namespace Corium::Runtime::Sync;
		using namespace Corium::Core::Chrono::Literals;

		Corium::Runtime::Sync::Exchanger<int> exchanger{};
		m_exchanged = exchanger.exchange(42, m_result, Chrono::until(1_ms));
		CORIUM_ASSERT(!m_exchanged && "Timed exchange with no partner should have returned false");
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

