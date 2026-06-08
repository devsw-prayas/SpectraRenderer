#pragma once

namespace Spectra::RHI {
#if defined(_MSC_VER)
#define SPEC_RHI_COMPILER_MSVC 1
#else
#define SPEC_RHI_COMPILER_MSVC 0
#endif

#if defined(__clang__)
#define SPEC_RHI_COMPILER_CLANG 1
#else
#define SPEC_RHI_COMPILER_CLANG 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define SPEC_RHI_COMPILER_GCC 1
#else
#define SPEC_RHI_COMPILER_GCC 0
#endif
}

#if SPEC_RHI_COMPILER_MSVC
#define SPEC_RHI_FORCEINLINE __forceinline
#define SPEC_RHI_NOINLINE    __declspec(noinline)
#elif SPEC_RHI_COMPILER_CLANG || SPEC_RHI_COMPILER_GCC
#define SPEC_RHI_FORCEINLINE inline __attribute__((always_inline))
#define SPEC_RHI_NOINLINE    __attribute__((noinline))
#else
#define SPEC_RHI_FORCEINLINE inline
#define SPEC_RHI_NOINLINE
#endif

#define SPEC_RHI_INLINE inline

#if SPEC_RHI_COMPILER_MSVC
#define SPEC_RHI_COMPILER_BARRIER() _ReadWriteBarrier()
#elif SPEC_RHI_COMPILER_CLANG || SPEC_RHI_COMPILER_GCC
#define SPEC_RHI_COMPILER_BARRIER() asm volatile("" ::: "memory")
#else
#define SPEC_RHI_COMPILER_BARRIER()
#endif

#if SPEC_RHI_COMPILER_MSVC
#define SPEC_RHI_OPTIMIZE_OFF __pragma(optimize("", off))
#define SPEC_RHI_OPTIMIZE_ON  __pragma(optimize("", on))
#elif SPEC_RHI_COMPILER_CLANG || SPEC_RHI_COMPILER_GCC
#define SPEC_RHI_OPTIMIZE_OFF _Pragma("clang optimize off")
#define SPEC_RHI_OPTIMIZE_ON  _Pragma("clang optimize on")
#else
#define SPEC_RHI_OPTIMIZE_OFF
#define SPEC_RHI_OPTIMIZE_ON
#endif

#if SPEC_RHI_COMPILER_CLANG || SPEC_RHI_COMPILER_GCC
#define SPEC_RHI_LIKELY(x)   __builtin_expect(!!(x), 1)
#define SPEC_RHI_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define SPEC_RHI_LIKELY(x)   (x)
#define SPEC_RHI_UNLIKELY(x) (x)
#endif

#if SPEC_RHI_COMPILER_MSVC
#define SPEC_RHI_DEBUG_BREAK() __debugbreak()
#define SPEC_RHI_TRAP()        __debugbreak()
#elif SPEC_RHI_COMPILER_CLANG || SPEC_RHI_COMPILER_GCC
#define SPEC_RHI_DEBUG_BREAK() __builtin_trap()
#define SPEC_RHI_TRAP()        __builtin_trap()
#else
#include <cstdlib>
#define SPEC_RHI_DEBUG_BREAK() std::abort()
#define SPEC_RHI_TRAP()        std::abort()
#endif

#if SPEC_RHI_COMPILER_MSVC
#define SPEC_RHI_UNREACHABLE() __assume(0)
#elif SPEC_RHI_COMPILER_CLANG || SPEC_RHI_COMPILER_GCC
#define SPEC_RHI_UNREACHABLE() __builtin_unreachable()
#else
#define SPEC_RHI_UNREACHABLE() SPEC_RHI_TRAP()
#endif

#if SPEC_RHI_COMPILER_MSVC
#define SPEC_RHI_PRAGMA(x) __pragma(x)
#elif SPEC_RHI_COMPILER_CLANG || SPEC_RHI_COMPILER_GCC
#define SPEC_RHI_PRAGMA(x) _Pragma(#x)
#else
#define SPEC_RHI_PRAGMA(x)
#endif

#define SPEC_RHI_DIAGNOSTIC_PUSH SPEC_RHI_PRAGMA(diagnostic push)
#define SPEC_RHI_DIAGNOSTIC_POP  SPEC_RHI_PRAGMA(diagnostic pop)

#if SPEC_RHI_COMPILER_MSVC
#define SPEC_RHI_DISABLE_WARNING(w) SPEC_RHI_PRAGMA(warning(disable : w))
#elif SPEC_RHI_COMPILER_CLANG || SPEC_RHI_COMPILER_GCC
#define SPEC_RHI_DISABLE_WARNING(w) SPEC_RHI_PRAGMA(clang diagnostic ignored w)
#else
#define SPEC_RHI_DISABLE_WARNING(w)
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(fallthrough)
#define SPEC_RHI_FALLTHROUGH [[fallthrough]]
#else
#define SPEC_RHI_FALLTHROUGH
#endif
#else
#define SPEC_RHI_FALLTHROUGH
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#define SPEC_RHI_NODISCARD [[nodiscard]]
#if __cplusplus >= 202002L
#define SPEC_RHI_NODISCARD_MSG(msg) [[nodiscard(msg)]]
#else
#define SPEC_RHI_NODISCARD_MSG(msg) [[nodiscard]]
#endif
#else
#define SPEC_RHI_NODISCARD
#define SPEC_RHI_NODISCARD_MSG(msg)
#endif
#else
#define SPEC_RHI_NODISCARD
#define SPEC_RHI_NODISCARD_MSG(msg)
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define SPEC_RHI_MAYBE_UNUSED [[maybe_unused]]
#else
#define SPEC_RHI_MAYBE_UNUSED
#endif
#else
#define SPEC_RHI_MAYBE_UNUSED
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(deprecated)
#define SPEC_RHI_DEPRECATED [[deprecated]]
#define SPEC_RHI_DEPRECATED_MSG(msg) [[deprecated(msg)]]
#else
#define SPEC_RHI_DEPRECATED
#define SPEC_RHI_DEPRECATED_MSG(msg)
#endif
#else
#define SPEC_RHI_DEPRECATED
#define SPEC_RHI_DEPRECATED_MSG(msg)
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(noreturn)
#define SPEC_RHI_NORETURN [[noreturn]]
#else
#define SPEC_RHI_NORETURN
#endif
#else
#define SPEC_RHI_NORETURN
#endif

#if SPEC_RHI_COMPILER_MSVC
#define SPEC_RHI_RESTRICT __restrict
#elif SPEC_RHI_COMPILER_CLANG || SPEC_RHI_COMPILER_GCC
#define SPEC_RHI_RESTRICT __restrict__
#else
#define SPEC_RHI_RESTRICT
#endif

#define SPEC_RHI_ALIGNAS(n) alignas(n)

#if defined(_WIN32)
#define SPEC_RHI_API_PTR __stdcall
#else
#define SPEC_RHI_API_PTR
#endif

#if SPEC_RHI_COMPILER_CLANG || SPEC_RHI_COMPILER_GCC
#define SPEC_RHI_ASSUME_ALIGNED(ptr, n) __builtin_assume_aligned((ptr), (n))
#else
#define SPEC_RHI_ASSUME_ALIGNED(ptr, n) (ptr)
#endif

#if SPEC_RHI_COMPILER_CLANG || SPEC_RHI_COMPILER_GCC
#define SPEC_RHI_HOT  __attribute__((hot))
#define SPEC_RHI_COLD __attribute__((cold))
#else
#define SPEC_RHI_HOT
#define SPEC_RHI_COLD
#endif
