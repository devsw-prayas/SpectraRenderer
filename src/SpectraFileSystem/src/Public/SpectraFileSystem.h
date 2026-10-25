#pragma once

#ifndef SPEC_FS_RUNTIME_API
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(SPECTRA_FILESYSTEM_SHARED)
#define SPEC_FS_RUNTIME_API __declspec(dllexport)
#else
#define SPEC_FS_RUNTIME_API __declspec(dllimport)
#endif

#elif defined(__GNUC__) || defined(__clang__ )
#define SPEC_FS_RUNTIME_API __attribute__((visibility("default")))
#else
#define SPEC_FS_RUNTIME_API
#endif
#endif

#include <cstdint>
#include <cstddef>
#include <bit>
#include <cstdio>
#include <cstring>
#include <new>
#include <span>
#include <string_view>
#include <type_traits>
