/*
* File: SpectraIntrin.h
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
#include <SpectraCompiler.h>


namespace Spectra::Platform::Internal {
    // These macros do not respect namespaces.
	// They are placed here purely for organizational sanity.
	
	// Non-atomic bit test helpers.

#ifndef Spec_BITTEST32
#if SPECTRA_COMPILER_MSVC

#define Spec_BITTEST32(p_Ptr, bit) \
    _bittest(reinterpret_cast<long const*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_BITTEST64
#if SPECTRA_COMPILER_MSVC

#define Spec_BITTEST64(p_Ptr, bit) \
    _bittest64(reinterpret_cast<long long const*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_BITTEST_AND_SET32
#if SPECTRA_COMPILER_MSVC

#define Spec_BITTEST_AND_SET32(p_Ptr, bit) \
    _bittestandset(reinterpret_cast<long*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_BITTEST_AND_SET64
#if SPECTRA_COMPILER_MSVC
#define Spec_BITTEST_AND_SET64(p_Ptr, bit) \
    _bittestandset64(reinterpret_cast<long long*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_BITTEST_AND_RESET32
#if SPECTRA_COMPILER_MSVC
#define Spec_BITTEST_AND_RESET32(p_Ptr, bit) \
    _bittestandreset(reinterpret_cast<long*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_BITTEST_AND_RESET64
#if SPECTRA_COMPILER_MSVC

#define Spec_BITTEST_AND_RESET64(p_Ptr, bit) \
    _bittestandreset64(reinterpret_cast<long long*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_BITTEST_AND_COMPLEMENT32
#if SPECTRA_COMPILER_MSVC

#define Spec_BITTEST_AND_COMPLEMENT32(p_Ptr, bit) \
    _bittestandcomplement(reinterpret_cast<long*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_BITTEST_AND_COMPLEMENT64
#if SPECTRA_COMPILER_MSVC

    // Atomic bit test and modify operations.

#define Spec_BITTEST_AND_COMPLEMENT64(p_Ptr, bit) \
    _bittestandcomplement64(reinterpret_cast<long long*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_INTERLOCKED_BITTEST_AND_SET
#if SPECTRA_COMPILER_MSVC

#define Spec_INTERLOCKED_BITTEST_AND_SET(p_Ptr, bit) \
    _interlockedbittestandset(reinterpret_cast<long volatile*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_INTERLOCKED_BITTEST_AND_RESET
#if SPECTRA_COMPILER_MSVC

#define Spec_INTERLOCKED_BITTEST_AND_RESET(p_Ptr, bit) \
    _interlockedbittestandreset(reinterpret_cast<long volatile*>(p_Ptr), (bit))

#endif
#endif

    // Bit scan helpers.
	// Return index of least / most significant set bit.

#ifndef Spec_BITSCAN_FORWARD32
#if SPECTRA_COMPILER_MSVC

#define Spec_BITSCAN_FORWARD32(outIndex, value) \
    _BitScanForward((outIndex), static_cast<unsigned long>(value))

#endif
#endif

#ifndef Spec_BITSCAN_FORWARD64
#if SPECTRA_COMPILER_MSVC

#define Spec_BITSCAN_FORWARD64(outIndex, value) \
    _BitScanForward64((outIndex), static_cast<unsigned long long>(value))

#endif
#endif

#ifndef Spec_BITSCAN_REVERSE32
#if SPECTRA_COMPILER_MSVC

#define Spec_BITSCAN_REVERSE32(outIndex, value) \
    _BitScanReverse((outIndex), static_cast<unsigned long>(value))

#endif
#endif

#ifndef Spec_BITSCAN_REVERSE64
#if SPECTRA_COMPILER_MSVC

#define Spec_BITSCAN_REVERSE64(outIndex, value) \
    _BitScanReverse64((outIndex), static_cast<unsigned long long>(value))

#endif
#endif

    // Bit population count.

#ifndef Spec_POPCOUNT32
#if SPECTRA_COMPILER_MSVC

#define Spec_POPCOUNT32(value) \
    __popcnt(static_cast<unsigned int>(value))

#endif
#endif

#ifndef Spec_POPCOUNT64
#if SPECTRA_COMPILER_MSVC

#define Spec_POPCOUNT64(value) \
    __popcnt64(static_cast<unsigned long long>(value))

#endif
#endif

    // Bit rotation helpers.

#ifndef Spec_ROTL32
#if SPECTRA_COMPILER_MSVC

#define Spec_ROTL32(value, shift) \
    _lrotl(static_cast<unsigned long>(value), (shift))

#endif
#endif

#ifndef Spec_ROTL64
#if SPECTRA_COMPILER_MSVC

#define Spec_ROTL64(value, shift) \
    _rotl64(static_cast<unsigned long long>(value), (shift))

#endif
#endif

#ifndef Spec_ROTR32
#if SPECTRA_COMPILER_MSVC

#define Spec_ROTR32(value, shift) \
    _lrotr(static_cast<unsigned long>(value), (shift))

#endif
#endif

#ifndef Spec_ROTR64
#if SPECTRA_COMPILER_MSVC

#define Spec_ROTR64(value, shift) \
    _rotr64(static_cast<unsigned long long>(value), (shift))

#endif
#endif
    
	// Byte-order reversal helpers.

#ifndef Spec_BYTESWAP16
#if SPECTRA_COMPILER_MSVC

#define Spec_BYTESWAP16(value) \
    _byteswap_ushort(static_cast<unsigned short>(value))

#endif
#endif

#ifndef Spec_BYTESWAP32
#if SPECTRA_COMPILER_MSVC

#define Spec_BYTESWAP32(value) \
    _byteswap_ulong(static_cast<unsigned long>(value))

#endif
#endif

#ifndef Spec_BYTESWAP64
#if SPECTRA_COMPILER_MSVC

#define Spec_BYTESWAP64(value) \
    _byteswap_uint64(static_cast<unsigned long long>(value))

#endif
#endif

    // CPU timestamp counter access.

#ifndef Spec_RDTSC
#if SPECTRA_COMPILER_MSVC

#define Spec_RDTSC() \
    __rdtsc()

#endif
#endif

    // Serialized timestamp counter access.

#ifndef Spec_RDTSCP
#if SPECTRA_COMPILER_MSVC

#define Spec_RDTSCP(aux) \
    __rdtscp((aux))

#endif
#endif

    // Performance monitoring counter access.

#ifndef Spec_READPMC
#if SPECTRA_COMPILER_MSVC

#define Spec_READPMC(counter) \
    __readpmc((counter))

#endif
#endif

    // CPU pause hint for spin-wait loops

#ifndef Spec_CPU_PAUSE
#if SPECTRA_COMPILER_MSVC

#define Spec_CPU_PAUSE() \
    _mm_pause()

#endif
#endif

#if SPECTRA_COMPILER_CLANG
	// clang-cl defines _MSC_VER but its <intrin.h> lacks these MSVC-only intrinsics.
	// Same signatures and results as MSVC (verified against it): a nonzero carry-in
	// counts as 1, and the full-width multiplies write lo to the 3rd arg, hi to the 4th.
	namespace ClangIntrin {
		template<typename T, typename W>
		inline unsigned char addCarry(unsigned char v_C, T v_A, T v_B, T* p_Out) {
			const W s = W(v_A) + W(v_B) + W(v_C != 0);
			*p_Out = T(s);
			return (unsigned char)(s >> (sizeof(T) * 8));
		}

		template<typename T, typename W>
		inline unsigned char subBorrow(unsigned char v_C, T v_A, T v_B, T* p_Out) {
			const W d = W(v_A) - W(v_B) - W(v_C != 0);
			*p_Out = T(d);
			return (unsigned char)(W(v_A) < W(v_B) + W(v_C != 0));
		}

		template<typename T>
		inline unsigned char addOverflow(unsigned char v_C, T v_A, T v_B, T* p_Out) {
			const __int128 s = __int128(v_A) + __int128(v_B) + __int128(v_C != 0);
			*p_Out = T(s);
			return (unsigned char)(s != __int128(T(s)));
		}

		template<typename T>
		inline unsigned char subOverflow(unsigned char v_C, T v_A, T v_B, T* p_Out) {
			const __int128 d = __int128(v_A) - __int128(v_B) - __int128(v_C != 0);
			*p_Out = T(d);
			return (unsigned char)(d != __int128(T(d)));
		}

		template<typename T>
		inline unsigned char mulOverflow(T v_A, T v_B, T* p_Out) {
			const __int128 p = __int128(v_A) * __int128(v_B);
			*p_Out = T(p);
			return (unsigned char)(p != __int128(T(p)));
		}

		template<typename T, typename W>
		inline unsigned char mulFullNarrow(T v_A, T v_B, W* p_Out) {
			const W p = W(W(v_A) * W(v_B));
			*p_Out = p;
			return (unsigned char)(p != W(T(p)));
		}

		template<typename T, typename W>
		inline unsigned char mulFull(T v_A, T v_B, T* p_Lo, T* p_Hi) {
			const W p = W(v_A) * W(v_B);
			*p_Lo = T(p);
			*p_Hi = T(p >> (sizeof(T) * 8));
			return (unsigned char)(p != W(T(p)));
		}

		template<typename T>
		inline T satAdd(T v_A, T v_B) {
			T r;
			if (!__builtin_add_overflow(v_A, v_B, &r)) return r;
			if constexpr (T(-1) < T(0)) return v_B < 0 ? T(T(1) << (sizeof(T) * 8 - 1)) : T(~(T(1) << (sizeof(T) * 8 - 1)));
			else return T(~T(0));
		}

		template<typename T>
		inline T satSub(T v_A, T v_B) {
			T r;
			if (!__builtin_sub_overflow(v_A, v_B, &r)) return r;
			if constexpr (T(-1) < T(0)) return v_B > 0 ? T(T(1) << (sizeof(T) * 8 - 1)) : T(~(T(1) << (sizeof(T) * 8 - 1)));
			else return T(0);
		}

		// Non-inlined so the return address is the instruction after the call site.
		[[gnu::noinline]] inline void* addressOfNextInstruction() {
			return __builtin_return_address(0);
		}
	}

#define SPEC_CLANG_INTRIN ::Spectra::Platform::Internal::ClangIntrin
#define Spec_ADDCARRY_U8(c, a, b, out)   SPEC_CLANG_INTRIN::addCarry<unsigned char, unsigned int>((c), (a), (b), (out))
#define Spec_ADDCARRY_U16(c, a, b, out)  SPEC_CLANG_INTRIN::addCarry<unsigned short, unsigned int>((c), (a), (b), (out))
#define Spec_SUBBORROW_U8(c, a, b, out)  SPEC_CLANG_INTRIN::subBorrow<unsigned char, int>((c), (a), (b), (out))
#define Spec_SUBBORROW_U16(c, a, b, out) SPEC_CLANG_INTRIN::subBorrow<unsigned short, int>((c), (a), (b), (out))

#define Spec_ADD_OVERFLOW_I8(c, a, b, out)  SPEC_CLANG_INTRIN::addOverflow<signed char>((c), (a), (b), (out))
#define Spec_ADD_OVERFLOW_I16(c, a, b, out) SPEC_CLANG_INTRIN::addOverflow<short>((c), (a), (b), (out))
#define Spec_ADD_OVERFLOW_I32(c, a, b, out) SPEC_CLANG_INTRIN::addOverflow<int>((c), (a), (b), (out))
#define Spec_ADD_OVERFLOW_I64(c, a, b, out) SPEC_CLANG_INTRIN::addOverflow<long long>((c), (a), (b), (out))
#define Spec_SUB_OVERFLOW_I8(c, a, b, out)  SPEC_CLANG_INTRIN::subOverflow<signed char>((c), (a), (b), (out))
#define Spec_SUB_OVERFLOW_I16(c, a, b, out) SPEC_CLANG_INTRIN::subOverflow<short>((c), (a), (b), (out))
#define Spec_SUB_OVERFLOW_I32(c, a, b, out) SPEC_CLANG_INTRIN::subOverflow<int>((c), (a), (b), (out))
#define Spec_SUB_OVERFLOW_I64(c, a, b, out) SPEC_CLANG_INTRIN::subOverflow<long long>((c), (a), (b), (out))

#define Spec_MUL_OVERFLOW_I16(a, b, out) SPEC_CLANG_INTRIN::mulOverflow<short>((a), (b), (out))
#define Spec_MUL_OVERFLOW_I32(a, b, out) SPEC_CLANG_INTRIN::mulOverflow<int>((a), (b), (out))
#define Spec_MUL_OVERFLOW_I64(a, b, out) SPEC_CLANG_INTRIN::mulOverflow<long long>((a), (b), (out))

#define Spec_MUL_FULL_OVERFLOW_I8(a, b, out)       SPEC_CLANG_INTRIN::mulFullNarrow<signed char, short>((a), (b), (out))
#define Spec_MUL_FULL_OVERFLOW_U8(a, b, out)       SPEC_CLANG_INTRIN::mulFullNarrow<unsigned char, unsigned short>((a), (b), (out))
#define Spec_MUL_FULL_OVERFLOW_I16(a, b, lo, hi)   SPEC_CLANG_INTRIN::mulFull<short, int>((a), (b), (lo), (hi))
#define Spec_MUL_FULL_OVERFLOW_I32(a, b, lo, hi)   SPEC_CLANG_INTRIN::mulFull<int, long long>((a), (b), (lo), (hi))
#define Spec_MUL_FULL_OVERFLOW_I64(a, b, lo, hi)   SPEC_CLANG_INTRIN::mulFull<long long, __int128>((a), (b), (lo), (hi))
#define Spec_MUL_FULL_OVERFLOW_U16(a, b, lo, hi)   SPEC_CLANG_INTRIN::mulFull<unsigned short, unsigned int>((a), (b), (lo), (hi))
#define Spec_MUL_FULL_OVERFLOW_U32(a, b, lo, hi)   SPEC_CLANG_INTRIN::mulFull<unsigned int, unsigned long long>((a), (b), (lo), (hi))
#define Spec_MUL_FULL_OVERFLOW_U64(a, b, lo, hi)   SPEC_CLANG_INTRIN::mulFull<unsigned long long, unsigned __int128>((a), (b), (lo), (hi))

#define Spec_SAT_ADD_I8(a, b)  SPEC_CLANG_INTRIN::satAdd<signed char>((a), (b))
#define Spec_SAT_ADD_I16(a, b) SPEC_CLANG_INTRIN::satAdd<short>((a), (b))
#define Spec_SAT_ADD_I32(a, b) SPEC_CLANG_INTRIN::satAdd<int>((a), (b))
#define Spec_SAT_ADD_I64(a, b) SPEC_CLANG_INTRIN::satAdd<long long>((a), (b))
#define Spec_SAT_ADD_U8(a, b)  SPEC_CLANG_INTRIN::satAdd<unsigned char>((a), (b))
#define Spec_SAT_ADD_U16(a, b) SPEC_CLANG_INTRIN::satAdd<unsigned short>((a), (b))
#define Spec_SAT_ADD_U32(a, b) SPEC_CLANG_INTRIN::satAdd<unsigned int>((a), (b))
#define Spec_SAT_ADD_U64(a, b) SPEC_CLANG_INTRIN::satAdd<unsigned long long>((a), (b))
#define Spec_SAT_SUB_I8(a, b)  SPEC_CLANG_INTRIN::satSub<signed char>((a), (b))
#define Spec_SAT_SUB_I16(a, b) SPEC_CLANG_INTRIN::satSub<short>((a), (b))
#define Spec_SAT_SUB_I32(a, b) SPEC_CLANG_INTRIN::satSub<int>((a), (b))
#define Spec_SAT_SUB_I64(a, b) SPEC_CLANG_INTRIN::satSub<long long>((a), (b))
#define Spec_SAT_SUB_U8(a, b)  SPEC_CLANG_INTRIN::satSub<unsigned char>((a), (b))
#define Spec_SAT_SUB_U16(a, b) SPEC_CLANG_INTRIN::satSub<unsigned short>((a), (b))
#define Spec_SAT_SUB_U32(a, b) SPEC_CLANG_INTRIN::satSub<unsigned int>((a), (b))
#define Spec_SAT_SUB_U64(a, b) SPEC_CLANG_INTRIN::satSub<unsigned long long>((a), (b))

#define Spec_ADDRESS_OF_NEXT_INSTRUCTION() SPEC_CLANG_INTRIN::addressOfNextInstruction()
#endif

    // Integer carry and borrow helpers.
	// Used for multi-precision arithmetic.

#ifndef Spec_ADDCARRY_U8
#if SPECTRA_COMPILER_MSVC
#define Spec_ADDCARRY_U8(c, a, b, out) \
            _addcarry_u8((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_ADDCARRY_U16
#if SPECTRA_COMPILER_MSVC
#define Spec_ADDCARRY_U16(c, a, b, out) \
            _addcarry_u16((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_ADDCARRY_U32
#if SPECTRA_COMPILER_MSVC
#define Spec_ADDCARRY_U32(c, a, b, out) \
            _addcarry_u32((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_ADDCARRY_U64
#if SPECTRA_COMPILER_MSVC
#define Spec_ADDCARRY_U64(c, a, b, out) \
            _addcarry_u64((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUBBORROW_U8
#if SPECTRA_COMPILER_MSVC
#define Spec_SUBBORROW_U8(c, a, b, out) \
            _subborrow_u8((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUBBORROW_U16
#if SPECTRA_COMPILER_MSVC
#define Spec_SUBBORROW_U16(c, a, b, out) \
            _subborrow_u16((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUBBORROW_U32
#if SPECTRA_COMPILER_MSVC
#define Spec_SUBBORROW_U32(c, a, b, out) \
            _subborrow_u32((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUBBORROW_U64
#if SPECTRA_COMPILER_MSVC
#define Spec_SUBBORROW_U64(c, a, b, out) \
            _subborrow_u64((c), (a), (b), (out))
#endif
#endif

    // Signed overflow-detecting arithmetic.

#ifndef Spec_ADD_OVERFLOW_I8
#if SPECTRA_COMPILER_MSVC
#define Spec_ADD_OVERFLOW_I8(c, a, b, out)  \
	_add_overflow_i8((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_ADD_OVERFLOW_I16
#if SPECTRA_COMPILER_MSVC
#define Spec_ADD_OVERFLOW_I16(c, a, b, out) \
	_add_overflow_i16((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_ADD_OVERFLOW_I32
#if SPECTRA_COMPILER_MSVC
#define Spec_ADD_OVERFLOW_I32(c, a, b, out)  \
	_add_overflow_i32((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_ADD_OVERFLOW_I64
#if SPECTRA_COMPILER_MSVC
#define Spec_ADD_OVERFLOW_I64(c, a, b, out) \
	_add_overflow_i64((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUB_OVERFLOW_I8
#if SPECTRA_COMPILER_MSVC
#define Spec_SUB_OVERFLOW_I8(c, a, b, out)   _sub_overflow_i8((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUB_OVERFLOW_I16
#if SPECTRA_COMPILER_MSVC
#define Spec_SUB_OVERFLOW_I16(c, a, b, out)  _sub_overflow_i16((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUB_OVERFLOW_I32
#if SPECTRA_COMPILER_MSVC
#define Spec_SUB_OVERFLOW_I32(c, a, b, out)  _sub_overflow_i32((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUB_OVERFLOW_I64
#if SPECTRA_COMPILER_MSVC
#define Spec_SUB_OVERFLOW_I64(c, a, b, out)  _sub_overflow_i64((c), (a), (b), (out))
#endif
#endif

    // Full-width multiplication helpers.

#ifndef Spec_MUL_OVERFLOW_I16
#if SPECTRA_COMPILER_MSVC
#define Spec_MUL_OVERFLOW_I16(a, b, out) \
            _mul_overflow_i16((a), (b), (out))
#endif
#endif

#ifndef Spec_MUL_OVERFLOW_I32
#if SPECTRA_COMPILER_MSVC
#define Spec_MUL_OVERFLOW_I32(a, b, out) \
            _mul_overflow_i32((a), (b), (out))
#endif
#endif

#ifndef Spec_MUL_OVERFLOW_I64
#if SPECTRA_COMPILER_MSVC
#define Spec_MUL_OVERFLOW_I64(a, b, out) \
            _mul_overflow_i64((a), (b), (out))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_I8
#if SPECTRA_COMPILER_MSVC
#define Spec_MUL_FULL_OVERFLOW_I8(a, b,  out) \
            _mul_full_overflow_i8((a), (b), (out))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_I16
#if SPECTRA_COMPILER_MSVC
#define Spec_MUL_FULL_OVERFLOW_I16(a, b, lo, hi) \
            _mul_full_overflow_i16((a), (b), (lo), (hi))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_I32
#if SPECTRA_COMPILER_MSVC
#define Spec_MUL_FULL_OVERFLOW_I32(a, b, lo, hi) \
            _mul_full_overflow_i32((a), (b), (lo), (hi))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_I64
#if SPECTRA_COMPILER_MSVC
#define Spec_MUL_FULL_OVERFLOW_I64(a, b, lo, hi) \
            _mul_full_overflow_i64((a), (b), (lo), (hi))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_U8
#if SPECTRA_COMPILER_MSVC
#define Spec_MUL_FULL_OVERFLOW_U8(a, b, out) \
            _mul_full_overflow_u8((a), (b), (out))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_U16
#if SPECTRA_COMPILER_MSVC
#define Spec_MUL_FULL_OVERFLOW_U16(a, b, lo, hi) \
            _mul_full_overflow_u16((a), (b), (lo), (hi))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_U32
#if SPECTRA_COMPILER_MSVC
#define Spec_MUL_FULL_OVERFLOW_U32(a, b, lo, hi) \
            _mul_full_overflow_u32((a), (b), (lo), (hi))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_U64
#if SPECTRA_COMPILER_MSVC
#define Spec_MUL_FULL_OVERFLOW_U64(a, b, lo, hi) \
            _mul_full_overflow_u64((a), (b), (lo), (hi))
#endif
#endif

    // Saturating arithmetic helpers.
	// Clamp results on overflow instead of wrapping.

#ifndef Spec_SAT_ADD_I8
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_ADD_I8(a, b) \
            _sat_add_i8((a), (b))
#endif
#endif

#ifndef Spec_SAT_ADD_I16
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_ADD_I16(a, b) \
            _sat_add_i16((a), (b))
#endif
#endif

#ifndef Spec_SAT_ADD_I32
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_ADD_I32(a, b) \
            _sat_add_i32((a), (b))
#endif
#endif

#ifndef Spec_SAT_ADD_I64
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_ADD_I64(a, b) \
            _sat_add_i64((a), (b))
#endif
#endif

#ifndef Spec_SAT_ADD_U8
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_ADD_U8(a, b) \
            _sat_add_u8((a), (b))
#endif
#endif

#ifndef Spec_SAT_ADD_U16
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_ADD_U16(a, b) \
            _sat_add_u16((a), (b))
#endif
#endif

#ifndef Spec_SAT_ADD_U32
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_ADD_U32(a, b) \
            _sat_add_u32((a), (b))
#endif
#endif

#ifndef Spec_SAT_ADD_U64
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_ADD_U64(a, b) \
            _sat_add_u64((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_I8
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_SUB_I8(a, b) \
            _sat_sub_i8((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_I16
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_SUB_I16(a, b) \
            _sat_sub_i16((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_I32
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_SUB_I32(a, b) \
            _sat_sub_i32((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_I64
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_SUB_I64(a, b) \
            _sat_sub_i64((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_U8
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_SUB_U8(a, b) \
            _sat_sub_u8((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_U16
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_SUB_U16(a, b) \
            _sat_sub_u16((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_U32
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_SUB_U32(a, b) \
            _sat_sub_u32((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_U64
#if SPECTRA_COMPILER_MSVC
#define Spec_SAT_SUB_U64(a, b) \
            _sat_sub_u64((a), (b))
#endif
#endif

	// Hardware-accelerated CRC32 helpers (SSE4.2).

#ifndef Spec_CRC32_U8
#if SPECTRA_COMPILER_MSVC
#define Spec_CRC32_U8(crc, data) \
            _mm_crc32_u8((crc), (data))
#endif
#endif

#ifndef Spec_CRC32_U16
#if SPECTRA_COMPILER_MSVC
#define Spec_CRC32_U16(crc, data) \
            _mm_crc32_u16((crc), (data))
#endif
#endif

#ifndef Spec_CRC32_U32
#if SPECTRA_COMPILER_MSVC
#define Spec_CRC32_U32(crc, data) \
            _mm_crc32_u32((crc), (data))
#endif
#endif

#ifndef Spec_CRC32_U64
#if SPECTRA_COMPILER_MSVC
#define Spec_CRC32_U64(crc, data) \
            _mm_crc32_u64((crc), (data))
#endif
#endif

    // Low-level control-flow inspection helpers.
	// ABI- and optimizer-sensitive; use with care.

#ifndef Spec_ADDRESS_OF_RETURN_ADDRESS
#if SPECTRA_COMPILER_MSVC
#define Spec_ADDRESS_OF_RETURN_ADDRESS() \
            _AddressOfReturnAddress()
#endif
#endif

#ifndef Spec_RETURN_ADDRESS
#if SPECTRA_COMPILER_MSVC
#define Spec_RETURN_ADDRESS() \
            _ReturnAddress()
#endif
#endif

#ifndef Spec_ADDRESS_OF_NEXT_INSTRUCTION
#if SPECTRA_COMPILER_MSVC
#define Spec_ADDRESS_OF_NEXT_INSTRUCTION() \
            _AddressOfNextInstruction()
#endif
#endif
}
