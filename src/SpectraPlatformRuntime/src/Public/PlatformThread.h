#pragma once
#include "SpectraPlatformRuntime.h"
#include "PlatformThreadUtils.h"

namespace Spectra::Platform::Runtime::Thread {

	class SPECTRA_RUNTIME_API PlatformThread final {
		static bool isValidHandle(const ThreadHandle& ro_Handle) noexcept;

	public:
		static ThreadHandle createThread(
			const ThreadLaunchDesc& ro_LaunchDesc, const ThreadLaunchExecDesc& ro_ExecDesc) noexcept;
		static bool detachThread(ThreadHandle v_Handle) noexcept;
		static bool closeHandle(ThreadHandle v_Handle) noexcept;
		static ThreadHandle duplicateHandle(ThreadHandle v_handle) noexcept;

		static bool isAlive(ThreadHandle v_Handle) noexcept;
		static ProcessorIdx getThreadID(ThreadHandle v_Handle) noexcept;

		static bool suspendThread(ThreadHandle v_Handle) noexcept;
		static bool resumeThread(ThreadHandle v_Handle) noexcept;
		static bool terminateThread(ThreadHandle v_Handle) noexcept;
		static bool joinThread(ThreadHandle v_Handle) noexcept;

		static bool setPriority(ThreadHandle v_Handle, Priority v_Priority) noexcept;
		static Priority getPriority(ThreadHandle v_Handle) noexcept;

		static bool setPriorityBoost(ThreadHandle v_Handle, Flag v_Permission) noexcept;
		static Flag getPriorityBoost(ThreadHandle v_Handle) noexcept;

		static bool setAffinity(ThreadHandle v_Handle, AffinityDesc& ro_Desc);
		static AffinityDesc getAffinity(ThreadHandle v_Handle);

		static void waitOnAddress(ParkHandle& ro_Permit) noexcept;
		static void wakeOnAddress(ParkHandle& ro_Permit) noexcept;
		static void wakeAllOnAddress(ParkHandle& ro_Permit) noexcept;

		static void waitOnAddressFor(ParkHandle& ro_Permit, uint32_t v_TimeoutMs) noexcept;
	};
}