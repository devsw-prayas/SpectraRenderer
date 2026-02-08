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

// Begging myself to not touch this file again, this was a somewhat direct copy-paste from
// Corium, I'm lazy asf


namespace Spectra::Platform::Internal {
    // These macros do not respect namespaces.
	// They are placed here purely for organizational sanity.
	
	// Logical memory order definitions.
	// These mirror C++ memory_order semantics but are interpreted
	// by the PlatformRuntime implementation, not the compiler.

#define SPECTRA_MEMORY_ORDER_RELAXED   0
#define SPECTRA_MEMORY_ORDER_CONSUME   1  // treated as ACQUIRE on MSVC
#define SPECTRA_MEMORY_ORDER_ACQUIRE   2
#define SPECTRA_MEMORY_ORDER_RELEASE   3
#define SPECTRA_MEMORY_ORDER_ACQ_REL   4
#define SPECTRA_MEMORY_ORDER_SEQ_CST   5

	// Backend atomic order mapping.
	// On MSVC, these are currently placeholders for future ports.

#define SPECTRA_ATOMIC_RELAXED   __ATOMIC_RELAXED
#define SPECTRA_ATOMIC_CONSUME   __ATOMIC_CONSUME
#define SPECTRA_ATOMIC_ACQUIRE   __ATOMIC_ACQUIRE
#define SPECTRA_ATOMIC_RELEASE   __ATOMIC_RELEASE
#define SPECTRA_ATOMIC_ACQ_REL   __ATOMIC_ACQ_REL
#define SPECTRA_ATOMIC_SEQ_CST   __ATOMIC_SEQ_CST

	// Atomic load primitives.
	// Implemented using volatile loads + fences on MSVC/x86.
	// Memory ordering is approximated and may be strengthened later.

#ifndef Spec_ATOMIC_LOAD
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_ATOMIC_LOAD(p_Ptr, memOrder)                                   \
    (                                                                       \
        ((memOrder) == SPECTRA_MEMORY_ORDER_SEQ_CST) ?                      \
            ([&]() {                                                        \
                auto _v = *reinterpret_cast<volatile                        \
                    std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr);        \
                _mm_mfence();                                               \
                return _v;                                                  \
            }())                                                            \
        :                                                                   \
            (*reinterpret_cast<volatile                                     \
                std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr))            \
    )
#endif
#endif

#define Spec_ATOMIC_LOAD_RELAXED(p_Ptr)   \
    Spec_ATOMIC_LOAD((p_Ptr), SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_LOAD_CONSUME(p_Ptr)   \
    Spec_ATOMIC_LOAD((p_Ptr), SPECTRA_MEMORY_ORDER_CONSUME)

#define Spec_ATOMIC_LOAD_ACQUIRE(p_Ptr)   \
    Spec_ATOMIC_LOAD((p_Ptr), SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_LOAD_SEQ_CST(p_Ptr)   \
    Spec_ATOMIC_LOAD((p_Ptr), SPECTRA_MEMORY_ORDER_SEQ_CST)

	// Atomic store primitives.
	// Seq-cst uses explicit fences; weaker orders are currently best-effort.

#ifndef Spec_ATOMIC_STORE
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_STORE(p_Ptr, value, memOrder)                            \
    do                                                                       \
    {                                                                        \
        if ((memOrder) == SPECTRA_MEMORY_ORDER_SEQ_CST)                      \
        {                                                                    \
            *reinterpret_cast<volatile                                       \
                std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr) = (value);   \
            _mm_mfence();                                                    \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            *reinterpret_cast<volatile                                       \
                std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr) = (value);   \
        }                                                                    \
    } while (0)

#endif
#endif

#define Spec_ATOMIC_STORE_RELAXED(p_Ptr, value)   \
    Spec_ATOMIC_STORE((p_Ptr), (value), SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_STORE_RELEASE(p_Ptr, value)   \
    Spec_ATOMIC_STORE((p_Ptr), (value), SPECTRA_MEMORY_ORDER_RELEASE)

#define Spec_ATOMIC_STORE_ACQ_REL(p_Ptr, value)   \
    Spec_ATOMIC_STORE((p_Ptr), (value), SPECTRA_MEMORY_ORDER_ACQ_REL)

#define Spec_ATOMIC_STORE_SEQ_CST(p_Ptr, value)   \
    Spec_ATOMIC_STORE((p_Ptr), (value), SPECTRA_MEMORY_ORDER_SEQ_CST)

	// Compiler-only reordering barriers.
	// Prevent instruction reordering without emitting CPU fences.

#ifndef Spec_COMPILER_READ_BARRIER
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_COMPILER_READ_BARRIER()        _ReadBarrier()

#endif
#endif

#ifndef Spec_COMPILER_WRITE_BARRIER
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_COMPILER_WRITE_BARRIER()       _WriteBarrier()

#endif
#endif

#ifndef Spec_COMPILER_READWRITE_BARRIER
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_COMPILER_READWRITE_BARRIER()   _ReadWriteBarrier()

#endif
#endif

	// CPU memory fence primitives.
	// Map directly to architectural fence instructions.

#ifndef Spec_CPU_LOAD_FENCE
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_CPU_LOAD_FENCE()               _mm_lfence()

#endif
#endif

#ifndef Spec_CPU_STORE_FENCE
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_CPU_STORE_FENCE()              _mm_sfence()

#endif
#endif

#ifndef Spec_CPU_FULL_FENCE
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_CPU_FULL_FENCE()               _mm_mfence()

#endif
#endif

	// Fast store fence.
	// Used for write-combining and streaming-store scenarios.

#ifndef Spec_CPU_FAST_STORE_FENCE
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_CPU_FAST_STORE_FENCE()         __faststorefence()

#endif
#endif

	// Atomic arithmetic operations.
	// Backed by MSVC interlocked intrinsics (seq-cst on x86).
	// memOrder is currently ignored but preserved for API stability.

#ifndef Spec_ATOMIC_ADD
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_ADD(p_Ptr, value, memOrder)                              \
    (                                                                        \
        ((memOrder) == SPECTRA_MEMORY_ORDER_SEQ_CST) ?                       \
            ([&]() {                                                         \
                auto _r = _interlockedadd(                                   \
                    reinterpret_cast<long volatile*>(p_Ptr),                \
                    static_cast<long>(value));                               \
                _mm_mfence();                                                 \
                return _r;                                                    \
            }())                                                             \
        :                                                                    \
            _interlockedadd(                                                  \
                reinterpret_cast<long volatile*>(p_Ptr),                    \
                static_cast<long>(value))                                    \
    )

#endif
#endif

#define Spec_ATOMIC_ADD_RELAXED(p, v)    \
    Spec_ATOMIC_ADD((p), (v), SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_ADD_ACQUIRE(p, v)    \
    Spec_ATOMIC_ADD((p), (v), SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_ADD_RELEASE(p, v)    \
    Spec_ATOMIC_ADD((p), (v), SPECTRA_MEMORY_ORDER_RELEASE)

#define Spec_ATOMIC_ADD_ACQ_REL(p, v)    \
    Spec_ATOMIC_ADD((p), (v), SPECTRA_MEMORY_ORDER_ACQ_REL)

#define Spec_ATOMIC_ADD_SEQ_CST(p, v)    \
    Spec_ATOMIC_ADD((p), (v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_ADD64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_ADD64(p_Ptr, value, memOrder)                            \
    (                                                                        \
        ((memOrder) == SPECTRA_MEMORY_ORDER_SEQ_CST) ?                       \
            ([&]() {                                                         \
                auto _r = _interlockedadd64(                                 \
                    reinterpret_cast<__int64 volatile*>(p_Ptr),             \
                    static_cast<__int64>(value));                            \
                _mm_mfence();                                                 \
                return _r;                                                    \
            }())                                                             \
        :                                                                    \
            _interlockedadd64(                                                \
                reinterpret_cast<__int64 volatile*>(p_Ptr),                 \
                static_cast<__int64>(value))                                  \
    )

#endif
#endif

#define Spec_ATOMIC_ADD64_RELAXED(p, v)  \
    Spec_ATOMIC_ADD64((p), (v), SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_ADD64_ACQUIRE(p, v)  \
    Spec_ATOMIC_ADD64((p), (v), SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_ADD64_RELEASE(p, v)  \
    Spec_ATOMIC_ADD64((p), (v), SPECTRA_MEMORY_ORDER_RELEASE)

#define Spec_ATOMIC_ADD64_ACQ_REL(p, v)  \
    Spec_ATOMIC_ADD64((p), (v), SPECTRA_MEMORY_ORDER_ACQ_REL)

#define Spec_ATOMIC_ADD64_SEQ_CST(p, v)  \
    Spec_ATOMIC_ADD64((p), (v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_INCREMENT
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_INCREMENT(p_Ptr, memOrder)                               \
    (                                                                        \
        ((memOrder) == SPECTRA_MEMORY_ORDER_SEQ_CST) ?                       \
            ([&]() {                                                         \
                auto _r = _InterlockedIncrement(                             \
                    reinterpret_cast<long volatile*>(p_Ptr));               \
                _mm_mfence();                                                 \
                return _r;                                                    \
            }())                                                             \
        :                                                                    \
            _InterlockedIncrement(                                           \
                reinterpret_cast<long volatile*>(p_Ptr))                     \
    )

#endif
#endif

#define Spec_ATOMIC_INCREMENT_RELAXED(p) \
    Spec_ATOMIC_INCREMENT((p), SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_INCREMENT_ACQUIRE(p) \
    Spec_ATOMIC_INCREMENT((p), SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_INCREMENT_RELEASE(p) \
    Spec_ATOMIC_INCREMENT((p), SPECTRA_MEMORY_ORDER_RELEASE)

#define Spec_ATOMIC_INCREMENT_ACQ_REL(p) \
    Spec_ATOMIC_INCREMENT((p), SPECTRA_MEMORY_ORDER_ACQ_REL)

#define Spec_ATOMIC_INCREMENT_SEQ_CST(p) \
    Spec_ATOMIC_INCREMENT((p), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_INCREMENT64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_INCREMENT64(p_Ptr, memOrder)                             \
    (                                                                        \
        ((memOrder) == SPECTRA_MEMORY_ORDER_SEQ_CST) ?                       \
            ([&]() {                                                         \
                auto _r = _InterlockedIncrement64(                           \
                    reinterpret_cast<__int64 volatile*>(p_Ptr));            \
                _mm_mfence();                                                 \
                return _r;                                                    \
            }())                                                             \
        :                                                                    \
            _InterlockedIncrement64(                                         \
                reinterpret_cast<__int64 volatile*>(p_Ptr))                  \
    )

#endif
#endif

#define Spec_ATOMIC_INCREMENT64_RELAXED(p) \
    Spec_ATOMIC_INCREMENT64((p), SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_INCREMENT64_ACQUIRE(p) \
    Spec_ATOMIC_INCREMENT64((p), SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_INCREMENT64_RELEASE(p) \
    Spec_ATOMIC_INCREMENT64((p), SPECTRA_MEMORY_ORDER_RELEASE)

#define Spec_ATOMIC_INCREMENT64_ACQ_REL(p) \
    Spec_ATOMIC_INCREMENT64((p), SPECTRA_MEMORY_ORDER_ACQ_REL)

#define Spec_ATOMIC_INCREMENT64_SEQ_CST(p) \
    Spec_ATOMIC_INCREMENT64((p), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_DECREMENT
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_DECREMENT(p_Ptr, memOrder)                               \
    (                                                                        \
        ((memOrder) == SPECTRA_MEMORY_ORDER_SEQ_CST) ?                       \
            ([&]() {                                                         \
                auto _r = _InterlockedDecrement(                             \
                    reinterpret_cast<long volatile*>(p_Ptr));               \
                _mm_mfence();                                                 \
                return _r;                                                    \
            }())                                                             \
        :                                                                    \
            _InterlockedDecrement(                                           \
                reinterpret_cast<long volatile*>(p_Ptr))                     \
    )

#endif
#endif

#define Spec_ATOMIC_DECREMENT_RELAXED(p) \
    Spec_ATOMIC_DECREMENT((p), SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_DECREMENT_ACQUIRE(p) \
    Spec_ATOMIC_DECREMENT((p), SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_DECREMENT_RELEASE(p) \
    Spec_ATOMIC_DECREMENT((p), SPECTRA_MEMORY_ORDER_RELEASE)

#define Spec_ATOMIC_DECREMENT_ACQ_REL(p) \
    Spec_ATOMIC_DECREMENT((p), SPECTRA_MEMORY_ORDER_ACQ_REL)

#define Spec_ATOMIC_DECREMENT_SEQ_CST(p) \
    Spec_ATOMIC_DECREMENT((p), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_DECREMENT64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_DECREMENT64(p_Ptr, memOrder)                             \
    (                                                                        \
        ((memOrder) == SPECTRA_MEMORY_ORDER_SEQ_CST) ?                       \
            ([&]() {                                                         \
                auto _r = _InterlockedDecrement64(                           \
                    reinterpret_cast<__int64 volatile*>(p_Ptr));            \
                _mm_mfence();                                                 \
                return _r;                                                    \
            }())                                                             \
        :                                                                    \
            _InterlockedDecrement64(                                         \
                reinterpret_cast<__int64 volatile*>(p_Ptr))                  \
    )

#endif
#endif

#define Spec_ATOMIC_DECREMENT64_RELAXED(p) \
    Spec_ATOMIC_DECREMENT64((p), SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_DECREMENT64_ACQUIRE(p) \
    Spec_ATOMIC_DECREMENT64((p), SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_DECREMENT64_RELEASE(p) \
    Spec_ATOMIC_DECREMENT64((p), SPECTRA_MEMORY_ORDER_RELEASE)

#define Spec_ATOMIC_DECREMENT64_ACQ_REL(p) \
    Spec_ATOMIC_DECREMENT64((p), SPECTRA_MEMORY_ORDER_ACQ_REL)

#define Spec_ATOMIC_DECREMENT64_SEQ_CST(p) \
    Spec_ATOMIC_DECREMENT64((p), SPECTRA_MEMORY_ORDER_SEQ_CST)

	// Atomic exchange operations.
	// Return the previous value stored at the address.

#ifndef Spec_ATOMIC_EXCHANGE
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_EXCHANGE(p_Ptr, value, memOrder)                         \
    (                                                                        \
        ((memOrder) == SPECTRA_MEMORY_ORDER_SEQ_CST) ?                       \
            ([&]() {                                                         \
                auto _r = _InterlockedExchange(                              \
                    reinterpret_cast<long volatile*>(p_Ptr),                \
                    static_cast<long>(value));                               \
                _mm_mfence();                                                 \
                return _r;                                                    \
            }())                                                             \
        :                                                                    \
            _InterlockedExchange(                                            \
                reinterpret_cast<long volatile*>(p_Ptr),                    \
                static_cast<long>(value))                                     \
    )

#endif
#endif

#define Spec_ATOMIC_EXCHANGE_RELAXED(p, v)   \
    Spec_ATOMIC_EXCHANGE((p), (v), SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_EXCHANGE_ACQUIRE(p, v)   \
    Spec_ATOMIC_EXCHANGE((p), (v), SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_EXCHANGE_RELEASE(p, v)   \
    Spec_ATOMIC_EXCHANGE((p), (v), SPECTRA_MEMORY_ORDER_RELEASE)

#define Spec_ATOMIC_EXCHANGE_ACQ_REL(p, v)   \
    Spec_ATOMIC_EXCHANGE((p), (v), SPECTRA_MEMORY_ORDER_ACQ_REL)

#define Spec_ATOMIC_EXCHANGE_SEQ_CST(p, v)   \
    Spec_ATOMIC_EXCHANGE((p), (v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_EXCHANGE64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_EXCHANGE64(p_Ptr, value, memOrder)                       \
    (                                                                        \
        ((memOrder) == SPECTRA_MEMORY_ORDER_SEQ_CST) ?                       \
            ([&]() {                                                         \
                auto _r = _InterlockedExchange64(                            \
                    reinterpret_cast<__int64 volatile*>(p_Ptr),             \
                    static_cast<__int64>(value));                            \
                _mm_mfence();                                                 \
                return _r;                                                    \
            }())                                                             \
        :                                                                    \
            _InterlockedExchange64(                                          \
                reinterpret_cast<__int64 volatile*>(p_Ptr),                 \
                static_cast<__int64>(value))                                  \
    )

#endif
#endif

#define Spec_ATOMIC_EXCHANGE64_RELAXED(p, v) \
    Spec_ATOMIC_EXCHANGE64((p), (v), SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_EXCHANGE64_ACQUIRE(p, v) \
    Spec_ATOMIC_EXCHANGE64((p), (v), SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_EXCHANGE64_RELEASE(p, v) \
    Spec_ATOMIC_EXCHANGE64((p), (v), SPECTRA_MEMORY_ORDER_RELEASE)

#define Spec_ATOMIC_EXCHANGE64_ACQ_REL(p, v) \
    Spec_ATOMIC_EXCHANGE64((p), (v), SPECTRA_MEMORY_ORDER_ACQ_REL)

#define Spec_ATOMIC_EXCHANGE64_SEQ_CST(p, v) \
    Spec_ATOMIC_EXCHANGE64((p), (v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_EXCHANGE_PTR
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_EXCHANGE_PTR(p_Ptr, value, memOrder)                     \
    (                                                                        \
        ((memOrder) == SPECTRA_MEMORY_ORDER_SEQ_CST) ?                       \
            ([&]() {                                                         \
                auto _r = _InterlockedExchangePointer(                       \
                    reinterpret_cast<void* volatile*>(p_Ptr),               \
                    reinterpret_cast<void*>(value));                         \
                _mm_mfence();                                                 \
                return _r;                                                    \
            }())                                                             \
        :                                                                    \
            _InterlockedExchangePointer(                                     \
                reinterpret_cast<void* volatile*>(p_Ptr),                   \
                reinterpret_cast<void*>(value))                               \
    )

#endif
#endif

#define Spec_ATOMIC_EXCHANGE_PTR_RELAXED(p, v) \
    Spec_ATOMIC_EXCHANGE_PTR((p), (v), SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_EXCHANGE_PTR_ACQUIRE(p, v) \
    Spec_ATOMIC_EXCHANGE_PTR((p), (v), SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_EXCHANGE_PTR_RELEASE(p, v) \
    Spec_ATOMIC_EXCHANGE_PTR((p), (v), SPECTRA_MEMORY_ORDER_RELEASE)

#define Spec_ATOMIC_EXCHANGE_PTR_ACQ_REL(p, v) \
    Spec_ATOMIC_EXCHANGE_PTR((p), (v), SPECTRA_MEMORY_ORDER_ACQ_REL)

#define Spec_ATOMIC_EXCHANGE_PTR_SEQ_CST(p, v) \
    Spec_ATOMIC_EXCHANGE_PTR((p), (v), SPECTRA_MEMORY_ORDER_SEQ_CST)

	// Atomic compare-and-exchange.
	// Weak flag and memory orders are accepted for API parity,
	// but are currently ignored on MSVC.

#ifndef Spec_ATOMIC_COMPARE_EXCHANGE32
#define Spec_ATOMIC_COMPARE_EXCHANGE32

#define Spec_ATOMIC_COMPARE_EXCHANGE32(p_Ptr, expected, desired, weak, success_memOrder, failure_memOrder) \
    ([&]() {                                                                                                \
        long _prev = _InterlockedCompareExchange(                                                           \
            reinterpret_cast<long volatile*>(p_Ptr),                                                        \
            static_cast<long>(desired),                                                                     \
            static_cast<long>(*(expected)));                                                                \
        *(expected) = _prev;                                                                                \
        return _prev;                                                                                       \
    }())

#endif

#define Spec_ATOMIC_COMPARE_EXCHANGE32_RELAXED_RELAXED(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE32((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELAXED, SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_COMPARE_EXCHANGE32_RELAXED_ACQUIRE(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE32((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELAXED, SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_COMPARE_EXCHANGE32_RELAXED_SEQ_CST(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE32((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELAXED, SPECTRA_MEMORY_ORDER_SEQ_CST)

#define Spec_ATOMIC_COMPARE_EXCHANGE32_ACQUIRE_RELAXED(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE32((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQUIRE, SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_COMPARE_EXCHANGE32_ACQUIRE_ACQUIRE(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE32((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQUIRE, SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_COMPARE_EXCHANGE32_ACQUIRE_SEQ_CST(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE32((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQUIRE, SPECTRA_MEMORY_ORDER_SEQ_CST)

#define Spec_ATOMIC_COMPARE_EXCHANGE32_RELEASE_RELAXED(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE32((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELEASE, SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_COMPARE_EXCHANGE32_RELEASE_ACQUIRE(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE32((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELEASE, SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_COMPARE_EXCHANGE32_RELEASE_SEQ_CST(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE32((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELEASE, SPECTRA_MEMORY_ORDER_SEQ_CST)

#define Spec_ATOMIC_COMPARE_EXCHANGE32_ACQ_REL_RELAXED(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE32((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQ_REL, SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_COMPARE_EXCHANGE32_ACQ_REL_ACQUIRE(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE32((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQ_REL, SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_COMPARE_EXCHANGE32_ACQ_REL_SEQ_CST(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE32((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQ_REL, SPECTRA_MEMORY_ORDER_SEQ_CST)

#define Spec_ATOMIC_COMPARE_EXCHANGE32_SEQ_CST_RELAXED(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE32((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_SEQ_CST, SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_COMPARE_EXCHANGE32_SEQ_CST_ACQUIRE(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE32((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_SEQ_CST, SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_COMPARE_EXCHANGE32_SEQ_CST_SEQ_CST(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE32((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_SEQ_CST, SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_COMPARE_EXCHANGE64
#define Spec_ATOMIC_COMPARE_EXCHANGE64

#define Spec_ATOMIC_COMPARE_EXCHANGE64(p_Ptr, expected, desired, weak, success_memOrder, failure_memOrder) \
    ([&]() {                                                                                                \
        long long _prev = _InterlockedCompareExchange64(                                                    \
            reinterpret_cast<long long volatile*>(p_Ptr),                                                   \
            static_cast<long long>(desired),                                                                \
            static_cast<long long>(*(expected)));                                                          \
        *(expected) = _prev;                                                                                \
        return _prev;                                                                                       \
    }())

#endif

#define Spec_ATOMIC_COMPARE_EXCHANGE64_RELAXED_RELAXED(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE64((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELAXED, SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_COMPARE_EXCHANGE64_RELAXED_ACQUIRE(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE64((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELAXED, SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_COMPARE_EXCHANGE64_RELAXED_SEQ_CST(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE64((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELAXED, SPECTRA_MEMORY_ORDER_SEQ_CST)

#define Spec_ATOMIC_COMPARE_EXCHANGE64_ACQUIRE_RELAXED(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE64((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQUIRE, SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_COMPARE_EXCHANGE64_ACQUIRE_ACQUIRE(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE64((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQUIRE, SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_COMPARE_EXCHANGE64_ACQUIRE_SEQ_CST(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE64((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQUIRE, SPECTRA_MEMORY_ORDER_SEQ_CST)

#define Spec_ATOMIC_COMPARE_EXCHANGE64_RELEASE_RELAXED(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE64((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELEASE, SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_COMPARE_EXCHANGE64_RELEASE_ACQUIRE(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE64((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELEASE, SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_COMPARE_EXCHANGE64_RELEASE_SEQ_CST(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE64((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELEASE, SPECTRA_MEMORY_ORDER_SEQ_CST)

#define Spec_ATOMIC_COMPARE_EXCHANGE64_ACQ_REL_RELAXED(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE64((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQ_REL, SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_COMPARE_EXCHANGE64_ACQ_REL_ACQUIRE(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE64((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQ_REL, SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_COMPARE_EXCHANGE64_ACQ_REL_SEQ_CST(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE64((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQ_REL, SPECTRA_MEMORY_ORDER_SEQ_CST)

#define Spec_ATOMIC_COMPARE_EXCHANGE64_SEQ_CST_RELAXED(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE64((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_SEQ_CST, SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_COMPARE_EXCHANGE64_SEQ_CST_ACQUIRE(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE64((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_SEQ_CST, SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_COMPARE_EXCHANGE64_SEQ_CST_SEQ_CST(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE64((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_SEQ_CST, SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_COMPARE_EXCHANGE_PTR
#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR(p_Ptr, expected, desired, weak, success_memOrder, failure_memOrder) \
    ([&]() {                                                                                                  \
        void* _prev = _InterlockedCompareExchangePointer(                                                     \
            reinterpret_cast<void* volatile*>(p_Ptr),                                                        \
            (desired),                                                                                       \
            *(expected));                                                                                   \
        *(expected) = _prev;                                                                                 \
        return _prev;                                                                                        \
    }())

#endif

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR_RELAXED_RELAXED(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE_PTR((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELAXED, SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR_RELAXED_ACQUIRE(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE_PTR((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELAXED, SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR_RELAXED_SEQ_CST(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE_PTR((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELAXED, SPECTRA_MEMORY_ORDER_SEQ_CST)

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR_ACQUIRE_RELAXED(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE_PTR((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQUIRE, SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR_ACQUIRE_ACQUIRE(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE_PTR((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQUIRE, SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR_ACQUIRE_SEQ_CST(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE_PTR((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQUIRE, SPECTRA_MEMORY_ORDER_SEQ_CST)

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR_RELEASE_RELAXED(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE_PTR((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELEASE, SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR_RELEASE_ACQUIRE(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE_PTR((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELEASE, SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR_RELEASE_SEQ_CST(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE_PTR((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_RELEASE, SPECTRA_MEMORY_ORDER_SEQ_CST)

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR_ACQ_REL_RELAXED(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE_PTR((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQ_REL, SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR_ACQ_REL_ACQUIRE(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE_PTR((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQ_REL, SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR_ACQ_REL_SEQ_CST(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE_PTR((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_ACQ_REL, SPECTRA_MEMORY_ORDER_SEQ_CST)

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR_SEQ_CST_RELAXED(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE_PTR((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_SEQ_CST, SPECTRA_MEMORY_ORDER_RELAXED)

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR_SEQ_CST_ACQUIRE(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE_PTR((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_SEQ_CST, SPECTRA_MEMORY_ORDER_ACQUIRE)

#define Spec_ATOMIC_COMPARE_EXCHANGE_PTR_SEQ_CST_SEQ_CST(p,e,d,w) \
    Spec_ATOMIC_COMPARE_EXCHANGE_PTR((p),(e),(d),(w), SPECTRA_MEMORY_ORDER_SEQ_CST, SPECTRA_MEMORY_ORDER_SEQ_CST)

	// Atomic bitwise operations (AND / OR / XOR / NAND).
	// Implemented using interlocked RMW instructions.

#ifndef Spec_ATOMIC_AND32
#define Spec_ATOMIC_AND32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_AND32(p_Ptr, value, memOrder) \
    _InterlockedAnd(reinterpret_cast<long volatile*>(p_Ptr), static_cast<long>(value))

#endif
#endif

#define Spec_ATOMIC_AND32_RELAXED(p,v)  Spec_ATOMIC_AND32((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_AND32_ACQUIRE(p,v)  Spec_ATOMIC_AND32((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_AND32_RELEASE(p,v)  Spec_ATOMIC_AND32((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_AND32_ACQ_REL(p,v)  Spec_ATOMIC_AND32((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_AND32_SEQ_CST(p,v)  Spec_ATOMIC_AND32((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_AND64
#define Spec_ATOMIC_AND64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_AND64(p_Ptr, value, memOrder) \
    _InterlockedAnd64(reinterpret_cast<long long volatile*>(p_Ptr), static_cast<long long>(value))

#endif
#endif

#define Spec_ATOMIC_AND64_RELAXED(p,v)  Spec_ATOMIC_AND64((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_AND64_ACQUIRE(p,v)  Spec_ATOMIC_AND64((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_AND64_RELEASE(p,v)  Spec_ATOMIC_AND64((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_AND64_ACQ_REL(p,v)  Spec_ATOMIC_AND64((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_AND64_SEQ_CST(p,v)  Spec_ATOMIC_AND64((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_OR32
#define Spec_ATOMIC_OR32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_OR32(p_Ptr, value, memOrder) \
    _InterlockedOr(reinterpret_cast<long volatile*>(p_Ptr), static_cast<long>(value))

#endif
#endif

#define Spec_ATOMIC_OR32_RELAXED(p,v)  Spec_ATOMIC_OR32((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_OR32_ACQUIRE(p,v)  Spec_ATOMIC_OR32((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_OR32_RELEASE(p,v)  Spec_ATOMIC_OR32((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_OR32_ACQ_REL(p,v)  Spec_ATOMIC_OR32((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_OR32_SEQ_CST(p,v)  Spec_ATOMIC_OR32((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_OR64
#define Spec_ATOMIC_OR64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_OR64(p_Ptr, value, memOrder) \
    _InterlockedOr64(reinterpret_cast<long long volatile*>(p_Ptr), static_cast<long long>(value))

#endif
#endif

#define Spec_ATOMIC_OR64_RELAXED(p,v)  Spec_ATOMIC_OR64((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_OR64_ACQUIRE(p,v)  Spec_ATOMIC_OR64((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_OR64_RELEASE(p,v)  Spec_ATOMIC_OR64((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_OR64_ACQ_REL(p,v)  Spec_ATOMIC_OR64((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_OR64_SEQ_CST(p,v)  Spec_ATOMIC_OR64((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_XOR32
#define Spec_ATOMIC_XOR32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_XOR32(p_Ptr, value, memOrder) \
    _InterlockedXor(reinterpret_cast<long volatile*>(p_Ptr), static_cast<long>(value))

#endif
#endif

#define Spec_ATOMIC_XOR32_RELAXED(p,v)  Spec_ATOMIC_XOR32((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_XOR32_ACQUIRE(p,v)  Spec_ATOMIC_XOR32((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_XOR32_RELEASE(p,v)  Spec_ATOMIC_XOR32((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_XOR32_ACQ_REL(p,v)  Spec_ATOMIC_XOR32((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_XOR32_SEQ_CST(p,v)  Spec_ATOMIC_XOR32((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_XOR64
#define Spec_ATOMIC_XOR64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_XOR64(p_Ptr, value, memOrder) \
    _InterlockedXor64(reinterpret_cast<long long volatile*>(p_Ptr), static_cast<long long>(value))

#endif
#endif

#define Spec_ATOMIC_XOR64_RELAXED(p,v)  Spec_ATOMIC_XOR64((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_XOR64_ACQUIRE(p,v)  Spec_ATOMIC_XOR64((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_XOR64_RELEASE(p,v)  Spec_ATOMIC_XOR64((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_XOR64_ACQ_REL(p,v)  Spec_ATOMIC_XOR64((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_XOR64_SEQ_CST(p,v)  Spec_ATOMIC_XOR64((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_NAND32
#define Spec_ATOMIC_NAND32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_NAND32(p_Ptr, value, memOrder) \
    _InterlockedNand(reinterpret_cast<long volatile*>(p_Ptr), static_cast<long>(value))

#endif
#endif

#define Spec_ATOMIC_NAND32_RELAXED(p,v)  Spec_ATOMIC_NAND32((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_NAND32_ACQUIRE(p,v)  Spec_ATOMIC_NAND32((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_NAND32_RELEASE(p,v)  Spec_ATOMIC_NAND32((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_NAND32_ACQ_REL(p,v)  Spec_ATOMIC_NAND32((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_NAND32_SEQ_CST(p,v)  Spec_ATOMIC_NAND32((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_NAND64
#define Spec_ATOMIC_NAND64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_NAND64(p_Ptr, value, memOrder) \
    _InterlockedNand64(reinterpret_cast<long long volatile*>(p_Ptr), static_cast<long long>(value))

#endif
#endif

#define Spec_ATOMIC_NAND64_RELAXED(p,v)  Spec_ATOMIC_NAND64((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_NAND64_ACQUIRE(p,v)  Spec_ATOMIC_NAND64((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_NAND64_RELEASE(p,v)  Spec_ATOMIC_NAND64((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_NAND64_ACQ_REL(p,v)  Spec_ATOMIC_NAND64((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_NAND64_SEQ_CST(p,v)  Spec_ATOMIC_NAND64((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

	// Atomic min/max operations.
	// These are non-standard extensions provided by MSVC.

#ifndef Spec_ATOMIC_MIN32
#define Spec_ATOMIC_MIN32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_MIN32(p_Ptr, value, memOrder) \
    _InterlockedMin(reinterpret_cast<long volatile*>(p_Ptr), static_cast<long>(value))

#endif
#endif

#define Spec_ATOMIC_MIN32_RELAXED(p,v)  Spec_ATOMIC_MIN32((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_MIN32_ACQUIRE(p,v)  Spec_ATOMIC_MIN32((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_MIN32_RELEASE(p,v)  Spec_ATOMIC_MIN32((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_MIN32_ACQ_REL(p,v)  Spec_ATOMIC_MIN32((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_MIN32_SEQ_CST(p,v)  Spec_ATOMIC_MIN32((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_MIN64
#define Spec_ATOMIC_MIN64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_MIN64(p_Ptr, value, memOrder) \
    _InterlockedMin64(reinterpret_cast<long long volatile*>(p_Ptr), static_cast<long long>(value))

#endif
#endif

#define Spec_ATOMIC_MIN64_RELAXED(p,v)  Spec_ATOMIC_MIN64((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_MIN64_ACQUIRE(p,v)  Spec_ATOMIC_MIN64((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_MIN64_RELEASE(p,v)  Spec_ATOMIC_MIN64((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_MIN64_ACQ_REL(p,v)  Spec_ATOMIC_MIN64((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_MIN64_SEQ_CST(p,v)  Spec_ATOMIC_MIN64((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_MAX32
#define Spec_ATOMIC_MAX32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_MAX32(p_Ptr, value, memOrder) \
    _InterlockedMax(reinterpret_cast<long volatile*>(p_Ptr), static_cast<long>(value))

#endif
#endif

#define Spec_ATOMIC_MAX32_RELAXED(p,v)  Spec_ATOMIC_MAX32((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_MAX32_ACQUIRE(p,v)  Spec_ATOMIC_MAX32((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_MAX32_RELEASE(p,v)  Spec_ATOMIC_MAX32((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_MAX32_ACQ_REL(p,v)  Spec_ATOMIC_MAX32((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_MAX32_SEQ_CST(p,v)  Spec_ATOMIC_MAX32((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_MAX64
#define Spec_ATOMIC_MAX64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_MAX64(p_Ptr, value, memOrder) \
    _InterlockedMax64(reinterpret_cast<long long volatile*>(p_Ptr), static_cast<long long>(value))

#endif
#endif

#define Spec_ATOMIC_MAX64_RELAXED(p,v)  Spec_ATOMIC_MAX64((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_MAX64_ACQUIRE(p,v)  Spec_ATOMIC_MAX64((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_MAX64_RELEASE(p,v)  Spec_ATOMIC_MAX64((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_MAX64_ACQ_REL(p,v)  Spec_ATOMIC_MAX64((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_MAX64_SEQ_CST(p,v)  Spec_ATOMIC_MAX64((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_UMIN32
#define Spec_ATOMIC_UMIN32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_UMIN32(p_Ptr, value, memOrder) \
    _InterlockedUMin(reinterpret_cast<unsigned long volatile*>(p_Ptr), static_cast<unsigned long>(value))

#endif
#endif

#define Spec_ATOMIC_UMIN32_RELAXED(p,v)  Spec_ATOMIC_UMIN32((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_UMIN32_ACQUIRE(p,v)  Spec_ATOMIC_UMIN32((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_UMIN32_RELEASE(p,v)  Spec_ATOMIC_UMIN32((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_UMIN32_ACQ_REL(p,v)  Spec_ATOMIC_UMIN32((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_UMIN32_SEQ_CST(p,v)  Spec_ATOMIC_UMIN32((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_UMIN64
#define Spec_ATOMIC_UMIN64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_UMIN64(p_Ptr, value, memOrder) \
    _InterlockedUMin64(reinterpret_cast<unsigned long long volatile*>(p_Ptr), static_cast<unsigned long long>(value))

#endif
#endif

#define Spec_ATOMIC_UMIN64_RELAXED(p,v)  Spec_ATOMIC_UMIN64((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_UMIN64_ACQUIRE(p,v)  Spec_ATOMIC_UMIN64((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_UMIN64_RELEASE(p,v)  Spec_ATOMIC_UMIN64((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_UMIN64_ACQ_REL(p,v)  Spec_ATOMIC_UMIN64((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_UMIN64_SEQ_CST(p,v)  Spec_ATOMIC_UMIN64((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_UMAX32
#define Spec_ATOMIC_UMAX32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_UMAX32(p_Ptr, value, memOrder) \
    _InterlockedUMax(reinterpret_cast<unsigned long volatile*>(p_Ptr), static_cast<unsigned long>(value))

#endif
#endif

#define Spec_ATOMIC_UMAX32_RELAXED(p,v)  Spec_ATOMIC_UMAX32((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_UMAX32_ACQUIRE(p,v)  Spec_ATOMIC_UMAX32((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_UMAX32_RELEASE(p,v)  Spec_ATOMIC_UMAX32((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_UMAX32_ACQ_REL(p,v)  Spec_ATOMIC_UMAX32((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_UMAX32_SEQ_CST(p,v)  Spec_ATOMIC_UMAX32((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

#ifndef Spec_ATOMIC_UMAX64
#define Spec_ATOMIC_UMAX64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ATOMIC_UMAX64(p_Ptr, value, memOrder) \
    _InterlockedUMax64(reinterpret_cast<unsigned long long volatile*>(p_Ptr), static_cast<unsigned long long>(value))

#endif
#endif

#define Spec_ATOMIC_UMAX64_RELAXED(p,v)  Spec_ATOMIC_UMAX64((p),(v), SPECTRA_MEMORY_ORDER_RELAXED)
#define Spec_ATOMIC_UMAX64_ACQUIRE(p,v)  Spec_ATOMIC_UMAX64((p),(v), SPECTRA_MEMORY_ORDER_ACQUIRE)
#define Spec_ATOMIC_UMAX64_RELEASE(p,v)  Spec_ATOMIC_UMAX64((p),(v), SPECTRA_MEMORY_ORDER_RELEASE)
#define Spec_ATOMIC_UMAX64_ACQ_REL(p,v)  Spec_ATOMIC_UMAX64((p),(v), SPECTRA_MEMORY_ORDER_ACQ_REL)
#define Spec_ATOMIC_UMAX64_SEQ_CST(p,v)  Spec_ATOMIC_UMAX64((p),(v), SPECTRA_MEMORY_ORDER_SEQ_CST)

	// Non-atomic bit test helpers.

#ifndef Spec_BITTEST32
#define Spec_BITTEST32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_BITTEST32(p_Ptr, bit) \
    _bittest(reinterpret_cast<long const*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_BITTEST64
#define Spec_BITTEST64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_BITTEST64(p_Ptr, bit) \
    _bittest64(reinterpret_cast<long long const*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_BITTEST_AND_SET32
#define Spec_BITTEST_AND_SET32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_BITTEST_AND_SET32(p_Ptr, bit) \
    _bittestandset(reinterpret_cast<long*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_BITTEST_AND_SET64
#define Spec_BITTEST_AND_SET64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_BITTEST_AND_SET64(p_Ptr, bit) \
    _bittestandset64(reinterpret_cast<long long*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_BITTEST_AND_RESET32
#define Spec_BITTEST_AND_RESET32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_BITTEST_AND_RESET32(p_Ptr, bit) \
    _bittestandreset(reinterpret_cast<long*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_BITTEST_AND_RESET64
#define Spec_BITTEST_AND_RESET64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_BITTEST_AND_RESET64(p_Ptr, bit) \
    _bittestandreset64(reinterpret_cast<long long*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_BITTEST_AND_COMPLEMENT32
#define Spec_BITTEST_AND_COMPLEMENT32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_BITTEST_AND_COMPLEMENT32(p_Ptr, bit) \
    _bittestandcomplement(reinterpret_cast<long*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_BITTEST_AND_COMPLEMENT64
#define Spec_BITTEST_AND_COMPLEMENT64
#if defined(SPECTRA_COMPILER_MSVC)

    // Atomic bit test and modify operations.

#define Spec_BITTEST_AND_COMPLEMENT64(p_Ptr, bit) \
    _bittestandcomplement64(reinterpret_cast<long long*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_INTERLOCKED_BITTEST_AND_SET
#define Spec_INTERLOCKED_BITTEST_AND_SET
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_INTERLOCKED_BITTEST_AND_SET(p_Ptr, bit) \
    _interlockedbittestandset(reinterpret_cast<long volatile*>(p_Ptr), (bit))

#endif
#endif

#ifndef Spec_INTERLOCKED_BITTEST_AND_RESET
#define Spec_INTERLOCKED_BITTEST_AND_RESET
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_INTERLOCKED_BITTEST_AND_RESET(p_Ptr, bit) \
    _interlockedbittestandreset(reinterpret_cast<long volatile*>(p_Ptr), (bit))

#endif
#endif

    // Bit scan helpers.
	// Return index of least / most significant set bit.

#ifndef Spec_BITSCAN_FORWARD32
#define Spec_BITSCAN_FORWARD32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_BITSCAN_FORWARD32(outIndex, value) \
    _BitScanForward((outIndex), static_cast<unsigned long>(value))

#endif
#endif

#ifndef Spec_BITSCAN_FORWARD64
#define Spec_BITSCAN_FORWARD64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_BITSCAN_FORWARD64(outIndex, value) \
    _BitScanForward64((outIndex), static_cast<unsigned long long>(value))

#endif
#endif

#ifndef Spec_BITSCAN_REVERSE32
#define Spec_BITSCAN_REVERSE32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_BITSCAN_REVERSE32(outIndex, value) \
    _BitScanReverse((outIndex), static_cast<unsigned long>(value))

#endif
#endif

#ifndef Spec_BITSCAN_REVERSE64
#define Spec_BITSCAN_REVERSE64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_BITSCAN_REVERSE64(outIndex, value) \
    _BitScanReverse64((outIndex), static_cast<unsigned long long>(value))

#endif
#endif

    // Bit population count.

#ifndef Spec_POPCOUNT32
#define Spec_POPCOUNT32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_POPCOUNT32(value) \
    __popcnt(static_cast<unsigned int>(value))

#endif
#endif

#ifndef Spec_POPCOUNT64
#define Spec_POPCOUNT64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_POPCOUNT64(value) \
    __popcnt64(static_cast<unsigned long long>(value))

#endif
#endif

    // Bit rotation helpers.

#ifndef Spec_ROTL32
#define Spec_ROTL32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ROTL32(value, shift) \
    _lrotl(static_cast<unsigned long>(value), (shift))

#endif
#endif

#ifndef Spec_ROTL64
#define Spec_ROTL64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ROTL64(value, shift) \
    _rotl64(static_cast<unsigned long long>(value), (shift))

#endif
#endif

#ifndef Spec_ROTR32
#define Spec_ROTR32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ROTR32(value, shift) \
    _lrotr(static_cast<unsigned long>(value), (shift))

#endif
#endif

#ifndef Spec_ROTR64
#define Spec_ROTR64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_ROTR64(value, shift) \
    _rotr64(static_cast<unsigned long long>(value), (shift))

#endif
#endif
    
	// Byte-order reversal helpers.

#ifndef Spec_BYTESWAP16
#define Spec_BYTESWAP16
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_BYTESWAP16(value) \
    _byteswap_ushort(static_cast<unsigned short>(value))

#endif
#endif

#ifndef Spec_BYTESWAP32
#define Spec_BYTESWAP32
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_BYTESWAP32(value) \
    _byteswap_ulong(static_cast<unsigned long>(value))

#endif
#endif

#ifndef Spec_BYTESWAP64
#define Spec_BYTESWAP64
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_BYTESWAP64(value) \
    _byteswap_uint64(static_cast<unsigned long long>(value))

#endif
#endif

    // CPU timestamp counter access.

#ifndef Spec_RDTSC
#define Spec_RDTSC
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_RDTSC() \
    __rdtsc()

#endif
#endif

    // Serialized timestamp counter access.

#ifndef Spec_RDTSCP
#define Spec_RDTSCP
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_RDTSCP(aux) \
    __rdtscp((aux))

#endif
#endif

    // Performance monitoring counter access.

#ifndef Spec_READPMC
#define Spec_READPMC
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_READPMC(counter) \
    __readpmc((counter))

#endif
#endif

    // CPU pause hint for spin-wait loops

#ifndef Spec_CPU_PAUSE
#define Spec_CPU_PAUSE
#if defined(SPECTRA_COMPILER_MSVC)

#define Spec_CPU_PAUSE() \
    _mm_pause()

#endif
#endif

    // Integer carry and borrow helpers.
	// Used for multi-precision arithmetic.

#ifndef Spec_ADDCARRY_U8
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_ADDCARRY_U8(c, a, b, out) \
            _addcarry_u8((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_ADDCARRY_U16
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_ADDCARRY_U16(c, a, b, out) \
            _addcarry_u16((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_ADDCARRY_U32
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_ADDCARRY_U32(c, a, b, out) \
            _addcarry_u32((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_ADDCARRY_U64
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_ADDCARRY_U64(c, a, b, out) \
            _addcarry_u64((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUBBORROW_U8
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SUBBORROW_U8(c, a, b, out) \
            _subborrow_u8((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUBBORROW_U16
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SUBBORROW_U16(c, a, b, out) \
            _subborrow_u16((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUBBORROW_U32
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SUBBORROW_U32(c, a, b, out) \
            _subborrow_u32((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUBBORROW_U64
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SUBBORROW_U64(c, a, b, out) \
            _subborrow_u64((c), (a), (b), (out))
#endif
#endif

    // Signed overflow-detecting arithmetic.

#ifndef Spec_ADD_OVERFLOW_I8
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_ADD_OVERFLOW_I8(c, a, b, out)  \
	_add_overflow_i8((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_ADD_OVERFLOW_I16
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_ADD_OVERFLOW_I16(c, a, b, out) \
	_add_overflow_i16((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_ADD_OVERFLOW_I32
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_ADD_OVERFLOW_I32(c, a, b, out)  \
	_add_overflow_i32((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_ADD_OVERFLOW_I64
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_ADD_OVERFLOW_I64(c, a, b, out) \
	_add_overflow_i64((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUB_OVERFLOW_I8
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SUB_OVERFLOW_I8(c, a, b, out)   _sub_overflow_i8((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUB_OVERFLOW_I16
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SUB_OVERFLOW_I16(c, a, b, out)  _sub_overflow_i16((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUB_OVERFLOW_I32
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SUB_OVERFLOW_I32(c, a, b, out)  _sub_overflow_i32((c), (a), (b), (out))
#endif
#endif

#ifndef Spec_SUB_OVERFLOW_I64
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SUB_OVERFLOW_I64(c, a, b, out)  _sub_overflow_i64((c), (a), (b), (out))
#endif
#endif

    // Full-width multiplication helpers.

#ifndef Spec_MUL_OVERFLOW_I16
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_MUL_OVERFLOW_I16(a, b, out) \
            _mul_overflow_i16((a), (b), (out))
#endif
#endif

#ifndef Spec_MUL_OVERFLOW_I32
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_MUL_OVERFLOW_I32(a, b, out) \
            _mul_overflow_i32((a), (b), (out))
#endif
#endif

#ifndef Spec_MUL_OVERFLOW_I64
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_MUL_OVERFLOW_I64(a, b, out) \
            _mul_overflow_i64((a), (b), (out))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_I8
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_MUL_FULL_OVERFLOW_I8(a, b, lo, hi) \
            _mul_full_overflow_i8((a), (b), (lo), (hi))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_I16
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_MUL_FULL_OVERFLOW_I16(a, b, lo, hi) \
            _mul_full_overflow_i16((a), (b), (lo), (hi))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_I32
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_MUL_FULL_OVERFLOW_I32(a, b, lo, hi) \
            _mul_full_overflow_i32((a), (b), (lo), (hi))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_I64
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_MUL_FULL_OVERFLOW_I64(a, b, lo, hi) \
            _mul_full_overflow_i64((a), (b), (lo), (hi))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_U8
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_MUL_FULL_OVERFLOW_U8(a, b, lo, hi) \
            _mul_full_overflow_u8((a), (b), (lo), (hi))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_U16
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_MUL_FULL_OVERFLOW_U16(a, b, lo, hi) \
            _mul_full_overflow_u16((a), (b), (lo), (hi))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_U32
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_MUL_FULL_OVERFLOW_U32(a, b, lo, hi) \
            _mul_full_overflow_u32((a), (b), (lo), (hi))
#endif
#endif

#ifndef Spec_MUL_FULL_OVERFLOW_U64
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_MUL_FULL_OVERFLOW_U64(a, b, lo, hi) \
            _mul_full_overflow_u64((a), (b), (lo), (hi))
#endif
#endif

    // Saturating arithmetic helpers.
	// Clamp results on overflow instead of wrapping.

#ifndef Spec_SAT_ADD_I8
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_ADD_I8(a, b) \
            _sat_add_i8((a), (b))
#endif
#endif

#ifndef Spec_SAT_ADD_I16
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_ADD_I16(a, b) \
            _sat_add_i16((a), (b))
#endif
#endif

#ifndef Spec_SAT_ADD_I32
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_ADD_I32(a, b) \
            _sat_add_i32((a), (b))
#endif
#endif

#ifndef Spec_SAT_ADD_I64
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_ADD_I64(a, b) \
            _sat_add_i64((a), (b))
#endif
#endif

#ifndef Spec_SAT_ADD_U8
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_ADD_U8(a, b) \
            _sat_add_u8((a), (b))
#endif
#endif

#ifndef Spec_SAT_ADD_U16
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_ADD_U16(a, b) \
            _sat_add_u16((a), (b))
#endif
#endif

#ifndef Spec_SAT_ADD_U32
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_ADD_U32(a, b) \
            _sat_add_u32((a), (b))
#endif
#endif

#ifndef Spec_SAT_ADD_U64
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_ADD_U64(a, b) \
            _sat_add_u64((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_I8
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_SUB_I8(a, b) \
            _sat_sub_i8((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_I16
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_SUB_I16(a, b) \
            _sat_sub_i16((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_I32
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_SUB_I32(a, b) \
            _sat_sub_i32((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_I64
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_SUB_I64(a, b) \
            _sat_sub_i64((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_U8
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_SUB_U8(a, b) \
            _sat_sub_u8((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_U16
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_SUB_U16(a, b) \
            _sat_sub_u16((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_U32
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_SUB_U32(a, b) \
            _sat_sub_u32((a), (b))
#endif
#endif

#ifndef Spec_SAT_SUB_U64
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_SAT_SUB_U64(a, b) \
            _sat_sub_u64((a), (b))
#endif
#endif

	// Hardware-accelerated CRC32 helpers (SSE4.2).

#ifndef Spec_CRC32_U8
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_CRC32_U8(crc, data) \
            _mm_crc32_u8((crc), (data))
#endif
#endif

#ifndef Spec_CRC32_U16
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_CRC32_U16(crc, data) \
            _mm_crc32_u16((crc), (data))
#endif
#endif

#ifndef Spec_CRC32_U32
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_CRC32_U32(crc, data) \
            _mm_crc32_u32((crc), (data))
#endif
#endif

#ifndef Spec_CRC32_U64
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_CRC32_U64(crc, data) \
            _mm_crc32_u64((crc), (data))
#endif
#endif

    // Low-level control-flow inspection helpers.
	// ABI- and optimizer-sensitive; use with care.

#ifndef Spec_ADDRESS_OF_RETURN_ADDRESS
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_ADDRESS_OF_RETURN_ADDRESS() \
            _AddressOfReturnAddress()
#endif
#endif

#ifndef Spec_RETURN_ADDRESS
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_RETURN_ADDRESS() \
            _ReturnAddress()
#endif
#endif

#ifndef Spec_ADDRESS_OF_NEXT_INSTRUCTION
#if defined(SPECTRA_COMPILER_MSVC)
#define Spec_ADDRESS_OF_NEXT_INSTRUCTION() \
            _AddressOfNextInstruction()
#endif
#endif
}
