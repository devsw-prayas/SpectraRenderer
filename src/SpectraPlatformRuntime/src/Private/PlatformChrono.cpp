#include "SpectraPlatformRuntime.h"
#include "PlatformChrono.h"
#include "ProcessEnvironment.h"

#define ALLOW_SYSCALL
#include "SpectraSyscalls.h"

namespace Spectra::Platform::Runtime::Chrono {
	Duration::Duration(int64_t ns)
		: m_Nanoseconds(ns) {
	}

	Duration Duration::operator+(const Duration& r) const {
		return Duration(m_Nanoseconds + r.m_Nanoseconds);
	}

	Duration Duration::operator-(const Duration& r) const {
		return Duration(m_Nanoseconds - r.m_Nanoseconds);
	}

	Duration Duration::operator*(int64_t s) const {
		return Duration(m_Nanoseconds * s);
	}

	Duration Duration::operator/(int64_t d) const {
		return Duration(m_Nanoseconds / d);
	}

	double Duration::toSeconds() const {
		return static_cast<double>(m_Nanoseconds) / 1e9;
	}

	double Duration::toMilliseconds() const {
		return static_cast<double>(m_Nanoseconds) / 1e6;
	}

	Timestamp::Timestamp(int64_t v, ClockDomain d)
		: m_Value(v), m_Domain(d) {
	}

	Duration Timestamp::operator-(const Timestamp& r) const {
		if (m_Domain != r.m_Domain) {
			Environment::PlatformTermination::terminate();
		}

		return Duration(m_Value - r.m_Value);
	}

	CycleCount::CycleCount(uint64_t v)
		: m_Value(v) {
	}

	uint64_t CycleCount::value() const {
		return m_Value;
	}

	Timestamp MonotonicClock::now() {
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);

		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);

		int64_t ns =
			(counter.QuadPart * 1'000'000'000LL) / freq.QuadPart;

		return Timestamp(ns, ClockDomain::MONOTONIC);
	}

	ClockInfo MonotonicClock::getInfo() {
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);

		ClockInfo info{};
		info.m_IsMonotonic = true;
		info.m_IsHighResolution = true;
		info.m_Frequency = static_cast<uint64_t>(freq.QuadPart);

		return info;
	}

	Timestamp WallClock::now() {
		FILETIME ft;
		GetSystemTimePreciseAsFileTime(&ft);

		ULARGE_INTEGER uli;
		uli.LowPart = ft.dwLowDateTime;
		uli.HighPart = ft.dwHighDateTime;

		int64_t ns = static_cast<int64_t>(uli.QuadPart) * 100;

		return Timestamp(ns, ClockDomain::WALL);
	}

	ClockInfo WallClock::getInfo() {
		ClockInfo info{};
		info.m_IsMonotonic = false;
		info.m_IsHighResolution = true;
		info.m_Frequency = 10'000'000ULL;

		return info;
	}

	CycleCount CycleClock::now() {
		return CycleCount(__rdtsc());
	}
}