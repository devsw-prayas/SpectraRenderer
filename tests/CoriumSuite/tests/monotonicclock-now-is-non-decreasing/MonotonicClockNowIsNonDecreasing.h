#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include <CoriumChrono.h>
#include <CoriumDiagnostics.h>

class MonotonicClockNowIsNonDecreasing final
	: public Hades::Runtime::IFixture<MonotonicClockNowIsNonDecreasing, Hades::Runtime::NullDeviceAdapter> {
private:
	Corium::Core::Chrono::Instant m_Prev{};
	Corium::Core::Chrono::Instant m_Curr{};

public:
	explicit MonotonicClockNowIsNonDecreasing(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		using namespace Corium::Core::Chrono;
		m_Prev = MonotonicClock::now();
		m_Curr = {};
	}

	void executeImpl() noexcept {
		using namespace Corium::Core::Chrono;

		for (size_t i = 0; i < 100'000; ++i) {
			m_Curr = MonotonicClock::now();
			CORIUM_ASSERT(m_Curr.m_Nanoseconds >= m_Prev.m_Nanoseconds);
			m_Prev = m_Curr;
		}
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

