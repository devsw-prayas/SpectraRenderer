#pragma once
#include <SpectraCompiler.h>

#if defined(SPECTRA_SHARED)

#if SPECTRA_COMPILER_MSVC
#if defined(SPECTRA_BUILDING_RUNTIME)
#define SPECTRA_RUNTIME_API __declspec(dllexport)
#else
#define SPECTRA_RUNTIME_API __declspec(dllimport)
#endif
#elif SPECTRA_COMPILER_CLANG || SPECTRA_COMPILER_GCC
#define SPECTRA_RUNTIME_API __attribute__((visibility("default")))
#else
#define SPECTRA_RUNTIME_API
#endif

#else
// Static build -> no import/export
#define SPECTRA_RUNTIME_API
#endif

#include <cstdint>
#include <functional>