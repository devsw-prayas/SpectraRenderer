#pragma once

// ================================================================
// Compiler intent policy
//
// Files that need raw compiler intrinsics MUST opt in explicitly
// by defining SPEC_ALLOW_COMPILER before including this header.
// ================================================================

// Just don't mess with this header, exposes low level shit,
// Don't want system gettin' fucked when a supposedly impossible linux port occurs

#if defined(_MSC_VER)
#define SPECTRA_COMPILER_MSVC 1
#else
#define SPECTRA_COMPILER_MSVC 0
#endif

#if defined(__clang__)
#define SPECTRA_COMPILER_CLANG 1
#else
#define SPECTRA_COMPILER_CLANG 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define SPECTRA_COMPILER_GCC 1
#else
#define SPECTRA_COMPILER_GCC 0
#endif

#if SPECTRA_COMPILER_MSVC
#define SPECTRA_FORCEINLINE __forceinline
#define SPECTRA_NOINLINE    __declspec(noinline)
#elif SPECTRA_COMPILER_CLANG || SPECTRA_COMPILER_GCC
#define SPECTRA_FORCEINLINE inline __attribute__((always_inline))
#define SPECTRA_NOINLINE    __attribute__((noinline))
#else
#define SPECTRA_FORCEINLINE inline
#define SPECTRA_NOINLINE
#endif

#define SPECTRA_INLINE inline

#if SPECTRA_COMPILER_CLANG || SPECTRA_COMPILER_GCC
#define SPECTRA_LIKELY(x)   __builtin_expect(!!(x), 1)
#define SPECTRA_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define SPECTRA_LIKELY(x)   (x)
#define SPECTRA_UNLIKELY(x) (x)
#endif

#if SPECTRA_COMPILER_MSVC
#define SPECTRA_DEBUG_BREAK() __debugbreak()
#define SPECTRA_TRAP()        __debugbreak()
#elif SPECTRA_COMPILER_CLANG || SPECTRA_COMPILER_GCC
#define SPECTRA_DEBUG_BREAK() __builtin_trap()
#define SPECTRA_TRAP()        __builtin_trap()
#else
#include <cstdlib>
#define SPECTRA_DEBUG_BREAK() std::abort()
#define SPECTRA_TRAP()        std::abort()
#endif

#if SPECTRA_COMPILER_MSVC
#define SPECTRA_UNREACHABLE() __assume(0)
#elif SPECTRA_COMPILER_CLANG || SPECTRA_COMPILER_GCC
#define SPECTRA_UNREACHABLE() __builtin_unreachable()
#else
#define SPECTRA_UNREACHABLE() SPECTRA_TRAP()
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#define SPECTRA_NODISCARD [[nodiscard]]
#if __cplusplus >= 202002L
#define SPECTRA_NODISCARD_MSG(msg) [[nodiscard(msg)]]
#else
#define SPECTRA_NODISCARD_MSG(msg) [[nodiscard]]
#endif
#else
#define SPECTRA_NODISCARD
#define SPECTRA_NODISCARD_MSG(msg)
#endif
#else
#define SPECTRA_NODISCARD
#define SPECTRA_NODISCARD_MSG(msg)
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define SPECTRA_MAYBE_UNUSED [[maybe_unused]]
#else
#define SPECTRA_MAYBE_UNUSED
#endif
#else
#define SPECTRA_MAYBE_UNUSED
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(fallthrough)
#define SPECTRA_FALLTHROUGH [[fallthrough]]
#else
#define SPECTRA_FALLTHROUGH
#endif
#else
#define SPECTRA_FALLTHROUGH
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(noreturn)
#define SPECTRA_NORETURN [[noreturn]]
#else
#define SPECTRA_NORETURN
#endif
#else
#define SPECTRA_NORETURN
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(deprecated)
#define SPECTRA_DEPRECATED [[deprecated]]
#define SPECTRA_DEPRECATED_MSG(msg) [[deprecated(msg)]]
#else
#define SPECTRA_DEPRECATED
#define SPECTRA_DEPRECATED_MSG(msg)
#endif
#else
#define SPECTRA_DEPRECATED
#define SPECTRA_DEPRECATED_MSG(msg)
#endif

#if SPECTRA_COMPILER_MSVC
#define SPECTRA_RESTRICT __restrict
#elif SPECTRA_COMPILER_CLANG || SPECTRA_COMPILER_GCC
#define SPECTRA_RESTRICT __restrict__
#else
#define SPECTRA_RESTRICT
#endif
