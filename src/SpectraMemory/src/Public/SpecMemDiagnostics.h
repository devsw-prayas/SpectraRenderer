#pragma once
#include "SpectraMemory.h"
#include "SpecMemCompiler.h"

#if defined(_DEBUG) || defined(DEBUG)
#define SPEC_MEM_BUILD_DEBUG 1
#define SPEC_MEM_BUILD_RELEASE 0
#else
#define SPEC_MEM_BUILD_DEBUG 0
#define SPEC_MEM_BUILD_RELEASE 1
#endif

#if SPEC_MEM_BUILD_DEBUG

#define SPEC_MEM_ASSERT(expr)                                     \
        do {                                                   \
            if (!(expr)) {                                    \
                SPEC_MEM_DEBUG_BREAK();                              \
                SPEC_MEM_TRAP();                                     \
            }                                                  \
        } while (0)

#else

#define SPEC_MEM_ASSERT(expr) do { (void)sizeof(expr); } while (0)

#endif

#if SPEC_MEM_BUILD_DEBUG
#define SPEC_MEM_ASSUME(expr) SPEC_MEM_ASSERT(expr)
#else
#if SPEC_MEM_COMPILER_MSVC
#define SPEC_MEM_ASSUME(expr) __assume(expr)
#elif SPEC_MEM_COMPILER_CLANG || SPEC_MEM_COMPILER_GCC
#define SPEC_MEM_ASSUME(expr) do { if (!(expr)) __builtin_unreachable(); } while (0)
#else
#define SPEC_MEM_ASSUME(expr) do { } while (0)
#endif
#endif

#if SPEC_MEM_BUILD_DEBUG
#define SPEC_MEM_DEBUG_ASSERT(expr) SPEC_MEM_ASSERT(expr)
#define SPEC_MEM_DEBUG_ASSUME(expr) SPEC_MEM_ASSUME(expr)
#else
#define SPEC_MEM_DEBUG_ASSERT(expr) do {} while (0)
#define SPEC_MEM_DEBUG_ASSUME(expr) do {} while (0)
#endif

#define SPEC_MEM_STATIC_ASSERT(expr, msg) static_assert(expr, msg)

#define SPEC_MEM_UNUSED(x) (void)(x)
