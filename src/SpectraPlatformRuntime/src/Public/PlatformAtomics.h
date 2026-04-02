#pragma once
#include "SpectraPlatformRuntime.h"
#include "SpectraAtomics.h"
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
			Valid increment(Valid v_Value = 1, Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicIncrement64<Valid>(&m_Value, v_Ordering
			);
		}

		SPECTRA_FORCEINLINE
			Valid decrement(Valid v_Value = 1, Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicDecrement64<Valid>(&m_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			Valid fetchAnd(Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicFetchAnd64<Valid>(&m_Value, v_Ordering);
		}

		SPECTRA_FORCEINLINE
			Valid fetchOr(Intrinsic::MemoryOrder v_Ordering = Intrinsic::MemoryOrder::SEQ_CST) noexcept {
			return Intrinsic::atomicFetchOr64<Valid>(&m_Value, v_Ordering);
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
