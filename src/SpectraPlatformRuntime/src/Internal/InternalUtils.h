#pragma once
#include "SpectraPlatformRuntime.h"
#include "FileUtils.h"
#include "MemoryUtils.h"
#include "PlatformThreadUtils.h"

// Enum <-> raw Win32 value conversions, centralized here so no Public header
// ever needs to see Windows types. Definitions live in InternalHelpers.cpp
// (behind ALLOW_SYSCALL) - never inline these in the header.
namespace Spectra::Platform::Runtime::Internal {
	uint32_t toWin32Access(File::FileAccess v_Access);
	uint32_t toWin32ShareMode(File::FileShareMode v_Share);
	uint32_t toWin32CreationDisposition(File::FileOpenMode v_OpenMode);
	uint32_t toWin32MappingProtect(File::FileAccess v_Access);
	uint32_t toWin32MapViewAccess(File::FileAccess v_Access);
	uint32_t toWin32SeekMethod(File::FileSeekOrigin v_Origin);

	uint32_t toWin32Protect(Memory::MemoryProtect v_Protect);
	Memory::MemoryProtect fromWin32Protect(uint32_t v_Protect);
	Memory::MemoryState fromWin32MemState(uint32_t v_State);

	int32_t toWin32Priority(Thread::Priority v_Priority);
	Thread::Priority fromWin32Priority(int32_t v_Priority);

#ifdef ALLOW_SYSCALL
	GROUP_AFFINITY fromAffinityDesc(const Thread::AffinityDesc& ro_Desc);
	Thread::AffinityDesc toAffinityDesc(const GROUP_AFFINITY& ro_Affinity);
#endif
}
