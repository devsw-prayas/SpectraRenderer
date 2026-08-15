#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumChrono.h>
#include <CoriumDiagnostics.h>

class IntervalConversionsMatchNanosecondBasis final
	: public Hades::Runtime::IFixture<IntervalConversionsMatchNanosecondBasis, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Core::Chrono::Interval m_Interval{ 1'500'000LL };
	Corium::Core::Chrono::Interval m_SecondsInterval{ 2'500'000'000LL };

public:
	explicit IntervalConversionsMatchNanosecondBasis(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		using namespace Corium::Core::Chrono;
		m_Interval = Interval{ 1'500'000LL };
		m_SecondsInterval = Interval{ 2'500'000'000LL };
	}

	void executeImpl() noexcept {
		CORIUM_ASSERT(m_Interval.toNanoseconds() == 1'500'000LL);
		CORIUM_ASSERT(m_Interval.toMicroseconds() == 1500LL);
		CORIUM_ASSERT(m_Interval.toMilliseconds() == 1LL);
		CORIUM_ASSERT(m_Interval.toSeconds() == 0LL);

		CORIUM_ASSERT(m_SecondsInterval.toSeconds() == 2LL);
		CORIUM_ASSERT(m_SecondsInterval.toMilliseconds() == 2500LL);
		CORIUM_ASSERT(m_SecondsInterval.toMicroseconds() == 2'500'000LL);
		CORIUM_ASSERT(m_SecondsInterval.toNanoseconds() == 2'500'000'000LL);
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

