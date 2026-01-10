#include <SpectraPlatformRuntime.h>
#include <PlatformThread.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#if defined(_MSVC_VER)
// TODO : Have to move to CMake
#pragma comment(lib, "synchronization.lib")
#endif
#include <Windows.h>
#endif

namespace Spectra::Platform::Runtime::Thread {
}