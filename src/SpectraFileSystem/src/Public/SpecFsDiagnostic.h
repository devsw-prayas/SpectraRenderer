#pragma once
#include "SpectraFileSystem.h"
#include "SpecFsCompiler.h"

#if defined(_DEBUG) || defined(DEBUG)
#define SPEC_FS_BUILD_DEBUG 1
#define SPEC_FS_BUILD_RELEASE 0
#else
#define SPEC_FS_BUILD_DEBUG 0
#define SPEC_FS_BUILD_RELEASE 1
#endif

#if SPEC_FS_BUILD_DEBUG

#define SPEC_FS_ASSERT(expr)                                     \
        do {                                                   \
            if (!(expr)) {                                    \
                SPEC_FS_DEBUG_BREAK();                              \
                SPEC_FS_TRAP();                                     \
            }                                                  \
        } while (0)

#else

#define SPEC_FS_ASSERT(expr) do { (void)sizeof(expr); } while (0)

#endif

#if SPEC_FS_BUILD_DEBUG
#define SPEC_FS_ASSUME(expr) SPEC_FS_ASSERT(expr)
#else
#if SPEC_FS_COMPILER_MSVC
#define SPEC_FS_ASSUME(expr) __assume(expr)
#elif SPEC_FS_COMPILER_CLANG || SPEC_FS_COMPILER_GCC
#define SPEC_FS_ASSUME(expr) do { if (!(expr)) __builtin_unreachable(); } while (0)
#else
#define SPEC_FS_ASSUME(expr) do { } while (0)
#endif
#endif

#if SPEC_FS_BUILD_DEBUG
#define SPEC_FS_DEBUG_ASSERT(expr) SPEC_FS_ASSERT(expr)
#define SPEC_FS_DEBUG_ASSUME(expr) SPEC_FS_ASSUME(expr)
#else
#define SPEC_FS_DEBUG_ASSERT(expr) do {} while (0)
#define SPEC_FS_DEBUG_ASSUME(expr) do {} while (0)
#endif

#define SPEC_FS_STATIC_ASSERT(expr, msg) static_assert(expr, msg)

#define SPEC_FS_UNUSED(x) (void)(x)
