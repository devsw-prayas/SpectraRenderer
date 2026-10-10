#pragma once
#include "SpectraCore.h"
#include "SpecCoreCompiler.h"

// SPEC_CORE_DEBUG_CHECKS (0/1) comes from SPEC_CORE_ENABLE_DEBUG_CHECKS=ON/OFF; unset (AUTO) follows _DEBUG.
#if defined(SPEC_CORE_DEBUG_CHECKS)
#define SPEC_CORE_BUILD_DEBUG SPEC_CORE_DEBUG_CHECKS
#define SPEC_CORE_BUILD_RELEASE (!SPEC_CORE_DEBUG_CHECKS)
#elif defined(_DEBUG)
#define SPEC_CORE_BUILD_DEBUG 1
#define SPEC_CORE_BUILD_RELEASE 0
#else
#define SPEC_CORE_BUILD_DEBUG 0
#define SPEC_CORE_BUILD_RELEASE 1
#endif

#if SPEC_CORE_BUILD_DEBUG

#define SPEC_CORE_ASSERT(expr)                                     \
        do {                                                   \
            if (!(expr)) {                                    \
                SPEC_CORE_DEBUG_BREAK();                              \
                SPEC_CORE_TRAP();                                     \
            }                                                  \
        } while (0)

#else

#define SPEC_CORE_ASSERT(expr) do { (void)sizeof(expr); } while (0)

#endif

#if SPEC_CORE_BUILD_DEBUG
#define SPEC_CORE_ASSUME(expr) SPEC_CORE_ASSERT(expr)
#else
#if SPEC_CORE_COMPILER_MSVC
#define SPEC_CORE_ASSUME(expr) __assume(expr)
#elif SPEC_CORE_COMPILER_CLANG || SPEC_CORE_COMPILER_GCC
#define SPEC_CORE_ASSUME(expr) do { if (!(expr)) __builtin_unreachable(); } while (0)
#else
#define SPEC_CORE_ASSUME(expr) do { } while (0)
#endif
#endif

#if SPEC_CORE_BUILD_DEBUG
#define SPEC_CORE_DEBUG_ASSERT(expr) SPEC_CORE_ASSERT(expr)
#define SPEC_CORE_DEBUG_ASSUME(expr) SPEC_CORE_ASSUME(expr)
#else
#define SPEC_CORE_DEBUG_ASSERT(expr) do {} while (0)
#define SPEC_CORE_DEBUG_ASSUME(expr) do {} while (0)
#endif

#define SPEC_CORE_STATIC_ASSERT(expr, msg) static_assert(expr, msg)

#define SPEC_CORE_UNUSED(x) (void)(x)
