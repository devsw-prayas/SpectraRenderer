#pragma once

namespace Spectra::FileSystem {
#if defined(_MSC_VER)
#define SPEC_FS_COMPILER_MSVC 1
#else
#define SPEC_FS_COMPILER_MSVC 0
#endif

#if defined(__clang__)
#define SPEC_FS_COMPILER_CLANG 1
#else
#define SPEC_FS_COMPILER_CLANG 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define SPEC_FS_COMPILER_GCC 1
#else
#define SPEC_FS_COMPILER_GCC 0
#endif
}

#if SPEC_FS_COMPILER_MSVC
#define SPEC_FS_FORCEINLINE __forceinline
#define SPEC_FS_NOINLINE    __declspec(noinline)
#elif SPEC_FS_COMPILER_CLANG || SPEC_FS_COMPILER_GCC
#define SPEC_FS_FORCEINLINE inline __attribute__((always_inline))
#define SPEC_FS_NOINLINE    __attribute__((noinline))
#else
#define SPEC_FS_FORCEINLINE inline
#define SPEC_FS_NOINLINE
#endif

#define SPEC_FS_INLINE inline

#if SPEC_FS_COMPILER_MSVC
#define SPEC_FS_COMPILER_BARRIER() _ReadWriteBarrier()
#elif SPEC_FS_COMPILER_CLANG || SPEC_FS_COMPILER_GCC
#define SPEC_FS_COMPILER_BARRIER() asm volatile("" ::: "memory")
#else
#define SPEC_FS_COMPILER_BARRIER()
#endif

#if SPEC_FS_COMPILER_MSVC
#define SPEC_FS_OPTIMIZE_OFF __pragma(optimize("", off))
#define SPEC_FS_OPTIMIZE_ON  __pragma(optimize("", on))
#elif SPEC_FS_COMPILER_CLANG || SPEC_FS_COMPILER_GCC
#define SPEC_FS_OPTIMIZE_OFF _Pragma("clang optimize off")
#define SPEC_FS_OPTIMIZE_ON  _Pragma("clang optimize on")
#else
#define SPEC_FS_OPTIMIZE_OFF
#define SPEC_FS_OPTIMIZE_ON
#endif

#if SPEC_FS_COMPILER_CLANG || SPEC_FS_COMPILER_GCC
#define SPEC_FS_LIKELY(x)   __builtin_expect(!!(x), 1)
#define SPEC_FS_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define SPEC_FS_LIKELY(x)   (x)
#define SPEC_FS_UNLIKELY(x) (x)
#endif

#if SPEC_FS_COMPILER_MSVC
#define SPEC_FS_DEBUG_BREAK() __debugbreak()
#define SPEC_FS_TRAP()        __debugbreak()
#elif SPEC_FS_COMPILER_CLANG || SPEC_FS_COMPILER_GCC
#define SPEC_FS_DEBUG_BREAK() __builtin_trap()
#define SPEC_FS_TRAP()        __builtin_trap()
#else
#include <cstdlib>
#define SPEC_FS_DEBUG_BREAK() std::abort()
#define SPEC_FS_TRAP()        std::abort()
#endif

#if SPEC_FS_COMPILER_MSVC
#define SPEC_FS_UNREACHABLE() __assume(0)
#elif SPEC_FS_COMPILER_CLANG || SPEC_FS_COMPILER_GCC
#define SPEC_FS_UNREACHABLE() __builtin_unreachable()
#else
#define SPEC_FS_UNREACHABLE() SPEC_FS_TRAP()
#endif

#if SPEC_FS_COMPILER_MSVC
#define SPEC_FS_PRAGMA(x) __pragma(x)
#elif SPEC_FS_COMPILER_CLANG || SPEC_FS_COMPILER_GCC
#define SPEC_FS_PRAGMA(x) _Pragma(#x)
#else
#define SPEC_FS_PRAGMA(x)
#endif

#define SPEC_FS_DIAGNOSTIC_PUSH SPEC_FS_PRAGMA(diagnostic push)
#define SPEC_FS_DIAGNOSTIC_POP  SPEC_FS_PRAGMA(diagnostic pop)

#if SPEC_FS_COMPILER_MSVC
#define SPEC_FS_DISABLE_WARNING(w) SPEC_FS_PRAGMA(warning(disable : w))
#elif SPEC_FS_COMPILER_CLANG || SPEC_FS_COMPILER_GCC
#define SPEC_FS_DISABLE_WARNING(w) SPEC_FS_PRAGMA(clang diagnostic ignored w)
#else
#define SPEC_FS_DISABLE_WARNING(w)
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(fallthrough)
#define SPEC_FS_FALLTHROUGH [[fallthrough]]
#else
#define SPEC_FS_FALLTHROUGH
#endif
#else
#define SPEC_FS_FALLTHROUGH
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#define SPEC_FS_NODISCARD [[nodiscard]]
#if __cplusplus >= 202002L
#define SPEC_FS_NODISCARD_MSG(msg) [[nodiscard(msg)]]
#else
#define SPEC_FS_NODISCARD_MSG(msg) [[nodiscard]]
#endif
#else
#define SPEC_FS_NODISCARD
#define SPEC_FS_NODISCARD_MSG(msg)
#endif
#else
#define SPEC_FS_NODISCARD
#define SPEC_FS_NODISCARD_MSG(msg)
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define SPEC_FS_MAYBE_UNUSED [[maybe_unused]]
#else
#define SPEC_FS_MAYBE_UNUSED
#endif
#else
#define SPEC_FS_MAYBE_UNUSED
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(deprecated)
#define SPEC_FS_DEPRECATED [[deprecated]]
#define SPEC_FS_DEPRECATED_MSG(msg) [[deprecated(msg)]]
#else
#define SPEC_FS_DEPRECATED
#define SPEC_FS_DEPRECATED_MSG(msg)
#endif
#else
#define SPEC_FS_DEPRECATED
#define SPEC_FS_DEPRECATED_MSG(msg)
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(noreturn)
#define SPEC_FS_NORETURN [[noreturn]]
#else
#define SPEC_FS_NORETURN
#endif
#else
#define SPEC_FS_NORETURN
#endif

#if SPEC_FS_COMPILER_MSVC
#define SPEC_FS_RESTRICT __restrict
#elif SPEC_FS_COMPILER_CLANG || SPEC_FS_COMPILER_GCC
#define SPEC_FS_RESTRICT __restrict__
#else
#define SPEC_FS_RESTRICT
#endif

#define SPEC_FS_ALIGNAS(n) alignas(n)

#if SPEC_FS_COMPILER_CLANG || SPEC_FS_COMPILER_GCC
#define SPEC_FS_ASSUME_ALIGNED(ptr, n) __builtin_assume_aligned((ptr), (n))
#else
#define SPEC_FS_ASSUME_ALIGNED(ptr, n) (ptr)
#endif

#if SPEC_FS_COMPILER_CLANG || SPEC_FS_COMPILER_GCC
#define SPEC_FS_HOT  __attribute__((hot))
#define SPEC_FS_COLD __attribute__((cold))
#else
#define SPEC_FS_HOT
#define SPEC_FS_COLD
#endif
