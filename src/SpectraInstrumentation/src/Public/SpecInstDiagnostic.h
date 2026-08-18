#pragma once
#include "SpectraInstrumentation.h"
#include "SpecInstCompiler.h"

#if defined(_DEBUG) || defined(DEBUG)
#define SPEC_INST_BUILD_DEBUG 1
#define SPEC_INST_BUILD_RELEASE 0
#else
#define SPEC_INST_BUILD_DEBUG 0
#define SPEC_INST_BUILD_RELEASE 1
#endif

#if SPEC_INST_BUILD_DEBUG

#define SPEC_INST_ASSERT(expr)                                     \
        do {                                                   \
            if (!(expr)) {                                    \
                SPEC_INST_DEBUG_BREAK();                              \
                SPEC_INST_TRAP();                                     \
            }                                                  \
        } while (0)

#else

#define SPEC_INST_ASSERT(expr) do { (void)sizeof(expr); } while (0)

#endif

#if SPEC_INST_BUILD_DEBUG
#define SPEC_INST_ASSUME(expr) SPEC_INST_ASSERT(expr)
#else
#if SPEC_INST_COMPILER_MSVC
#define SPEC_INST_ASSUME(expr) __assume(expr)
#elif SPEC_INST_COMPILER_CLANG || SPEC_INST_COMPILER_GCC
#define SPEC_INST_ASSUME(expr) do { if (!(expr)) __builtin_unreachable(); } while (0)
#else
#define SPEC_INST_ASSUME(expr) do { } while (0)
#endif
#endif

#if SPEC_INST_BUILD_DEBUG
#define SPEC_INST_DEBUG_ASSERT(expr) SPEC_INST_ASSERT(expr)
#define SPEC_INST_DEBUG_ASSUME(expr) SPEC_INST_ASSUME(expr)
#else
#define SPEC_INST_DEBUG_ASSERT(expr) do {} while (0)
#define SPEC_INST_DEBUG_ASSUME(expr) do {} while (0)
#endif

#define SPEC_INST_STATIC_ASSERT(expr, msg) static_assert(expr, msg)

#define SPEC_INST_UNUSED(x) (void)(x)
