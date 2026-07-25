/*
* File: SpectraAtomics.h
*
*
*
*
*
*
*
* Copyright (c) 2026 StormWeaver
*
* This file is part of the Spectra Render Engine API
*
* Licensed under the MIT License. You may obtain a copy of the License at
* https://opensource.org/licenses/MIT
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND...
*/
#pragma once
#include <SpectraPlatformRuntime.h>
#include <SpectraIntrin.h>
#include "SpectraDiagnostics.h"

namespace Spectra::Platform::Runtime::Intrinsic {
	template<typename T>
	struct ValidAtomicParameter final {
	private:
		using Decayed = std::remove_cv_t<std::remove_reference_t<T>>;

	public:
		SPECTRA_STATIC_ASSERT(
			std::is_trivially_copyable_v<Decayed>,
			"Atomic type must be trivially copyable."
		);

		SPECTRA_STATIC_ASSERT(
			!std::is_const_v<Decayed>,
			"Atomic type must not be const."
		);

		SPECTRA_STATIC_ASSERT(
			!std::is_volatile_v<Decayed>,
			"Atomic type must not be volatile."
		);

		SPECTRA_STATIC_ASSERT(
			sizeof(Decayed) <= 8,
			"Atomic type exceeds supported size (64-bit max)."
		);

		using Type =
			std::conditional_t<
			std::is_pointer_v<Decayed>,
			uintptr_t,
			std::conditional_t<
			(sizeof(Decayed) <= 4),
			uint32_t,
			uint64_t
			>
			>;
	};

	enum class SPECTRA_RUNTIME_API MemoryOrder : std::uint8_t {
		RELAXED = SPECTRA_MEMORY_ORDER_RELAXED,
		CONSUME = SPECTRA_MEMORY_ORDER_CONSUME,
		ACQUIRE = SPECTRA_MEMORY_ORDER_ACQUIRE,
		RELEASE = SPECTRA_MEMORY_ORDER_RELEASE,
		ACQ_REL = SPECTRA_MEMORY_ORDER_ACQ_REL,
		SEQ_CST = SPECTRA_MEMORY_ORDER_SEQ_CST
	};

	// =========================================================
	// Atomic Load
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic load")
		Valid atomicLoad(Valid* p_Memory, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_LOAD_RELAXED(Valid, p_Memory);
		case MemoryOrder::CONSUME:
			return Spec_ATOMIC_LOAD_CONSUME(Valid, p_Memory);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_LOAD_ACQUIRE(Valid, p_Memory);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_LOAD_SEQ_CST(Valid, p_Memory);
		case MemoryOrder::RELEASE:
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_LOAD_SEQ_CST(Valid, p_Memory);
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Store
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic store")
		void atomicStore(Valid* p_Memory, Valid v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			Spec_ATOMIC_STORE_RELAXED(Valid, p_Memory, v_Value);
			return;
		case MemoryOrder::SEQ_CST:
			Spec_ATOMIC_STORE_SEQ_CST(Valid, p_Memory, v_Value);
			return;
		case MemoryOrder::RELEASE:
			Spec_ATOMIC_STORE_RELEASE(Valid, p_Memory, v_Value);
			return;
		case MemoryOrder::ACQ_REL:
			Spec_ATOMIC_STORE_ACQ_REL(Valid, p_Memory, v_Value);
			return;
		case MemoryOrder::ACQUIRE:
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Fetch Add
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch add")
		Valid atomicFetchAdd32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_ADD_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_ADD_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_ADD_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_ADD_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_ADD_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch add")
		Valid atomicFetchAdd64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_ADD64_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_ADD64_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_ADD64_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_ADD64_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_ADD64_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Increment
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic increment")
		Valid atomicIncrement32(Valid* p_Memory, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_INCREMENT_RELAXED(p_Memory);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_INCREMENT_ACQUIRE(p_Memory);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_INCREMENT_RELEASE(p_Memory);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_INCREMENT_ACQ_REL(p_Memory);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_INCREMENT_SEQ_CST(p_Memory);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic increment")
		Valid atomicIncrement64(Valid* p_Memory, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_INCREMENT64_RELAXED(p_Memory);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_INCREMENT64_ACQUIRE(p_Memory);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_INCREMENT64_RELEASE(p_Memory);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_INCREMENT64_ACQ_REL(p_Memory);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_INCREMENT64_SEQ_CST(p_Memory);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Decrement
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic decrement")
		Valid atomicDecrement32(Valid* p_Memory, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_DECREMENT_RELAXED(p_Memory);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_DECREMENT_ACQUIRE(p_Memory);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_DECREMENT_RELEASE(p_Memory);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_DECREMENT_ACQ_REL(p_Memory);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_DECREMENT_SEQ_CST(p_Memory);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic decrement")
		Valid atomicDecrement64(Valid* p_Memory, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_DECREMENT64_RELAXED(p_Memory);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_DECREMENT64_ACQUIRE(p_Memory);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_DECREMENT64_RELEASE(p_Memory);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_DECREMENT64_ACQ_REL(p_Memory);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_DECREMENT64_SEQ_CST(p_Memory);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Exchange
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic exchange")
		Valid atomicExchange32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_EXCHANGE_RELAXED(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_EXCHANGE_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_EXCHANGE_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
		case MemoryOrder::CONSUME:
		case MemoryOrder::ACQUIRE:
			SPECTRA_ASSERT(false && "Invalid memory order for atomic exchange");
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic exchange")
		Valid atomicExchange64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_EXCHANGE64_RELAXED(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_EXCHANGE64_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_EXCHANGE64_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
		case MemoryOrder::CONSUME:
		case MemoryOrder::ACQUIRE:
			SPECTRA_ASSERT(false && "Invalid memory order for atomic exchange");
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic exchange pointer")
		Valid atomicExchangePointer(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_EXCHANGE_PTR_RELAXED(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_EXCHANGE_PTR_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_EXCHANGE_PTR_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
		case MemoryOrder::CONSUME:
		case MemoryOrder::ACQUIRE:
			SPECTRA_ASSERT(false && "Invalid memory order for atomic exchange pointer");
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Compare Exchange
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic compare exchange")
		Valid atomicCompareExchange32(
			Valid* p_Memory,
			T* p_Expected,
			T v_Desired,
			MemoryOrder v_OrderingSuccess,
			MemoryOrder v_OrderingFailure) {
		switch (v_OrderingSuccess) {
		case MemoryOrder::RELAXED:
			switch (v_OrderingFailure) {
			case MemoryOrder::RELAXED:
				return Spec_ATOMIC_COMPARE_EXCHANGE32_RELAXED_RELAXED(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::ACQUIRE:
				return Spec_ATOMIC_COMPARE_EXCHANGE32_RELAXED_ACQUIRE(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::SEQ_CST:
				return Spec_ATOMIC_COMPARE_EXCHANGE32_RELAXED_SEQ_CST(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::RELEASE:
			case MemoryOrder::ACQ_REL:
			case MemoryOrder::CONSUME:
				SPECTRA_UNREACHABLE();
			}

		case MemoryOrder::ACQUIRE:
			switch (v_OrderingFailure) {
			case MemoryOrder::RELAXED:
				return Spec_ATOMIC_COMPARE_EXCHANGE32_ACQUIRE_RELAXED(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::ACQUIRE:
				return Spec_ATOMIC_COMPARE_EXCHANGE32_ACQUIRE_ACQUIRE(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::SEQ_CST:
				return Spec_ATOMIC_COMPARE_EXCHANGE32_ACQUIRE_SEQ_CST(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::RELEASE:
			case MemoryOrder::ACQ_REL:
			case MemoryOrder::CONSUME:
				SPECTRA_UNREACHABLE();
			}

		case MemoryOrder::RELEASE:
			switch (v_OrderingFailure) {
			case MemoryOrder::RELAXED:
				return Spec_ATOMIC_COMPARE_EXCHANGE32_RELEASE_RELAXED(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::ACQUIRE:
				return Spec_ATOMIC_COMPARE_EXCHANGE32_RELEASE_ACQUIRE(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::SEQ_CST:
				return Spec_ATOMIC_COMPARE_EXCHANGE32_RELEASE_SEQ_CST(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::RELEASE:
			case MemoryOrder::ACQ_REL:
			case MemoryOrder::CONSUME:
				SPECTRA_UNREACHABLE();
			}

		case MemoryOrder::ACQ_REL:
			switch (v_OrderingFailure) {
			case MemoryOrder::RELAXED:
				return Spec_ATOMIC_COMPARE_EXCHANGE32_ACQ_REL_RELAXED(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::ACQUIRE:
				return Spec_ATOMIC_COMPARE_EXCHANGE32_ACQ_REL_ACQUIRE(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::SEQ_CST:
				return Spec_ATOMIC_COMPARE_EXCHANGE32_ACQ_REL_SEQ_CST(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::RELEASE:
			case MemoryOrder::ACQ_REL:
			case MemoryOrder::CONSUME:
				SPECTRA_UNREACHABLE();
			}

		case MemoryOrder::SEQ_CST:
			switch (v_OrderingFailure) {
			case MemoryOrder::RELAXED:
				return Spec_ATOMIC_COMPARE_EXCHANGE32_SEQ_CST_RELAXED(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::ACQUIRE:
				return Spec_ATOMIC_COMPARE_EXCHANGE32_SEQ_CST_ACQUIRE(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::SEQ_CST:
				return Spec_ATOMIC_COMPARE_EXCHANGE32_SEQ_CST_SEQ_CST(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::RELEASE:
			case MemoryOrder::ACQ_REL:
			case MemoryOrder::CONSUME:
				SPECTRA_UNREACHABLE();
			}

		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic compare exchange")
		Valid atomicCompareExchange64(
			Valid* p_Memory,
			T* p_Expected,
			T v_Desired,
			MemoryOrder v_OrderingSuccess,
			MemoryOrder v_OrderingFailure) {
		switch (v_OrderingSuccess) {
		case MemoryOrder::RELAXED:
			switch (v_OrderingFailure) {
			case MemoryOrder::RELAXED:
				return Spec_ATOMIC_COMPARE_EXCHANGE64_RELAXED_RELAXED(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::ACQUIRE:
				return Spec_ATOMIC_COMPARE_EXCHANGE64_RELAXED_ACQUIRE(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::SEQ_CST:
				return Spec_ATOMIC_COMPARE_EXCHANGE64_RELAXED_SEQ_CST(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::RELEASE:
			case MemoryOrder::ACQ_REL:
			case MemoryOrder::CONSUME:
				SPECTRA_UNREACHABLE();
			}

		case MemoryOrder::ACQUIRE:
			switch (v_OrderingFailure) {
			case MemoryOrder::RELAXED:
				return Spec_ATOMIC_COMPARE_EXCHANGE64_ACQUIRE_RELAXED(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::ACQUIRE:
				return Spec_ATOMIC_COMPARE_EXCHANGE64_ACQUIRE_ACQUIRE(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::SEQ_CST:
				return Spec_ATOMIC_COMPARE_EXCHANGE64_ACQUIRE_SEQ_CST(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::RELEASE:
			case MemoryOrder::ACQ_REL:
			case MemoryOrder::CONSUME:
				SPECTRA_UNREACHABLE();
			}

		case MemoryOrder::RELEASE:
			switch (v_OrderingFailure) {
			case MemoryOrder::RELAXED:
				return Spec_ATOMIC_COMPARE_EXCHANGE64_RELEASE_RELAXED(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::ACQUIRE:
				return Spec_ATOMIC_COMPARE_EXCHANGE64_RELEASE_ACQUIRE(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::SEQ_CST:
				return Spec_ATOMIC_COMPARE_EXCHANGE64_RELEASE_SEQ_CST(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::RELEASE:
			case MemoryOrder::ACQ_REL:
			case MemoryOrder::CONSUME:
				SPECTRA_UNREACHABLE();
			}

		case MemoryOrder::ACQ_REL:
			switch (v_OrderingFailure) {
			case MemoryOrder::RELAXED:
				return Spec_ATOMIC_COMPARE_EXCHANGE64_ACQ_REL_RELAXED(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::ACQUIRE:
				return Spec_ATOMIC_COMPARE_EXCHANGE64_ACQ_REL_ACQUIRE(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::SEQ_CST:
				return Spec_ATOMIC_COMPARE_EXCHANGE64_ACQ_REL_SEQ_CST(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::RELEASE:
			case MemoryOrder::ACQ_REL:
			case MemoryOrder::CONSUME:
				SPECTRA_UNREACHABLE();
			}

		case MemoryOrder::SEQ_CST:
			switch (v_OrderingFailure) {
			case MemoryOrder::RELAXED:
				return Spec_ATOMIC_COMPARE_EXCHANGE64_SEQ_CST_RELAXED(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::ACQUIRE:
				return Spec_ATOMIC_COMPARE_EXCHANGE64_SEQ_CST_ACQUIRE(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::SEQ_CST:
				return Spec_ATOMIC_COMPARE_EXCHANGE64_SEQ_CST_SEQ_CST(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::RELEASE:
			case MemoryOrder::ACQ_REL:
			case MemoryOrder::CONSUME:
				SPECTRA_UNREACHABLE();
			}

		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic compare exchange pointer")
		Valid atomicCompareExchangePointer(
			Valid* p_Memory,
			T* p_Expected,
			T v_Desired,
			MemoryOrder v_OrderingSuccess,
			MemoryOrder v_OrderingFailure) {
		switch (v_OrderingSuccess) {
		case MemoryOrder::RELAXED:
			switch (v_OrderingFailure) {
			case MemoryOrder::RELAXED:
				return Spec_ATOMIC_COMPARE_EXCHANGE_PTR_RELAXED_RELAXED(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::ACQUIRE:
				return Spec_ATOMIC_COMPARE_EXCHANGE_PTR_RELAXED_ACQUIRE(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::SEQ_CST:
				return Spec_ATOMIC_COMPARE_EXCHANGE_PTR_RELAXED_SEQ_CST(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::RELEASE:
			case MemoryOrder::ACQ_REL:
			case MemoryOrder::CONSUME:
				SPECTRA_UNREACHABLE();
			}

		case MemoryOrder::ACQUIRE:
			switch (v_OrderingFailure) {
			case MemoryOrder::RELAXED:
				return Spec_ATOMIC_COMPARE_EXCHANGE_PTR_ACQUIRE_RELAXED(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::ACQUIRE:
				return Spec_ATOMIC_COMPARE_EXCHANGE_PTR_ACQUIRE_ACQUIRE(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::SEQ_CST:
				return Spec_ATOMIC_COMPARE_EXCHANGE_PTR_ACQUIRE_SEQ_CST(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::RELEASE:
			case MemoryOrder::ACQ_REL:
			case MemoryOrder::CONSUME:
				SPECTRA_UNREACHABLE();
			}

		case MemoryOrder::RELEASE:
			switch (v_OrderingFailure) {
			case MemoryOrder::RELAXED:
				return Spec_ATOMIC_COMPARE_EXCHANGE_PTR_RELEASE_RELAXED(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::ACQUIRE:
				return Spec_ATOMIC_COMPARE_EXCHANGE_PTR_RELEASE_ACQUIRE(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::SEQ_CST:
				return Spec_ATOMIC_COMPARE_EXCHANGE_PTR_RELEASE_SEQ_CST(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::RELEASE:
			case MemoryOrder::ACQ_REL:
			case MemoryOrder::CONSUME:
				SPECTRA_UNREACHABLE();
			}

		case MemoryOrder::ACQ_REL:
			switch (v_OrderingFailure) {
			case MemoryOrder::RELAXED:
				return Spec_ATOMIC_COMPARE_EXCHANGE_PTR_ACQ_REL_RELAXED(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::ACQUIRE:
				return Spec_ATOMIC_COMPARE_EXCHANGE_PTR_ACQ_REL_ACQUIRE(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::SEQ_CST:
				return Spec_ATOMIC_COMPARE_EXCHANGE_PTR_ACQ_REL_SEQ_CST(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::RELEASE:
			case MemoryOrder::ACQ_REL:
			case MemoryOrder::CONSUME:
				SPECTRA_UNREACHABLE();
			}

		case MemoryOrder::SEQ_CST:
			switch (v_OrderingFailure) {
			case MemoryOrder::RELAXED:
				return Spec_ATOMIC_COMPARE_EXCHANGE_PTR_SEQ_CST_RELAXED(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::ACQUIRE:
				return Spec_ATOMIC_COMPARE_EXCHANGE_PTR_SEQ_CST_ACQUIRE(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::SEQ_CST:
				return Spec_ATOMIC_COMPARE_EXCHANGE_PTR_SEQ_CST_SEQ_CST(p_Memory, p_Expected, v_Desired, false);
			case MemoryOrder::RELEASE:
			case MemoryOrder::ACQ_REL:
			case MemoryOrder::CONSUME:
				SPECTRA_UNREACHABLE();
			}

		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Fetch AND
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch AND")
		Valid atomicFetchAnd32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_AND32_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_AND32_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_AND32_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_AND32_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_AND32_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch AND")
		Valid atomicFetchAnd64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_AND64_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_AND64_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_AND64_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_AND64_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_AND64_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Fetch OR
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch OR")
		Valid atomicFetchOr32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_OR32_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_OR32_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_OR32_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_OR32_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_OR32_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch OR")
		Valid atomicFetchOr64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_OR64_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_OR64_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_OR64_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_OR64_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_OR64_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Fetch XOR
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch XOR")
		Valid atomicFetchXor32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_XOR32_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_XOR32_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_XOR32_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_XOR32_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_XOR32_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch XOR")
		Valid atomicFetchXor64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_XOR64_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_XOR64_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_XOR64_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_XOR64_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_XOR64_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Fetch NAND
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch NAND")
		Valid atomicFetchNand32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_NAND32_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_NAND32_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_NAND32_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_NAND32_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_NAND32_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch NAND")
		Valid atomicFetchNand64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_NAND64_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_NAND64_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_NAND64_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_NAND64_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_NAND64_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Fetch Min
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch min")
		Valid atomicFetchMin32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_MIN32_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_MIN32_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_MIN32_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_MIN32_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_MIN32_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch min")
		Valid atomicFetchMin64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_MIN64_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_MIN64_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_MIN64_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_MIN64_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_MIN64_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Fetch Max
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch max")
		Valid atomicFetchMax32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_MAX32_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_MAX32_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_MAX32_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_MAX32_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_MAX32_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch max")
		Valid atomicFetchMax64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_MAX64_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_MAX64_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_MAX64_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_MAX64_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_MAX64_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Fetch Unsigned Min
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch unsigned min")
		Valid atomicFetchUnsignedMin32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_UMIN32_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_UMIN32_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_UMIN32_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_UMIN32_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_UMIN32_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch unsigned min")
		Valid atomicFetchUnsignedMin64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_UMIN64_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_UMIN64_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_UMIN64_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_UMIN64_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_UMIN64_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Fetch Unsigned Max
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch unsigned max")
		Valid atomicFetchUnsignedMax32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_UMAX32_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_UMAX32_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_UMAX32_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_UMAX32_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_UMAX32_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic fetch unsigned max")
		Valid atomicFetchUnsignedMax64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_ATOMIC_UMAX64_RELAXED(p_Memory, v_Value);
		case MemoryOrder::ACQUIRE:
			return Spec_ATOMIC_UMAX64_ACQUIRE(p_Memory, v_Value);
		case MemoryOrder::RELEASE:
			return Spec_ATOMIC_UMAX64_RELEASE(p_Memory, v_Value);
		case MemoryOrder::ACQ_REL:
			return Spec_ATOMIC_UMAX64_ACQ_REL(p_Memory, v_Value);
		case MemoryOrder::SEQ_CST:
			return Spec_ATOMIC_UMAX64_SEQ_CST(p_Memory, v_Value);
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Test and Set Bit
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic test and set")
		bool atomicTestAndSet32(Valid* p_Memory, int v_Bit, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_BITTEST_AND_SET32(p_Memory, v_Bit) != 0;
		case MemoryOrder::ACQUIRE:
			return Spec_BITTEST_AND_SET32(p_Memory, v_Bit) != 0;
		case MemoryOrder::RELEASE:
			return Spec_BITTEST_AND_SET32(p_Memory, v_Bit) != 0;
		case MemoryOrder::ACQ_REL:
			return Spec_BITTEST_AND_SET32(p_Memory, v_Bit) != 0;
		case MemoryOrder::SEQ_CST:
			return Spec_BITTEST_AND_SET32(p_Memory, v_Bit) != 0;
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic test and set")
		bool atomicTestAndSet64(Valid* p_Memory, int v_Bit, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_BITTEST_AND_SET64(p_Memory, v_Bit) != 0;
		case MemoryOrder::ACQUIRE:
			return Spec_BITTEST_AND_SET64(p_Memory, v_Bit) != 0;
		case MemoryOrder::RELEASE:
			return Spec_BITTEST_AND_SET64(p_Memory, v_Bit) != 0;
		case MemoryOrder::ACQ_REL:
			return Spec_BITTEST_AND_SET64(p_Memory, v_Bit) != 0;
		case MemoryOrder::SEQ_CST:
			return Spec_BITTEST_AND_SET64(p_Memory, v_Bit) != 0;
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Test and Reset Bit
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic test and reset")
		bool atomicTestAndReset32(Valid* p_Memory, int v_Bit, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_BITTEST_AND_RESET32(p_Memory, v_Bit) != 0;
		case MemoryOrder::ACQUIRE:
			return Spec_BITTEST_AND_RESET32(p_Memory, v_Bit) != 0;
		case MemoryOrder::RELEASE:
			return Spec_BITTEST_AND_RESET32(p_Memory, v_Bit) != 0;
		case MemoryOrder::ACQ_REL:
			return Spec_BITTEST_AND_RESET32(p_Memory, v_Bit) != 0;
		case MemoryOrder::SEQ_CST:
			return Spec_BITTEST_AND_RESET32(p_Memory, v_Bit) != 0;
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic test and reset")
		bool atomicTestAndReset64(Valid* p_Memory, int v_Bit, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_BITTEST_AND_RESET64(p_Memory, v_Bit) != 0;
		case MemoryOrder::ACQUIRE:
			return Spec_BITTEST_AND_RESET64(p_Memory, v_Bit) != 0;
		case MemoryOrder::RELEASE:
			return Spec_BITTEST_AND_RESET64(p_Memory, v_Bit) != 0;
		case MemoryOrder::ACQ_REL:
			return Spec_BITTEST_AND_RESET64(p_Memory, v_Bit) != 0;
		case MemoryOrder::SEQ_CST:
			return Spec_BITTEST_AND_RESET64(p_Memory, v_Bit) != 0;
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Interlocked Test and Set Bit
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic interlocked test and set")
		bool atomicInterlockedTestAndSet(Valid* p_Memory, int v_Bit, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_INTERLOCKED_BITTEST_AND_SET(p_Memory, v_Bit) != 0;
		case MemoryOrder::ACQUIRE:
			return Spec_INTERLOCKED_BITTEST_AND_SET(p_Memory, v_Bit) != 0;
		case MemoryOrder::RELEASE:
			return Spec_INTERLOCKED_BITTEST_AND_SET(p_Memory, v_Bit) != 0;
		case MemoryOrder::ACQ_REL:
			return Spec_INTERLOCKED_BITTEST_AND_SET(p_Memory, v_Bit) != 0;
		case MemoryOrder::SEQ_CST:
			return Spec_INTERLOCKED_BITTEST_AND_SET(p_Memory, v_Bit) != 0;
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Atomic Interlocked Test and Reset Bit
	// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic interlocked test and reset")
		bool atomicInterlockedTestAndReset(Valid* p_Memory, int v_Bit, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_INTERLOCKED_BITTEST_AND_RESET(p_Memory, v_Bit) != 0;
		case MemoryOrder::ACQUIRE:
			return Spec_INTERLOCKED_BITTEST_AND_RESET(p_Memory, v_Bit) != 0;
		case MemoryOrder::RELEASE:
			return Spec_INTERLOCKED_BITTEST_AND_RESET(p_Memory, v_Bit) != 0;
		case MemoryOrder::ACQ_REL:
			return Spec_INTERLOCKED_BITTEST_AND_RESET(p_Memory, v_Bit) != 0;
		case MemoryOrder::SEQ_CST:
			return Spec_INTERLOCKED_BITTEST_AND_RESET(p_Memory, v_Bit) != 0;
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
// Atomic Clear (Bit Test and Reset)
// =========================================================

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE SPECTRA_NODISCARD_MSG("Cannot discard an atomic clear")
		bool atomicClear(Valid* p_Memory, int v_Bit, MemoryOrder v_Ordering) {
		switch (v_Ordering) {
		case MemoryOrder::RELAXED:
			return Spec_INTERLOCKED_BITTEST_AND_RESET(p_Memory, v_Bit) != 0;
		case MemoryOrder::ACQUIRE:
			return Spec_INTERLOCKED_BITTEST_AND_RESET(p_Memory, v_Bit) != 0;
		case MemoryOrder::RELEASE:
			return Spec_INTERLOCKED_BITTEST_AND_RESET(p_Memory, v_Bit) != 0;
		case MemoryOrder::ACQ_REL:
			return Spec_INTERLOCKED_BITTEST_AND_RESET(p_Memory, v_Bit) != 0;
		case MemoryOrder::SEQ_CST:
			return Spec_INTERLOCKED_BITTEST_AND_RESET(p_Memory, v_Bit) != 0;
		case MemoryOrder::CONSUME:
			SPECTRA_UNREACHABLE();
		}
		SPECTRA_UNREACHABLE();
	}

	// =========================================================
	// Compiler Memory Barriers (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		void compilerReadReorderingBarrier() noexcept {
		Spec_COMPILER_READ_BARRIER();
	}

	SPECTRA_FORCEINLINE
		void compilerWriteReorderingBarrier() noexcept {
		Spec_COMPILER_WRITE_BARRIER();
	}

	SPECTRA_FORCEINLINE
		void compilerReadWriteReorderingBarrier() noexcept {
		Spec_COMPILER_READWRITE_BARRIER();
	}

	// =========================================================
	// CPU Memory Fences (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		void cpuLoadMemoryFence() noexcept {
		Spec_CPU_LOAD_FENCE();
	}

	SPECTRA_FORCEINLINE
		void cpuStoreMemoryFence() noexcept {
		Spec_CPU_STORE_FENCE();
	}

	SPECTRA_FORCEINLINE
		void cpuFullMemoryFence() noexcept {
		Spec_CPU_FULL_FENCE();
	}

	SPECTRA_FORCEINLINE
		void cpuFastStoreMemoryFence() noexcept {
		Spec_CPU_FAST_STORE_FENCE();
	}

	SPECTRA_FORCEINLINE
		void cpuSpinWaitPauseHint() noexcept {
		Spec_CPU_PAUSE();
	}

	// =========================================================
	// Bit Test (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		bool testBit(const uint32_t* p_Memory, int v_Bit) noexcept {
		return Spec_BITTEST32(p_Memory, v_Bit) != 0;
	}

	SPECTRA_FORCEINLINE
		bool testBit(const uint64_t* p_Memory, int v_Bit) noexcept {
		return Spec_BITTEST64(p_Memory, v_Bit) != 0;
	}

	// =========================================================
	// Bit Test and Complement (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		bool testAndComplementBit(int32_t* p_Memory, int v_Bit) noexcept {
		return Spec_BITTEST_AND_COMPLEMENT32(p_Memory, v_Bit) != 0;
	}

	SPECTRA_FORCEINLINE
		bool testAndComplementBit(int64_t* p_Memory, int v_Bit) noexcept {
		return Spec_BITTEST_AND_COMPLEMENT64(p_Memory, v_Bit) != 0;
	}

	// =========================================================
	// Bit Scan Helpers (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		bool scanLeastSignificantSetBit(uint32_t v_Value, uint32_t* p_Index) noexcept {
		return Spec_BITSCAN_FORWARD32(reinterpret_cast<unsigned long*>(p_Index), v_Value) != 0;
	}

	SPECTRA_FORCEINLINE
		bool scanLeastSignificantSetBit(uint64_t v_Value, uint32_t* p_Index) noexcept {
		return Spec_BITSCAN_FORWARD64(reinterpret_cast<unsigned long*>(p_Index), v_Value) != 0;
	}

	SPECTRA_FORCEINLINE
		bool scanMostSignificantSetBit(uint32_t v_Value, uint32_t* p_Index) noexcept {
		return Spec_BITSCAN_REVERSE32(reinterpret_cast<unsigned long*>(p_Index), v_Value) != 0;
	}

	SPECTRA_FORCEINLINE
		bool scanMostSignificantSetBit(uint64_t v_Value, uint32_t* p_Index) noexcept {
		return Spec_BITSCAN_REVERSE64(reinterpret_cast<unsigned long*>(p_Index), v_Value) != 0;
	}

	// =========================================================
	// Population Count (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		uint32_t countSetBits(uint32_t v_Value) noexcept {
		return static_cast<uint32_t>(Spec_POPCOUNT32(v_Value));
	}

	SPECTRA_FORCEINLINE
		uint32_t countSetBits(uint64_t v_Value) noexcept {
		return static_cast<uint32_t>(Spec_POPCOUNT64(v_Value));
	}

	// =========================================================
	// Bit Rotation (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		uint32_t rotateBitsLeft(uint32_t v_Value, int v_Shift) noexcept {
		return Spec_ROTL32(v_Value, v_Shift);
	}

	SPECTRA_FORCEINLINE
		uint64_t rotateBitsLeft(uint64_t v_Value, int v_Shift) noexcept {
		return Spec_ROTL64(v_Value, v_Shift);
	}

	SPECTRA_FORCEINLINE
		uint32_t rotateBitsRight(uint32_t v_Value, int v_Shift) noexcept {
		return Spec_ROTR32(v_Value, v_Shift);
	}

	SPECTRA_FORCEINLINE
		uint64_t rotateBitsRight(uint64_t v_Value, int v_Shift) noexcept {
		return Spec_ROTR64(v_Value, v_Shift);
	}

	// =========================================================
	// Byte Swap (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		uint16_t byteSwapEndianness(uint16_t v_Value) noexcept {
		return Spec_BYTESWAP16(v_Value);
	}

	SPECTRA_FORCEINLINE
		uint32_t byteSwapEndianness(uint32_t v_Value) noexcept {
		return Spec_BYTESWAP32(v_Value);
	}

	SPECTRA_FORCEINLINE
		uint64_t byteSwapEndianness(uint64_t v_Value) noexcept {
		return Spec_BYTESWAP64(v_Value);
	}

	// =========================================================
	// Timestamp & Performance Counters (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		uint64_t readTimeStampCounter() noexcept {
		return Spec_RDTSC();
	}

	SPECTRA_FORCEINLINE
		uint64_t readSerializedTimeStampCounter(uint32_t* p_Aux) noexcept {
		return Spec_RDTSCP(p_Aux);
	}

	SPECTRA_FORCEINLINE
		uint64_t readPerformanceMonitoringCounter(uint32_t v_Counter) noexcept {
		return Spec_READPMC(v_Counter);
	}

	// =========================================================
	// Carry / Borrow Helpers (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		uint8_t addWithCarry(
			uint8_t carryIn,
			uint8_t a,
			uint8_t b,
			uint8_t* out) noexcept {
		return Spec_ADDCARRY_U8(carryIn, a, b, out);
	}

	SPECTRA_FORCEINLINE
		uint8_t addWithCarry(
			uint8_t carryIn,
			uint16_t a,
			uint16_t b,
			uint16_t* out) noexcept {
		return Spec_ADDCARRY_U16(carryIn, a, b, out);
	}

	SPECTRA_FORCEINLINE
		uint8_t addWithCarry(
			uint8_t carryIn,
			uint32_t a,
			uint32_t b,
			uint32_t* out) noexcept {
		return Spec_ADDCARRY_U32(carryIn, a, b, out);
	}

	SPECTRA_FORCEINLINE
		uint8_t addWithCarry(
			uint8_t carryIn,
			uint64_t a,
			uint64_t b,
			uint64_t* out) noexcept {
		return Spec_ADDCARRY_U64(carryIn, a, b, out);
	}

	SPECTRA_FORCEINLINE
		uint8_t subtractWithBorrow(
			uint8_t borrowIn,
			uint8_t a,
			uint8_t b,
			uint8_t* out) noexcept {
		return Spec_SUBBORROW_U8(borrowIn, a, b, out);
	}

	SPECTRA_FORCEINLINE
		uint8_t subtractWithBorrow(
			uint8_t borrowIn,
			uint16_t a,
			uint16_t b,
			uint16_t* out) noexcept {
		return Spec_SUBBORROW_U16(borrowIn, a, b, out);
	}

	SPECTRA_FORCEINLINE
		uint8_t subtractWithBorrow(
			uint8_t borrowIn,
			uint32_t a,
			uint32_t b,
			uint32_t* out) noexcept {
		return Spec_SUBBORROW_U32(borrowIn, a, b, out);
	}

	SPECTRA_FORCEINLINE
		uint8_t subtractWithBorrow(
			uint8_t borrowIn,
			uint64_t a,
			uint64_t b,
			uint64_t* out) noexcept {
		return Spec_SUBBORROW_U64(borrowIn, a, b, out);
	}

	// =========================================================
	// Overflow Detection - Add (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		bool addWithOverflowDetection(
			uint8_t carryIn,
			int8_t a,
			int8_t b,
			int8_t* out) noexcept {
		return Spec_ADD_OVERFLOW_I8(carryIn, a, b, out) != 0;
	}

	SPECTRA_FORCEINLINE
		bool addWithOverflowDetection(
			int8_t a,
			int8_t b,
			int8_t* out) noexcept {
		return addWithOverflowDetection(0, a, b, out);
	}

	SPECTRA_FORCEINLINE
		bool addWithOverflowDetection(
			uint8_t carryIn,
			int16_t a,
			int16_t b,
			int16_t* out) noexcept {
		return Spec_ADD_OVERFLOW_I16(carryIn, a, b, out) != 0;
	}

	SPECTRA_FORCEINLINE
		bool addWithOverflowDetection(
			int16_t a,
			int16_t b,
			int16_t* out) noexcept {
		return addWithOverflowDetection(0, a, b, out);
	}

	SPECTRA_FORCEINLINE
		bool addWithOverflowDetection(
			uint8_t carryIn,
			int32_t a,
			int32_t b,
			int32_t* out) noexcept {
		return Spec_ADD_OVERFLOW_I32(carryIn, a, b, out) != 0;
	}

	SPECTRA_FORCEINLINE
		bool addWithOverflowDetection(
			int32_t a,
			int32_t b,
			int32_t* out) noexcept {
		return addWithOverflowDetection(0, a, b, out);
	}

	SPECTRA_FORCEINLINE
		bool addWithOverflowDetection(
			uint8_t carryIn,
			int64_t a,
			int64_t b,
			int64_t* out) noexcept {
		return Spec_ADD_OVERFLOW_I64(carryIn, a, b, out) != 0;
	}

	SPECTRA_FORCEINLINE
		bool addWithOverflowDetection(
			int64_t a,
			int64_t b,
			int64_t* out) noexcept {
		return addWithOverflowDetection(0, a, b, out);
	}

	// =========================================================
	// Overflow Detection - Subtract (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		bool subtractWithOverflowDetection(
			uint8_t borrowIn,
			int8_t a,
			int8_t b,
			int8_t* out) noexcept {
		return Spec_SUB_OVERFLOW_I8(borrowIn, a, b, out) != 0;
	}

	SPECTRA_FORCEINLINE
		bool subtractWithOverflowDetection(
			int8_t a,
			int8_t b,
			int8_t* out) noexcept {
		return subtractWithOverflowDetection(0, a, b, out);
	}

	SPECTRA_FORCEINLINE
		bool subtractWithOverflowDetection(
			uint8_t borrowIn,
			int16_t a,
			int16_t b,
			int16_t* out) noexcept {
		return Spec_SUB_OVERFLOW_I16(borrowIn, a, b, out) != 0;
	}

	SPECTRA_FORCEINLINE
		bool subtractWithOverflowDetection(
			int16_t a,
			int16_t b,
			int16_t* out) noexcept {
		return subtractWithOverflowDetection(0, a, b, out);
	}

	SPECTRA_FORCEINLINE
		bool subtractWithOverflowDetection(
			uint8_t borrowIn,
			int32_t a,
			int32_t b,
			int32_t* out) noexcept {
		return Spec_SUB_OVERFLOW_I32(borrowIn, a, b, out) != 0;
	}

	SPECTRA_FORCEINLINE
		bool subtractWithOverflowDetection(
			int32_t a,
			int32_t b,
			int32_t* out) noexcept {
		return subtractWithOverflowDetection(0, a, b, out);
	}

	SPECTRA_FORCEINLINE
		bool subtractWithOverflowDetection(
			uint8_t borrowIn,
			int64_t a,
			int64_t b,
			int64_t* out) noexcept {
		return Spec_SUB_OVERFLOW_I64(borrowIn, a, b, out) != 0;
	}

	SPECTRA_FORCEINLINE
		bool subtractWithOverflowDetection(
			int64_t a,
			int64_t b,
			int64_t* out) noexcept {
		return subtractWithOverflowDetection(0, a, b, out);
	}

	// =========================================================
	// Overflow Detection - Multiply (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		bool multiplyWithOverflowDetection(
			int16_t a,
			int16_t b,
			int16_t* out) noexcept {
		return Spec_MUL_OVERFLOW_I16(a, b, out) != 0;
	}

	SPECTRA_FORCEINLINE
		bool multiplyWithOverflowDetection(
			int32_t a,
			int32_t b,
			int32_t* out) noexcept {
		return Spec_MUL_OVERFLOW_I32(a, b, out) != 0;
	}

	SPECTRA_FORCEINLINE
		bool multiplyWithOverflowDetection(
			int64_t a,
			int64_t b,
			int64_t* out) noexcept {
		return Spec_MUL_OVERFLOW_I64(a, b, out) != 0;
	}

	// =========================================================
	// Full-Width Multiplication (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		bool multiplyFullWidth(
			int8_t a,
			int8_t b,
			int16_t* out) noexcept {
		return Spec_MUL_FULL_OVERFLOW_I8(a, b, out) != 0;
	}

	SPECTRA_FORCEINLINE
		bool multiplyFullWidth(
			uint8_t a,
			uint8_t b,
			uint16_t* out) noexcept {
		return Spec_MUL_FULL_OVERFLOW_U8(a, b, out) != 0;
	}

	SPECTRA_FORCEINLINE
		bool multiplyFullWidth(
			int16_t a,
			int16_t b,
			int16_t* hi,
			int16_t* lo) noexcept {
		return Spec_MUL_FULL_OVERFLOW_I16(a, b, hi, lo) != 0;
	}

	SPECTRA_FORCEINLINE
		bool multiplyFullWidth(
			uint16_t a,
			uint16_t b,
			uint16_t* lo,
			uint16_t* hi) noexcept {
		return Spec_MUL_FULL_OVERFLOW_U16(a, b, lo, hi) != 0;
	}

	SPECTRA_FORCEINLINE
		bool multiplyFullWidth(
			int32_t a,
			int32_t b,
			int32_t* lo,
			int32_t* hi) noexcept {
		return Spec_MUL_FULL_OVERFLOW_I32(a, b, lo, hi) != 0;
	}

	SPECTRA_FORCEINLINE
		bool multiplyFullWidth(
			uint32_t a,
			uint32_t b,
			uint32_t* lo,
			uint32_t* hi) noexcept {
		return Spec_MUL_FULL_OVERFLOW_U32(a, b, lo, hi) != 0;
	}

	SPECTRA_FORCEINLINE
		bool multiplyFullWidth(
			int64_t a,
			int64_t b,
			int64_t* lo,
			int64_t* hi) noexcept {
		return Spec_MUL_FULL_OVERFLOW_I64(a, b, lo, hi) != 0;
	}

	SPECTRA_FORCEINLINE
		bool multiplyFullWidth(
			uint64_t a,
			uint64_t b,
			uint64_t* lo,
			uint64_t* hi) noexcept {
		return Spec_MUL_FULL_OVERFLOW_U64(a, b, lo, hi) != 0;
	}

	// =========================================================
	// Saturating Arithmetic (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		int8_t saturatingAdd(int8_t a, int8_t b) noexcept {
		return Spec_SAT_ADD_I8(a, b);
	}

	SPECTRA_FORCEINLINE
		int8_t saturatingSubtract(int8_t a, int8_t b) noexcept {
		return Spec_SAT_SUB_I8(a, b);
	}

	SPECTRA_FORCEINLINE
		uint8_t saturatingAdd(uint8_t a, uint8_t b) noexcept {
		return Spec_SAT_ADD_U8(a, b);
	}

	SPECTRA_FORCEINLINE
		uint8_t saturatingSubtract(uint8_t a, uint8_t b) noexcept {
		return Spec_SAT_SUB_U8(a, b);
	}

	SPECTRA_FORCEINLINE
		int16_t saturatingAdd(int16_t a, int16_t b) noexcept {
		return Spec_SAT_ADD_I16(a, b);
	}

	SPECTRA_FORCEINLINE
		int16_t saturatingSubtract(int16_t a, int16_t b) noexcept {
		return Spec_SAT_SUB_I16(a, b);
	}

	SPECTRA_FORCEINLINE
		uint16_t saturatingAdd(uint16_t a, uint16_t b) noexcept {
		return Spec_SAT_ADD_U16(a, b);
	}

	SPECTRA_FORCEINLINE
		uint16_t saturatingSubtract(uint16_t a, uint16_t b) noexcept {
		return Spec_SAT_SUB_U16(a, b);
	}

	SPECTRA_FORCEINLINE
		int32_t saturatingAdd(int32_t a, int32_t b) noexcept {
		return Spec_SAT_ADD_I32(a, b);
	}

	SPECTRA_FORCEINLINE
		int32_t saturatingSubtract(int32_t a, int32_t b) noexcept {
		return Spec_SAT_SUB_I32(a, b);
	}

	SPECTRA_FORCEINLINE
		uint32_t saturatingAdd(uint32_t a, uint32_t b) noexcept {
		return Spec_SAT_ADD_U32(a, b);
	}

	SPECTRA_FORCEINLINE
		uint32_t saturatingSubtract(uint32_t a, uint32_t b) noexcept {
		return Spec_SAT_SUB_U32(a, b);
	}

	SPECTRA_FORCEINLINE
		int64_t saturatingAdd(int64_t a, int64_t b) noexcept {
		return Spec_SAT_ADD_I64(a, b);
	}

	SPECTRA_FORCEINLINE
		int64_t saturatingSubtract(int64_t a, int64_t b) noexcept {
		return Spec_SAT_SUB_I64(a, b);
	}

	SPECTRA_FORCEINLINE
		uint64_t saturatingAdd(uint64_t a, uint64_t b) noexcept {
		return Spec_SAT_ADD_U64(a, b);
	}

	SPECTRA_FORCEINLINE
		uint64_t saturatingSubtract(uint64_t a, uint64_t b) noexcept {
		return Spec_SAT_SUB_U64(a, b);
	}

	// =========================================================
	// CRC32 (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		uint32_t computeCRC32(uint32_t crc, uint8_t data) noexcept {
		return Spec_CRC32_U8(crc, data);
	}

	SPECTRA_FORCEINLINE
		uint32_t computeCRC32(uint32_t crc, uint16_t data) noexcept {
		return Spec_CRC32_U16(crc, data);
	}

	SPECTRA_FORCEINLINE
		uint32_t computeCRC32(uint32_t crc, uint32_t data) noexcept {
		return Spec_CRC32_U32(crc, data);
	}

	SPECTRA_FORCEINLINE
		uint32_t computeCRC32(uint32_t crc, uint64_t data) noexcept {
		return static_cast<uint32_t>(Spec_CRC32_U64(crc, data));
	}

	// =========================================================
	// Control-Flow Introspection (No Memory Order)
	// =========================================================

	SPECTRA_FORCEINLINE
		void* getReturnAddress() noexcept {
		return Spec_RETURN_ADDRESS();
	}

	SPECTRA_FORCEINLINE
		void* getAddressOfReturnAddress() noexcept {
		return Spec_ADDRESS_OF_RETURN_ADDRESS();
	}

	SPECTRA_FORCEINLINE
		void* getAddressOfNextInstruction() noexcept {
		return Spec_ADDRESS_OF_NEXT_INSTRUCTION();
	}

}
