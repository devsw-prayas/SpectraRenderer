#pragma once
#include "SpectraPlatformRuntime.h"
#include "FileUtils.h"
#include "MemoryUtils.h"
#include "PlatformThreadUtils.h"
#include "WindowUtils.h"

// Enum <-> raw Win32 value conversions, centralized here so no Public header
// ever needs to see Windows types. Definitions live in InternalHelpers.cpp
// (behind ALLOW_SYSCALL) - never inline these in the header.
namespace Spectra::Platform::Runtime::Internal {
	class FileMappings final {
	public:
		static uint32_t toWin32Access(File::FileAccess v_Access);
		static uint32_t toWin32ShareMode(File::FileShareMode v_Share);
		static uint32_t toWin32CreationDisposition(File::FileOpenMode v_OpenMode);
		static uint32_t toWin32MappingProtect(File::FileAccess v_Access);
		static uint32_t toWin32MapViewAccess(File::FileAccess v_Access);
		static uint32_t toWin32SeekMethod(File::FileSeekOrigin v_Origin);
	};

	class MemMappings final {
	public:
		static uint32_t toWin32Protect(Memory::MemoryProtect v_Protect);
		static Memory::MemoryProtect fromWin32Protect(uint32_t v_Protect);
		static Memory::MemoryState fromWin32MemState(uint32_t v_State);
	};

	class ThreadMappings final {
	public:
		static int32_t toWin32Priority(Thread::Priority v_Priority);
		static Thread::Priority fromWin32Priority(int32_t v_Priority);

#ifdef ALLOW_SYSCALL
#ifdef SPECTRA_COMPILER_MSVC
		static GROUP_AFFINITY fromAffinityDesc(const Thread::AffinityDesc& ro_Desc);
		static Thread::AffinityDesc toAffinityDesc(const GROUP_AFFINITY& ro_Affinity);
#endif
#endif
	};

	class WindowMappings final {
	public:
		static uint32_t toWin32Style(Windows::WindowStyleFlags v_Flags);
		static uint32_t toWin32ExStyle(Windows::WindowStyleFlags v_Flags);
		static int32_t toWin32HitTest(Windows::WindowHitTestResult v_Result);
		static uint16_t              toWin32VKey(Windows::KeyCode v_Code);
		static Windows::KeyCode      fromWin32VKey(uint16_t v_VKey);
		static uint32_t                    toWin32RimType(Windows::RawInputDeviceType v_Type);
		static Windows::RawInputDeviceType fromWin32RimType(uint32_t v_Type);
		static Windows::WindowVisualState fromWin32SizeParam(uint32_t v_SizeParam);
		static int32_t                    toWin32ShowCmd(Windows::WindowVisualState v_State);
	};

}
