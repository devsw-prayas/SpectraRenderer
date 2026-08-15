#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumChrono.h>
#include <CoriumDiagnostics.h>

class IntervalArithmeticAddSubtractRoundTrips final
	: public Hades::Runtime::IFixture<IntervalArithmeticAddSubtractRoundTrips, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Core::Chrono::Interval m_A{ 100 };
	Corium::Core::Chrono::Interval m_B{ 40 };

public:
	explicit IntervalArithmeticAddSubtractRoundTrips(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		using namespace Corium::Core::Chrono;
		m_A = Interval{ 100 };
		m_B = Interval{ 40 };
	}

	void executeImpl() noexcept {
		CORIUM_ASSERT((m_A + m_B) - m_B == m_A);
		CORIUM_ASSERT(m_A * 2 == Corium::Core::Chrono::Interval{ 200 });
		CORIUM_ASSERT(m_A / 2 == Corium::Core::Chrono::Interval{ 50 });
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

