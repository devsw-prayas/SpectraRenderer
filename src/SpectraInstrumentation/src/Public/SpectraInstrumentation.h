#pragma once

#ifndef SPEC_INST_RUNTIME_API
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(SPECTRA_INSTRUMENTATION_SHARED)
#define SPEC_INST_RUNTIME_API __declspec(dllexport)
#else
#define SPEC_INST_RUNTIME_API __declspec(dllimport)
#endif

#elif defined(__GNUC__) || defined(__clang__ )
#define SPEC_INST_RUNTIME_API __attribute__((visibility("default")))
#else
#define SPEC_INST_RUNTIME_API
#endif
#endif