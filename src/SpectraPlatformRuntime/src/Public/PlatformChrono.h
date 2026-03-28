#pragma once
#include "SpectraPlatformRuntime.h"

namespace Spectra::Platform::Runtime::Chrono {
	enum class ClockDomain final : uint8_t {
		MONOTONIC, WALL, CYCLE
	};

    struct alignas(8) Duration final {
        int64_t m_Nanoseconds;

        Duration() = default;
        explicit Duration(int64_t ns);

        Duration operator+(const Duration& r) const;
        Duration operator-(const Duration& r) const;
        Duration operator*(int64_t s) const;
        Duration operator/(int64_t d) const;

        double toSeconds() const;
        double toMilliseconds() const;
    };


    struct alignas(16) Timestamp final {
        int64_t m_Value;
        ClockDomain m_Domain;

        Timestamp() = default;
        Timestamp(int64_t v, ClockDomain d);

        Duration operator-(const Timestamp& r) const;
    };

    struct alignas(8) CycleCount final {
        uint64_t m_Value;

        CycleCount() = default;
        explicit CycleCount(uint64_t v);

        uint64_t value() const;
    };


    struct alignas(16) ClockInfo final {
        uint64_t m_Frequency;
        bool m_IsMonotonic;
        bool m_IsHighResolution;
    };

    class MonotonicClock final {
    public:
        static Timestamp now();
        static ClockInfo getInfo();
    };

    class WallClock final {
    public:
        static Timestamp now();
        static ClockInfo getInfo();
    };

    class CycleClock final {
    public:
        static CycleCount now();
    };

}
