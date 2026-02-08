#pragma once
#include "SpectraPlatformRuntime.h"
															
namespace Spectra::Platform::Runtime::Atomic {

	using Integer32 = long;
	using Integer64 = long long;

	class SPECTRA_RUNTIME_API alignas(32) Atomic32 final{
		volatile Integer32 m_Value;
	public:
		Atomic32(Integer32 v_Value) : m_Value(v_Value) {}

		operator Integer32();
		volatile Integer32* addrOf();

		Integer32 operator++();
		Integer32 operator++(int);

		Integer32 operator--();
		Integer32 operator--(int);

		Integer32 swap(Integer32 v_Value);
		Integer32 compareAndSwap(Integer32 v_Desired, Integer32 v_Expected);
	};

	class SPECTRA_RUNTIME_API alignas(32) Atomic64 final {
		volatile Integer64 m_Value;
	public:
		Atomic64(Integer64 v_Value) : m_Value(v_Value) {}

		operator Integer64();
		volatile Integer64* addrOf();

		Integer64 operator++();
		Integer64 operator++(int);

		Integer64 operator--();
		Integer64 operator--(int);

		Integer64 swap(Integer64 v_Value);
		Integer64 compareAndSwap(Integer64 v_Desired, Integer64 v_Expected);
	};
}
