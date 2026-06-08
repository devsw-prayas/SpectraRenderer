#pragma once
#include "SpectraRHI.h"
#include "SpecRHICompiler.h"

#if defined(_DEBUG) || defined(DEBUG)
#define SPEC_RHI_BUILD_DEBUG 1
#define SPEC_RHI_BUILD_RELEASE 0
#else
#define SPEC_RHI_BUILD_DEBUG 0
#define SPEC_RHI_BUILD_RELEASE 1
#endif

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
