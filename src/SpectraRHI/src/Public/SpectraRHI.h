#pragma once

#ifndef SPEC_RHI_RUNTIME_API
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(SPEC_RHI_SHARED)
#define SPEC_RHI_RUNTIME_API __declspec(dllexport)
#else
#define SPEC_RHI_RUNTIME_API __declspec(dllimport)
#endif

#elif defined(__GNUC__) || defined(__clang__ )
#define SPEC_RHI_RUNTIME_API __attribute__((visibility("default")))
#else
#define SPEC_RHI_RUNTIME_API
#endif
#endif

#ifndef ABSTRACT_RHI
#define ABSTRACT \
	= 0
#endif

#include <cstdint>

