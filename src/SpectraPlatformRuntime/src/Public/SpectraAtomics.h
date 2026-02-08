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

namespace Spectra::Platform::Runtime::Intrinsic {
	// Contains wrapper functions over the macros inside SpectraIntrin.h

	enum class SPECTRA_RUNTIME_API MemoryOrder : std::uint8_t {
		RELAXED = SPECTRA_MEMORY_ORDER_RELAXED,
		CONSUME = SPECTRA_MEMORY_ORDER_CONSUME,
		ACQUIRE = SPECTRA_MEMORY_ORDER_ACQUIRE,
		RELEASE = SPECTRA_MEMORY_ORDER_RELEASE,
		ACQ_REL = SPECTRA_MEMORY_ORDER_ACQ_REL,
		SEQ_CST = SPECTRA_MEMORY_ORDER_SEQ_CST
	};

	// =========================================================
// Atomic Load / Store
// =========================================================

	template<typename T>
	SPECTRA_FORCEINLINE
		SPECTRA_NODISCARD_MSG("Atomic Loads should not be discarded")
		T atomicLoad(const T* p_Memory, MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_LOAD(p_Memory, static_cast<int>(v_Ordering));
	}

	template<typename T>
	SPECTRA_FORCEINLINE
		void atomicStore(T* p_Memory, T v_Value, MemoryOrder v_Ordering) noexcept
	{
		Spec_ATOMIC_STORE(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	// =========================================================
	// Atomic Arithmetic
	// =========================================================

	SPECTRA_FORCEINLINE
		int32_t atomicFetchAdd(int32_t* p_Memory, int32_t v_Value, MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_ADD(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int64_t atomicFetchAdd(int64_t* p_Memory, int64_t v_Value, MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_ADD64(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int32_t atomicIncrementAndFetch(int32_t* p_Memory, MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_INCREMENT(p_Memory, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int64_t atomicIncrementAndFetch(int64_t* p_Memory, MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_INCREMENT64(p_Memory, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int32_t atomicDecrementAndFetch(int32_t* p_Memory, MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_DECREMENT(p_Memory, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int64_t atomicDecrementAndFetch(int64_t* p_Memory, MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_DECREMENT64(p_Memory, static_cast<int>(v_Ordering));
	}

	// =========================================================
	// Atomic Exchange
	// =========================================================

	SPECTRA_FORCEINLINE
		int32_t atomicExchangeAndFetchPrevious(
			int32_t* p_Memory,
			int32_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_EXCHANGE(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int64_t atomicExchangeAndFetchPrevious(
			int64_t* p_Memory,
			int64_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_EXCHANGE64(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	template<typename T>
	SPECTRA_FORCEINLINE
		T* atomicExchangePointerAndFetchPrevious(
			T** p_Memory,
			T* p_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return static_cast<T*>(
			Spec_ATOMIC_EXCHANGE_PTR(
				p_Memory, p_Value, static_cast<int>(v_Ordering))
			);
	}

	// =========================================================
	// Atomic Compare Exchange
	// =========================================================

	SPECTRA_FORCEINLINE
		int32_t atomicCompareExchangeAndFetchPrevious(
			int32_t* p_Memory,
			int32_t* p_Expected,
			int32_t  v_Desired,
			bool     v_Weak,
			MemoryOrder v_Success,
			MemoryOrder v_Failure) noexcept
	{
		return Spec_ATOMIC_COMPARE_EXCHANGE32(
			p_Memory,
			p_Expected,
			v_Desired,
			v_Weak,
			static_cast<int>(v_Success),
			static_cast<int>(v_Failure));
	}

	SPECTRA_FORCEINLINE
		int64_t atomicCompareExchangeAndFetchPrevious(
			int64_t* p_Memory,
			int64_t* p_Expected,
			int64_t  v_Desired,
			bool     v_Weak,
			MemoryOrder v_Success,
			MemoryOrder v_Failure) noexcept
	{
		return Spec_ATOMIC_COMPARE_EXCHANGE64(
			p_Memory,
			p_Expected,
			v_Desired,
			v_Weak,
			static_cast<int>(v_Success),
			static_cast<int>(v_Failure));
	}

	template<typename T>
	SPECTRA_FORCEINLINE
		T* atomicCompareExchangePointerAndFetchPrevious(
			T** p_Memory,
			T** p_Expected,
			T* p_Desired,
			bool v_Weak,
			MemoryOrder v_Success,
			MemoryOrder v_Failure) noexcept
	{
		return static_cast<T*>(
			Spec_ATOMIC_COMPARE_EXCHANGE_PTR(
				p_Memory,
				p_Expected,
				p_Desired,
				v_Weak,
				static_cast<int>(v_Success),
				static_cast<int>(v_Failure))
			);
	}

	// =========================================================
	// Atomic Bitwise Read-Modify-Write
	// =========================================================

	SPECTRA_FORCEINLINE
		int32_t atomicFetchAnd(
			int32_t* p_Memory,
			int32_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_AND32(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int64_t atomicFetchAnd(
			int64_t* p_Memory,
			int64_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_AND64(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int32_t atomicFetchOr(
			int32_t* p_Memory,
			int32_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_OR32(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int64_t atomicFetchOr(
			int64_t* p_Memory,
			int64_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_OR64(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int32_t atomicFetchXor(
			int32_t* p_Memory,
			int32_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_XOR32(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int64_t atomicFetchXor(
			int64_t* p_Memory,
			int64_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_XOR64(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int32_t atomicFetchNand(
			int32_t* p_Memory,
			int32_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_NAND32(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int64_t atomicFetchNand(
			int64_t* p_Memory,
			int64_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_NAND64(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	// =========================================================
	// Atomic Min / Max
	// =========================================================

	SPECTRA_FORCEINLINE
		int32_t atomicFetchMin(
			int32_t* p_Memory,
			int32_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_MIN32(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int64_t atomicFetchMin(
			int64_t* p_Memory,
			int64_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_MIN64(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int32_t atomicFetchMax(
			int32_t* p_Memory,
			int32_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_MAX32(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		int64_t atomicFetchMax(
			int64_t* p_Memory,
			int64_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_MAX64(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		uint32_t atomicFetchUnsignedMin(
			uint32_t* p_Memory,
			uint32_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_UMIN32(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		uint64_t atomicFetchUnsignedMin(
			uint64_t* p_Memory,
			uint64_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_UMIN64(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		uint32_t atomicFetchUnsignedMax(
			uint32_t* p_Memory,
			uint32_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_UMAX32(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	SPECTRA_FORCEINLINE
		uint64_t atomicFetchUnsignedMax(
			uint64_t* p_Memory,
			uint64_t  v_Value,
			MemoryOrder v_Ordering) noexcept
	{
		return Spec_ATOMIC_UMAX64(p_Memory, v_Value, static_cast<int>(v_Ordering));
	}

	// =========================================================
	// Compiler & CPU Memory Barriers
	// =========================================================

	SPECTRA_FORCEINLINE
		void compilerReadReorderingBarrier() noexcept
	{
		Spec_COMPILER_READ_BARRIER();
	}

	SPECTRA_FORCEINLINE
		void compilerWriteReorderingBarrier() noexcept
	{
		Spec_COMPILER_WRITE_BARRIER();
	}

	SPECTRA_FORCEINLINE
		void compilerReadWriteReorderingBarrier() noexcept
	{
		Spec_COMPILER_READWRITE_BARRIER();
	}

	SPECTRA_FORCEINLINE
		void cpuLoadMemoryFence() noexcept
	{
		Spec_CPU_LOAD_FENCE();
	}

	SPECTRA_FORCEINLINE
		void cpuStoreMemoryFence() noexcept
	{
		Spec_CPU_STORE_FENCE();
	}

	SPECTRA_FORCEINLINE
		void cpuFullMemoryFence() noexcept
	{
		Spec_CPU_FULL_FENCE();
	}

	SPECTRA_FORCEINLINE
		void cpuSpinWaitPauseHint() noexcept
	{
		Spec_CPU_PAUSE();
	}

	// =========================================================
// Bit Test & Bit Modification
// =========================================================

	SPECTRA_FORCEINLINE
		bool testBit(const uint32_t* p_Memory, int v_Bit) noexcept
	{
		return Spec_BITTEST32(p_Memory, v_Bit) != 0;
	}

	SPECTRA_FORCEINLINE
		bool testBit(const uint64_t* p_Memory, int v_Bit) noexcept
	{
		return Spec_BITTEST64(p_Memory, v_Bit) != 0;
	}

	SPECTRA_FORCEINLINE
		bool atomicTestAndSetBit(int32_t* p_Memory, int v_Bit) noexcept
	{
		return Spec_BITTEST_AND_SET32(p_Memory, v_Bit) != 0;
	}

	SPECTRA_FORCEINLINE
		bool atomicTestAndSetBit(int64_t* p_Memory, int v_Bit) noexcept
	{
		return Spec_BITTEST_AND_SET64(p_Memory, v_Bit) != 0;
	}

	SPECTRA_FORCEINLINE
		bool atomicTestAndResetBit(int32_t* p_Memory, int v_Bit) noexcept
	{
		return Spec_BITTEST_AND_RESET32(p_Memory, v_Bit) != 0;
	}

	SPECTRA_FORCEINLINE
		bool atomicTestAndResetBit(int64_t* p_Memory, int v_Bit) noexcept
	{
		return Spec_BITTEST_AND_RESET64(p_Memory, v_Bit) != 0;
	}

	SPECTRA_FORCEINLINE
		bool atomicTestAndComplementBit(int32_t* p_Memory, int v_Bit) noexcept
	{
		return Spec_BITTEST_AND_COMPLEMENT32(p_Memory, v_Bit) != 0;
	}

	SPECTRA_FORCEINLINE
		bool atomicTestAndComplementBit(int64_t* p_Memory, int v_Bit) noexcept
	{
		return Spec_BITTEST_AND_COMPLEMENT64(p_Memory, v_Bit) != 0;
	}

	SPECTRA_FORCEINLINE
		bool atomicInterlockedTestAndSetBit(int32_t* p_Memory, int v_Bit) noexcept
	{
		return Spec_INTERLOCKED_BITTEST_AND_SET(p_Memory, v_Bit) != 0;
	}

	SPECTRA_FORCEINLINE
		bool atomicInterlockedTestAndResetBit(int32_t* p_Memory, int v_Bit) noexcept
	{
		return Spec_INTERLOCKED_BITTEST_AND_RESET(p_Memory, v_Bit) != 0;
	}

	// =========================================================
	// Bit Scan Helpers
	// =========================================================

	SPECTRA_FORCEINLINE
		bool scanLeastSignificantSetBit(uint32_t v_Value, uint32_t* p_Index) noexcept
	{
		return Spec_BITSCAN_FORWARD32(reinterpret_cast<unsigned long*>(p_Index), v_Value) != 0;
	}

	SPECTRA_FORCEINLINE
		bool scanLeastSignificantSetBit(uint64_t v_Value, uint32_t* p_Index) noexcept
	{
		return Spec_BITSCAN_FORWARD64(reinterpret_cast<unsigned long*>(p_Index), v_Value) != 0;
	}

	SPECTRA_FORCEINLINE
		bool scanMostSignificantSetBit(uint32_t v_Value, uint32_t* p_Index) noexcept
	{
		return Spec_BITSCAN_REVERSE32(reinterpret_cast<unsigned long*>(p_Index), v_Value) != 0;
	}

	SPECTRA_FORCEINLINE
		bool scanMostSignificantSetBit(uint64_t v_Value, uint32_t* p_Index) noexcept
	{
		return Spec_BITSCAN_REVERSE64(reinterpret_cast<unsigned long*>(p_Index), v_Value) != 0;
	}

	// =========================================================
	// Population Count
	// =========================================================

	SPECTRA_FORCEINLINE
		uint32_t countSetBits(uint32_t v_Value) noexcept
	{
		return static_cast<uint32_t>(Spec_POPCOUNT32(v_Value));
	}

	SPECTRA_FORCEINLINE
		uint32_t countSetBits(uint64_t v_Value) noexcept
	{
		return static_cast<uint32_t>(Spec_POPCOUNT64(v_Value));
	}

	// =========================================================
	// Bit Rotation
	// =========================================================

	SPECTRA_FORCEINLINE
		uint32_t rotateBitsLeft(uint32_t v_Value, int v_Shift) noexcept
	{
		return Spec_ROTL32(v_Value, v_Shift);
	}

	SPECTRA_FORCEINLINE
		uint64_t rotateBitsLeft(uint64_t v_Value, int v_Shift) noexcept
	{
		return Spec_ROTL64(v_Value, v_Shift);
	}

	SPECTRA_FORCEINLINE
		uint32_t rotateBitsRight(uint32_t v_Value, int v_Shift) noexcept
	{
		return Spec_ROTR32(v_Value, v_Shift);
	}

	SPECTRA_FORCEINLINE
		uint64_t rotateBitsRight(uint64_t v_Value, int v_Shift) noexcept
	{
		return Spec_ROTR64(v_Value, v_Shift);
	}

	// =========================================================
	// Byte Swap / Endianness
	// =========================================================

	SPECTRA_FORCEINLINE
		uint16_t byteSwapEndianness(uint16_t v_Value) noexcept
	{
		return Spec_BYTESWAP16(v_Value);
	}

	SPECTRA_FORCEINLINE
		uint32_t byteSwapEndianness(uint32_t v_Value) noexcept
	{
		return Spec_BYTESWAP32(v_Value);
	}

	SPECTRA_FORCEINLINE
		uint64_t byteSwapEndianness(uint64_t v_Value) noexcept
	{
		return Spec_BYTESWAP64(v_Value);
	}

	// =========================================================
	// Timestamp & Performance Counters
	// =========================================================

	SPECTRA_FORCEINLINE
		uint64_t readTimeStampCounter() noexcept
	{
		return Spec_RDTSC();
	}

	SPECTRA_FORCEINLINE
		uint64_t readSerializedTimeStampCounter(uint32_t* p_Aux) noexcept
	{
		return Spec_RDTSCP(p_Aux);
	}

	SPECTRA_FORCEINLINE
		uint64_t readPerformanceMonitoringCounter(uint32_t v_Counter) noexcept
	{
		return Spec_READPMC(v_Counter);
	}

	// =========================================================
	// Carry / Borrow Helpers (Unsigned Arithmetic)
	// =========================================================

	SPECTRA_FORCEINLINE
		uint8_t addWithCarry(
			uint8_t carryIn,
			uint8_t a,
			uint8_t b,
			uint8_t* out) noexcept
	{
		return Spec_ADDCARRY_U8(carryIn, a, b, out);
	}

	SPECTRA_FORCEINLINE
		uint8_t addWithCarry(
			uint8_t carryIn,
			uint16_t a,
			uint16_t b,
			uint16_t* out) noexcept
	{
		return Spec_ADDCARRY_U16(carryIn, a, b, out);
	}

	SPECTRA_FORCEINLINE
		uint8_t addWithCarry(
			uint8_t carryIn,
			uint32_t a,
			uint32_t b,
			uint32_t* out) noexcept
	{
		return Spec_ADDCARRY_U32(carryIn, a, b, out);
	}

	SPECTRA_FORCEINLINE
		uint8_t addWithCarry(
			uint8_t carryIn,
			uint64_t a,
			uint64_t b,
			uint64_t* out) noexcept
	{
		return Spec_ADDCARRY_U64(carryIn, a, b, out);
	}

	SPECTRA_FORCEINLINE
		uint8_t subtractWithBorrow(
			uint8_t borrowIn,
			uint8_t a,
			uint8_t b,
			uint8_t* out) noexcept
	{
		return Spec_SUBBORROW_U8(borrowIn, a, b, out);
	}

	SPECTRA_FORCEINLINE
		uint8_t subtractWithBorrow(
			uint8_t borrowIn,
			uint16_t a,
			uint16_t b,
			uint16_t* out) noexcept
	{
		return Spec_SUBBORROW_U16(borrowIn, a, b, out);
	}

	SPECTRA_FORCEINLINE
		uint8_t subtractWithBorrow(
			uint8_t borrowIn,
			uint32_t a,
			uint32_t b,
			uint32_t* out) noexcept
	{
		return Spec_SUBBORROW_U32(borrowIn, a, b, out);
	}

	SPECTRA_FORCEINLINE
		uint8_t subtractWithBorrow(
			uint8_t borrowIn,
			uint64_t a,
			uint64_t b,
			uint64_t* out) noexcept
	{
		return Spec_SUBBORROW_U64(borrowIn, a, b, out);
	}

	// =========================================================
	// Signed Overflow Detection (Add / Sub)
	// =========================================================
	// NOTE: These intrinsics REQUIRE carry/borrow input.
	//       Do NOT drop it.

	SPECTRA_FORCEINLINE
		bool addWithOverflowDetection(
			uint8_t carryIn,
			int32_t a,
			int32_t b,
			int32_t* out) noexcept
	{
		return Spec_ADD_OVERFLOW_I32(carryIn, a, b, out) != 0;
	}

	SPECTRA_FORCEINLINE
		bool addWithOverflowDetection(
			uint8_t carryIn,
			int64_t a,
			int64_t b,
			int64_t* out) noexcept
	{
		return Spec_ADD_OVERFLOW_I64(carryIn, a, b, out) != 0;
	}

	// Convenience overloads (carryIn = 0)

	SPECTRA_FORCEINLINE
		bool addWithOverflowDetection(
			int32_t a,
			int32_t b,
			int32_t* out) noexcept
	{
		return addWithOverflowDetection(0, a, b, out);
	}

	SPECTRA_FORCEINLINE
		bool addWithOverflowDetection(
			int64_t a,
			int64_t b,
			int64_t* out) noexcept
	{
		return addWithOverflowDetection(0, a, b, out);
	}

	SPECTRA_FORCEINLINE
		bool subtractWithOverflowDetection(
			uint8_t borrowIn,
			int32_t a,
			int32_t b,
			int32_t* out) noexcept
	{
		return Spec_SUB_OVERFLOW_I32(borrowIn, a, b, out) != 0;
	}

	SPECTRA_FORCEINLINE
		bool subtractWithOverflowDetection(
			uint8_t borrowIn,
			int64_t a,
			int64_t b,
			int64_t* out) noexcept
	{
		return Spec_SUB_OVERFLOW_I64(borrowIn, a, b, out) != 0;
	}

	// Convenience overloads (borrowIn = 0)

	SPECTRA_FORCEINLINE
		bool subtractWithOverflowDetection(
			int32_t a,
			int32_t b,
			int32_t* out) noexcept
	{
		return subtractWithOverflowDetection(0, a, b, out);
	}

	SPECTRA_FORCEINLINE
		bool subtractWithOverflowDetection(
			int64_t a,
			int64_t b,
			int64_t* out) noexcept
	{
		return subtractWithOverflowDetection(0, a, b, out);
	}

	// =========================================================
	// Multiplication Overflow Detection
	// =========================================================
	// NOTE: These DO NOT take carry-in.

	SPECTRA_FORCEINLINE
		bool multiplyWithOverflowDetection(
			int32_t a,
			int32_t b,
			int32_t* out) noexcept
	{
		return Spec_MUL_OVERFLOW_I32(a, b, out) != 0;
	}

	SPECTRA_FORCEINLINE
		bool multiplyWithOverflowDetection(
			int64_t a,
			int64_t b,
			int64_t* out) noexcept
	{
		return Spec_MUL_OVERFLOW_I64(a, b, out) != 0;
	}

	// =========================================================
	// Full-Width Multiplication
	// =========================================================

	SPECTRA_FORCEINLINE
		bool multiplyFullWidth(
			int64_t a,
			int64_t b,
			int64_t* lo,
			int64_t* hi) noexcept
	{
		return Spec_MUL_FULL_OVERFLOW_I64(a, b, lo, hi) != 0;
	}

	SPECTRA_FORCEINLINE
		bool multiplyFullWidth(
			uint64_t a,
			uint64_t b,
			uint64_t* lo,
			uint64_t* hi) noexcept
	{
		return Spec_MUL_FULL_OVERFLOW_U64(a, b, lo, hi) != 0;
	}


	// =========================================================
	// Saturating Arithmetic
	// =========================================================

	SPECTRA_FORCEINLINE
		int32_t saturatingAdd(int32_t a, int32_t b) noexcept
	{
		return Spec_SAT_ADD_I32(a, b);
	}

	SPECTRA_FORCEINLINE
		int32_t saturatingSubtract(int32_t a, int32_t b) noexcept
	{
		return Spec_SAT_SUB_I32(a, b);
	}

	SPECTRA_FORCEINLINE
		uint32_t saturatingAdd(uint32_t a, uint32_t b) noexcept
	{
		return Spec_SAT_ADD_U32(a, b);
	}

	SPECTRA_FORCEINLINE
		uint32_t saturatingSubtract(uint32_t a, uint32_t b) noexcept
	{
		return Spec_SAT_SUB_U32(a, b);
	}

	// =========================================================
	// CRC32
	// =========================================================

	SPECTRA_FORCEINLINE
		uint32_t computeCRC32(uint32_t crc, uint8_t data) noexcept
	{
		return Spec_CRC32_U8(crc, data);
	}

	SPECTRA_FORCEINLINE
		uint32_t computeCRC32(uint32_t crc, uint16_t data) noexcept
	{
		return Spec_CRC32_U16(crc, data);
	}

	SPECTRA_FORCEINLINE
		uint32_t computeCRC32(uint32_t crc, uint32_t data) noexcept
	{
		return Spec_CRC32_U32(crc, data);
	}

	SPECTRA_FORCEINLINE
		uint32_t computeCRC32(uint64_t crc, uint64_t data) noexcept
	{
		return Spec_CRC32_U64(crc, data);
	}

	// =========================================================
	// Control-Flow Introspection
	// =========================================================

	SPECTRA_FORCEINLINE
		void* getReturnAddress() noexcept
	{
		return Spec_RETURN_ADDRESS();
	}

	SPECTRA_FORCEINLINE
		void* getAddressOfReturnAddress() noexcept
	{
		return Spec_ADDRESS_OF_RETURN_ADDRESS();
	}

	SPECTRA_FORCEINLINE
		void* getAddressOfNextInstruction() noexcept
	{
		return Spec_ADDRESS_OF_NEXT_INSTRUCTION();
	}

} 