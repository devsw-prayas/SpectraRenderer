#pragma once
#include "SpectraRHI.h"
#include "SpecRHICompiler.h"

// SPEC_RHI_DEBUG_CHECKS (0/1) comes from SPEC_RHI_ENABLE_DEBUG_CHECKS=ON/OFF; unset (AUTO) follows _DEBUG.
#if defined(SPEC_RHI_DEBUG_CHECKS)
#define SPEC_RHI_BUILD_DEBUG SPEC_RHI_DEBUG_CHECKS
#elif defined(_DEBUG)
#define SPEC_RHI_BUILD_DEBUG 1
#else
#define SPEC_RHI_BUILD_DEBUG 0
#endif
#define SPEC_RHI_BUILD_RELEASE (!SPEC_RHI_BUILD_DEBUG)

#if SPEC_RHI_BUILD_DEBUG

#define SPEC_RHI_ASSERT(expr)                                     \
        do {                                                   \
            if (!(expr)) {                                    \
                SPEC_RHI_DEBUG_BREAK();                              \
                SPEC_RHI_TRAP();                                     \
            }                                                  \
        } while (0)

#else

#define SPEC_RHI_ASSERT(expr) do { (void)sizeof(expr); } while (0)

#endif

#if SPEC_RHI_BUILD_DEBUG
#define SPEC_RHI_ASSUME(expr) SPEC_RHI_ASSERT(expr)
#else
#if SPEC_RHI_COMPILER_MSVC
#define SPEC_RHI_ASSUME(expr) __assume(expr)
#elif SPEC_RHI_COMPILER_CLANG || SPEC_RHI_COMPILER_GCC
#define SPEC_RHI_ASSUME(expr) do { if (!(expr)) __builtin_unreachable(); } while (0)
#else
#define SPEC_RHI_ASSUME(expr) do { } while (0)
#endif
#endif

#if SPEC_RHI_BUILD_DEBUG
#define SPEC_RHI_DEBUG_ASSERT(expr) SPEC_RHI_ASSERT(expr)
#define SPEC_RHI_DEBUG_ASSUME(expr) SPEC_RHI_ASSUME(expr)
#else
#define SPEC_RHI_DEBUG_ASSERT(expr) do {} while (0)
#define SPEC_RHI_DEBUG_ASSUME(expr) do {} while (0)
#endif

#define SPEC_RHI_STATIC_ASSERT(expr, msg) static_assert(expr, msg)

#define SPEC_RHI_UNUSED(x) (void)(x)
