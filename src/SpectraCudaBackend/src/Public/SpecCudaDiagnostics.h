#pragma once
#include "SpectraCudaBackend.h"
#include "SpecCudaCompiler.h"

// SPEC_CUDA_BK_DEBUG_CHECKS (0/1) comes from SPEC_CUDA_BK_ENABLE_DEBUG_CHECKS=ON/OFF; unset (AUTO) follows _DEBUG.
#if defined(SPEC_CUDA_BK_DEBUG_CHECKS)
#define SPEC_CUDA_BK_BUILD_DEBUG SPEC_CUDA_BK_DEBUG_CHECKS
#elif defined(_DEBUG)
#define SPEC_CUDA_BK_BUILD_DEBUG 1
#else
#define SPEC_CUDA_BK_BUILD_DEBUG 0
#endif
#define SPEC_CUDA_BK_BUILD_RELEASE (!SPEC_CUDA_BK_BUILD_DEBUG)

#if SPEC_CUDA_BK_BUILD_DEBUG

#define SPEC_CUDA_BK_ASSERT(expr)                                     \
        do {                                                   \
            if (!(expr)) {                                    \
                SPEC_CUDA_BK_DEBUG_BREAK();                              \
                SPEC_CUDA_BK_TRAP();                                     \
            }                                                  \
        } while (0)

#else

#define SPEC_CUDA_BK_ASSERT(expr) do { (void)sizeof(expr); } while (0)

#endif

#if SPEC_CUDA_BK_BUILD_DEBUG
#define SPEC_CUDA_BK_ASSUME(expr) SPEC_CUDA_BK_ASSERT(expr)
#else
#if SPEC_CUDA_BK_COMPILER_MSVC
#define SPEC_CUDA_BK_ASSUME(expr) __assume(expr)
#elif SPEC_CUDA_BK_COMPILER_CLANG || SPEC_CUDA_BK_COMPILER_GCC
#define SPEC_CUDA_BK_ASSUME(expr) do { if (!(expr)) __builtin_unreachable(); } while (0)
#else
#define SPEC_CUDA_BK_ASSUME(expr) do { } while (0)
#endif
#endif

#if SPEC_CUDA_BK_BUILD_DEBUG
#define SPEC_CUDA_BK_DEBUG_ASSERT(expr) SPEC_CUDA_BK_ASSERT(expr)
#define SPEC_CUDA_BK_DEBUG_ASSUME(expr) SPEC_CUDA_BK_ASSUME(expr)
#else
#define SPEC_CUDA_BK_DEBUG_ASSERT(expr) do {} while (0)
#define SPEC_CUDA_BK_DEBUG_ASSUME(expr) do {} while (0)
#endif

#define SPEC_CUDA_BK_STATIC_ASSERT(expr, msg) static_assert(expr, msg)

#define SPEC_CUDA_BK_UNUSED(x) (void)(x)