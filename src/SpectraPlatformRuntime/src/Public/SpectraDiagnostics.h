#pragma once
#include <SpectraCompiler.h>

// SPECTRA_DEBUG_CHECKS (0/1) comes from SPECTRA_ENABLE_DEBUG_CHECKS=ON/OFF; unset (AUTO) follows _DEBUG.
#if defined(SPECTRA_DEBUG_CHECKS)
#define SPECTRA_BUILD_DEBUG SPECTRA_DEBUG_CHECKS
#elif defined(_DEBUG)
#define SPECTRA_BUILD_DEBUG 1
#else
#define SPECTRA_BUILD_DEBUG 0
#endif
#define SPECTRA_BUILD_RELEASE (!SPECTRA_BUILD_DEBUG)

#if SPECTRA_BUILD_DEBUG
#define SPECTRA_ASSERT(expr)                        \
        do {                                            \
            if (!(expr)) {                             \
                SPECTRA_DEBUG_BREAK();                 \
                SPECTRA_TRAP();                        \
            }                                           \
        } while (0)
#else
#define SPECTRA_ASSERT(expr) do { (void)sizeof(expr); } while (0)
#endif

#if SPECTRA_BUILD_DEBUG
#define SPECTRA_ASSUME(expr) SPECTRA_ASSERT(expr)
#else
#if SPECTRA_COMPILER_MSVC
#define SPECTRA_ASSUME(expr) __assume(expr)
#else
#define SPECTRA_ASSUME(expr) do { if (!(expr)) SPECTRA_UNREACHABLE(); } while (0)
#endif
#endif

#if SPECTRA_BUILD_DEBUG
#define SPECTRA_DEBUG_ASSERT(expr) SPECTRA_ASSERT(expr)
#define SPECTRA_DEBUG_ASSUME(expr) SPECTRA_ASSUME(expr)
#else
#define SPECTRA_DEBUG_ASSERT(expr) do {} while (0)
#define SPECTRA_DEBUG_ASSUME(expr) do {} while (0)
#endif

#define SPECTRA_STATIC_ASSERT(expr, msg) static_assert(expr, msg)
#define SPECTRA_UNUSED(x) (void)(x)
