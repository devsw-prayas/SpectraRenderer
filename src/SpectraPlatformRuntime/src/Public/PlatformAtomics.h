#pragma once
#include "SpectraPlatformRuntime.h"
#include "SpectraDiagnostics.h"
#include <CoriumAtomics.h>
#include <type_traits>

// Spectra's atomics are a wrapper over Corium's header-only atomics. Spectra keeps its own class API and semantics:
// compareExchange writes the OBSERVED value back through the expected pointer (Corium's legacy shim leaves it untouched),
// and operands never widen (no 1/2-byte value is promoted to 4 bytes).
namespace Spectra::Platform::Runtime::Intrinsic {
	namespace Corium_ = ::Corium::Core::Atomics;

	using MemoryOrder = ::Corium::Atomics::MemoryOrder;

	template<typename T>
	struct ValidAtomicParameter final {
	private:
		using Decayed = std::remove_cv_t<std::remove_reference_t<T>>;

	public:
		SPECTRA_STATIC_ASSERT(std::is_trivially_copyable_v<Decayed>, "Atomic type must be trivially copyable.");
		SPECTRA_STATIC_ASSERT(sizeof(Decayed) <= 8, "Atomic type exceeds supported size (64-bit max).");

		// Pointers travel as uintptr_t; everything else keeps its own width as an unsigned integer.
		using Type = std::conditional_t<std::is_pointer_v<Decayed>, uintptr_t,
			std::conditional_t<(sizeof(Decayed) == 1), uint8_t,
			std::conditional_t<(sizeof(Decayed) == 2), uint16_t,
			std::conditional_t<(sizeof(Decayed) == 4), uint32_t, uint64_t>>>>;
	};

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_NODISCARD_MSG("Cannot discard an atomic load") SPECTRA_FORCEINLINE
		Valid atomicLoad(Valid* p_Memory, MemoryOrder v_Ordering) { return Corium_::atomicLoad<Valid>(p_Memory, v_Ordering); }

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE
		void atomicStore(Valid* p_Memory, Valid v_Value, MemoryOrder v_Ordering) { Corium_::atomicStore<Valid>(p_Memory, v_Value, v_Ordering); }

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE
		Valid atomicExchange32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) { return Corium_::atomicExchange32<Valid>(p_Memory, static_cast<Valid>(v_Value), v_Ordering); }

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE
		Valid atomicExchange64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) { return Corium_::atomicExchange64<Valid>(p_Memory, static_cast<Valid>(v_Value), v_Ordering); }

	// Returns the observed value and writes it through p_Expected (success iff it equals what the caller expected).
	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE
		Valid atomicCompareExchange32(Valid* p_Memory, T* p_Expected, T v_Desired, MemoryOrder v_Success, MemoryOrder v_Failure) {
		Valid expected = static_cast<Valid>(*p_Expected);
		const Valid observed = Corium_::atomicCompareExchange32<Valid>(p_Memory, &expected, static_cast<Valid>(v_Desired), v_Success, v_Failure);
		*p_Expected = static_cast<T>(observed);
		return observed;
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE
		Valid atomicCompareExchange64(Valid* p_Memory, T* p_Expected, T v_Desired, MemoryOrder v_Success, MemoryOrder v_Failure) {
		Valid expected = static_cast<Valid>(*p_Expected);
		const Valid observed = Corium_::atomicCompareExchange64<Valid>(p_Memory, &expected, static_cast<Valid>(v_Desired), v_Success, v_Failure);
		*p_Expected = static_cast<T>(observed);
		return observed;
	}

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE Valid atomicFetchAdd32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) { return Corium_::atomicFetchAdd32<Valid>(p_Memory, static_cast<Valid>(v_Value), v_Ordering); }
	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE Valid atomicFetchAdd64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) { return Corium_::atomicFetchAdd64<Valid>(p_Memory, static_cast<Valid>(v_Value), v_Ordering); }

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE Valid atomicIncrement32(Valid* p_Memory, MemoryOrder v_Ordering) { return Corium_::atomicIncrement32<Valid>(p_Memory, v_Ordering); }
	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE Valid atomicIncrement64(Valid* p_Memory, MemoryOrder v_Ordering) { return Corium_::atomicIncrement64<Valid>(p_Memory, v_Ordering); }
	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE Valid atomicDecrement32(Valid* p_Memory, MemoryOrder v_Ordering) { return Corium_::atomicDecrement32<Valid>(p_Memory, v_Ordering); }
	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE Valid atomicDecrement64(Valid* p_Memory, MemoryOrder v_Ordering) { return Corium_::atomicDecrement64<Valid>(p_Memory, v_Ordering); }

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE Valid atomicFetchAnd32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) { return Corium_::atomicFetchAnd<Valid>(p_Memory, static_cast<Valid>(v_Value), v_Ordering); }
	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE Valid atomicFetchAnd64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) { return Corium_::atomicFetchAnd<Valid>(p_Memory, static_cast<Valid>(v_Value), v_Ordering); }
	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE Valid atomicFetchOr32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) { return Corium_::atomicFetchOr<Valid>(p_Memory, static_cast<Valid>(v_Value), v_Ordering); }
	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE Valid atomicFetchOr64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) { return Corium_::atomicFetchOr<Valid>(p_Memory, static_cast<Valid>(v_Value), v_Ordering); }
	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE Valid atomicFetchXor32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) { return Corium_::atomicFetchXor<Valid>(p_Memory, static_cast<Valid>(v_Value), v_Ordering); }
	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE Valid atomicFetchXor64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) { return Corium_::atomicFetchXor<Valid>(p_Memory, static_cast<Valid>(v_Value), v_Ordering); }
	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE Valid atomicFetchNand32(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) { return Corium_::atomicFetchNand<Valid>(p_Memory, static_cast<Valid>(v_Value), v_Ordering); }
	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE Valid atomicFetchNand64(Valid* p_Memory, T v_Value, MemoryOrder v_Ordering) { return Corium_::atomicFetchNand<Valid>(p_Memory, static_cast<Valid>(v_Value), v_Ordering); }

	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE bool atomicTestAndSet32(Valid* p_Memory, int v_Bit, MemoryOrder v_Ordering) { return Corium_::atomicTestAndSet<Valid>(p_Memory, v_Bit, v_Ordering); }
	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE bool atomicTestAndSet64(Valid* p_Memory, int v_Bit, MemoryOrder v_Ordering) { return Corium_::atomicTestAndSet<Valid>(p_Memory, v_Bit, v_Ordering); }
	template<typename T, typename Valid = ValidAtomicParameter<T>::Type>
	SPECTRA_FORCEINLINE bool atomicClear(Valid* p_Memory, int v_Bit, MemoryOrder v_Ordering) { return Corium_::atomicClear<Valid>(p_Memory, v_Bit, v_Ordering); }
}
#include "SpectraCompiler.h"

namespace Spectra::Platform::Runtime::Atomic {
	template<typename T>
	struct AtomicValue32 final {
	private:
		using Valid = Intrinsic::ValidAtomicParameter<T>::Type;

		alignas(sizeof(Valid)) Valid m_Value;

	public:

		AtomicValue32() noexcept = default;
		~AtomicValue32() = default;

		explicit AtomicValue32(Valid v_Value) noexcept
			: m_Value(v_Value) {
		}

		AtomicValue32(const AtomicValue32&) = default;
		AtomicValue32& operator=(const AtomicValue32&) = default;
		AtomicValue32(AtomicValue32&&) noexcept = default;
		AtomicValue32& operator=(AtomicValue32&&) noexcept = default;

		SPECTRA_FORCEINLINE
			Valid load(Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) const noexcept {
			return Intrinsic::atomicLoad<Valid>(
				const_cast<Valid*>(&m_Value),
				v_Ordering
			);
		}

		SPECTRA_FORCEINLINE
			void store(
				Valid v_Value,
				Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			Intrinsic::atomicStore<Valid>(&m_Value, v_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			Valid exchange(Valid v_Value, Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicExchange32<Valid>(&m_Value, v_Value, v_Ordering
			);
		}

		SPECTRA_FORCEINLINE
			Valid compareExchange(Valid* v_Expected, Valid v_Desired, Intrinsic::MemoryOrder v_Success, Intrinsic::MemoryOrder v_Failure) noexcept {
			return Intrinsic::atomicCompareExchange32<Valid>(&m_Value, v_Expected, v_Desired, v_Success, v_Failure);
		}

		SPECTRA_FORCEINLINE
			Valid fetchAdd(Valid v_Value, Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicFetchAdd32<Valid>(&m_Value, v_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			Valid increment(Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicIncrement32<Valid>(&m_Value, v_Ordering
			);
		}

		SPECTRA_FORCEINLINE
			Valid decrement(Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicDecrement32<Valid>(&m_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			Valid fetchAnd(Valid v_Value, Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicFetchAnd32<Valid>(&m_Value, v_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			Valid fetchOr(Valid v_Value, Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicFetchOr32<Valid>(&m_Value, v_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			Valid fetchXor(
				Valid v_Value,
				Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicFetchXor32<Valid>(&m_Value, v_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			Valid fetchNand(
				Valid v_Value,
				Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicFetchNand32<Valid>(&m_Value, v_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			bool testAndSet(
				int bit,
				Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicTestAndSet32<Valid>(&m_Value, bit, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			bool clear(
				int bit,
				Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicClear<Valid>(&m_Value, bit, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			Valid* data() noexcept {
			return &m_Value;
		}

		SPECTRA_FORCEINLINE
			const Valid* data() const noexcept {
			return &m_Value;
		}

		SPECTRA_FORCEINLINE
			Valid* address() noexcept {
			return data();
		}

		SPECTRA_FORCEINLINE
			const Valid* address() const noexcept {
			return data();
		}
	};

	template<typename T>
	struct AtomicValue64 {
	private:
		using Valid = Intrinsic::ValidAtomicParameter<T>::Type;

		alignas(sizeof(Valid)) Valid m_Value;

	public:

		AtomicValue64() noexcept = default;

		explicit AtomicValue64(Valid v_Value) noexcept
			: m_Value(v_Value) {
		}

		AtomicValue64(const AtomicValue64&) = default;
		AtomicValue64& operator=(const AtomicValue64&) = default;
		AtomicValue64(AtomicValue64&&) noexcept = default;
		AtomicValue64& operator=(AtomicValue64&&) noexcept = default;

		SPECTRA_FORCEINLINE
			Valid load(Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) const noexcept {
			return Intrinsic::atomicLoad<Valid>(
				const_cast<Valid*>(&m_Value),
				v_Ordering
			);
		}

		SPECTRA_FORCEINLINE
			void store(
				Valid v_Value,
				Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			Intrinsic::atomicStore<Valid>(&m_Value, v_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			Valid exchange(Valid v_Value, Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicExchange64<Valid>(&m_Value, v_Value, v_Ordering
			);
		}

		SPECTRA_FORCEINLINE
			Valid compareExchange(Valid* v_Expected, Valid v_Desired, Intrinsic::MemoryOrder v_Success, Intrinsic::MemoryOrder v_Failure) noexcept {
			return Intrinsic::atomicCompareExchange64<Valid>(&m_Value, v_Expected, v_Desired, v_Success, v_Failure);
		}

		SPECTRA_FORCEINLINE
			Valid fetchAdd(Valid v_Value, Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicFetchAdd64<Valid>(&m_Value, v_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			Valid increment(Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicIncrement64<Valid>(&m_Value, v_Ordering
			);
		}

		SPECTRA_FORCEINLINE
			Valid decrement(Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicDecrement64<Valid>(&m_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			Valid fetchAnd(Valid v_Value, Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicFetchAnd64<Valid>(&m_Value, v_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			Valid fetchOr(Valid v_Value, Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicFetchOr64<Valid>(&m_Value, v_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			Valid fetchXor(
				Valid v_Value,
				Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicFetchXor64<Valid>(&m_Value, v_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			Valid fetchNand(
				Valid v_Value,
				Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicFetchNand64<Valid>(&m_Value, v_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			bool testAndSet(
				int bit,
				Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicTestAndSet64<Valid>(&m_Value, bit, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			bool clear(
				int bit,
				Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicClear<Valid>(&m_Value, bit, v_Ordering);
		}
	};

	template<typename T>
	struct AtomicPointer final : AtomicValue64<T*> {
		using AtomicValue64<T*>::AtomicValue64;
	};
}
