#pragma once
#include <Fixture.h>

#include <HadesAdapters.h>
#include <HadesChrono.h>
#include <HadesHash.h>

#include "../../common/PlatformSuiteBoot.h"
#include <PlatformChrono.h>

class TestFixture1 final
	: public Hades::Runtime::IFixture<TestFixture1, Hades::Runtime::NullDeviceAdapter> {
public:
	explicit TestFixture1(Hades::Runtime::NullDeviceAdapter& ro_Adapter) noexcept
		: IFixture(ro_Adapter) {
	}

	void startupImpl() noexcept {
		PLATFORM_CHECK(PlatformSuite::boot());
	}

	void executeImpl() noexcept {
		using namespace Spectra::Platform::Runtime;

		const Chrono::ClockInfo info = Chrono::MonotonicClock::getInfo();
		PLATFORM_CHECK(info.m_Frequency > 0 && info.m_IsMonotonic);

		Chrono::Timestamp previous = Chrono::MonotonicClock::now();
		for (int i = 0; i < 100000; ++i) {
			const Chrono::Timestamp current = Chrono::MonotonicClock::now();
			PLATFORM_CHECK((current - previous).m_Nanoseconds >= 0 && "The monotonic clock must never go backwards");
			previous = current;
		}

		const Chrono::CycleCount c0 = Chrono::CycleClock::now();
		const Chrono::Timestamp t0 = Chrono::MonotonicClock::now();
		while ((Chrono::MonotonicClock::now() - t0).m_Nanoseconds < 1'000'000) {}
		PLATFORM_CHECK(Chrono::CycleClock::now().value() > c0.value() && "The cycle counter must advance");
		PLATFORM_CHECK((Chrono::WallClock::now() - Chrono::WallClock::now()).m_Nanoseconds <= 0);

		const Chrono::Duration second(1'000'000'000);
		PLATFORM_CHECK(second.toSeconds() == 1.0 && second.toMilliseconds() == 1000.0);
		PLATFORM_CHECK((second + second).m_Nanoseconds == 2'000'000'000 && (second - second).m_Nanoseconds == 0);
		PLATFORM_CHECK((second * 3).m_Nanoseconds == 3'000'000'000 && (second / 4).m_Nanoseconds == 250'000'000);
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
