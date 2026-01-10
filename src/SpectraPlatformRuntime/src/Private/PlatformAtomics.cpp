#include "SpectraPlatformRuntime.h"
#include "PlatformAtomics.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace Spectra::Platform::Runtime::Atomic {
    Atomic32::operator Integer32() {
        return InterlockedCompareExchange(&m_Value, 0, 0);
    }

    Integer32 Atomic32::operator++() {
        return InterlockedIncrement(&m_Value);
    }

    Integer32 Atomic32::operator++(int) {
        Integer32 newVal = InterlockedIncrement(&m_Value);
        return newVal - 1;
    }

    Integer32 Atomic32::operator--() {
        return InterlockedDecrement(&m_Value);
    }

    Integer32 Atomic32::operator--(int) {
        Integer32 newVal = InterlockedDecrement(&m_Value);
        return newVal + 1;
    }

    Integer32 Atomic32::swap(Integer32 v_Value) {
        return InterlockedExchange(&m_Value, v_Value);
    }

    Integer32 Atomic32::compareAndSwap(Integer32 v_Desired, Integer32 v_Expected) {
        return InterlockedCompareExchange(&m_Value, v_Desired, v_Expected);
    }

    volatile Integer32* Atomic32::addrOf() {
        return &m_Value;
    }

    Atomic64::operator Integer64() {
        return InterlockedCompareExchange64(&m_Value, 0, 0);
    }

    Integer64 Atomic64::operator++() {
        return InterlockedIncrement64(&m_Value);
    }

    Integer64 Atomic64::operator++(int) {
        Integer64 newVal = InterlockedIncrement64(&m_Value);
        return newVal - 1;
    }

    Integer64 Atomic64::operator--() {
        return InterlockedDecrement64(&m_Value);
    }

    Integer64 Atomic64::operator--(int) {
        Integer64 newVal = InterlockedDecrement64(&m_Value);
        return newVal + 1;
    }

    Integer64 Atomic64::swap(Integer64 v_Value) {
        return InterlockedExchange64(&m_Value, v_Value);
    }

    Integer64 Atomic64::compareAndSwap(Integer64 v_Desired, Integer64 v_Expected) {
        return InterlockedCompareExchange64(&m_Value, v_Desired, v_Expected);
    }

    volatile Integer64* Atomic64::addrOf() {
        return &m_Value;
    }
}