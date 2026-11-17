#pragma once
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <atomic>
#include <thread>
#include <memory>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#ifdef BOOTSTRAP_EXPORTS
#  define BOOTSTRAP_API __declspec(dllexport)
#else
#  define BOOTSTRAP_API __declspec(dllimport)
#endif

BOOTSTRAP_API int bootstrapRun();
