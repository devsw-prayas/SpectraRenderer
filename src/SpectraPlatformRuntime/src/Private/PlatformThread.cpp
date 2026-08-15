#include <SpectraPlatformRuntime.h>
#include <PlatformThread.h>
#include <PlatformThreadUtils.h>
#include <SpectraCompiler.h>

#define ALLOW_SYSCALL
#include <SpectraSyscalls.h>
#include <InternalUtils.h>

namespace Spectra::Platform::Runtime::Thread {
	constexpr size_t INVALID_THREAD_SLOT = static_cast<size_t>(-1);
	using ThreadEntry = void(*)(void*);

	struct alignas(64) RegistryEntry final {
		HANDLE m_OsHandle{ INVALID_HANDLE_VALUE };
		DWORD m_OsThreadId{ 0 };
		uint32_t m_Pad0{ 0 };
		uint64_t m_Generation{ 0 };
		Atomic::AtomicValue64<uint64_t> m_TokenMask{};
		Atomic::AtomicValue32<uint32_t> m_State{};

		void* m_StartContext{ nullptr };
		ThreadEntry m_StartEntry{ nullptr };
		void* m_StartupContext{ nullptr };
		ThreadEntry m_StartupEntry{ nullptr };
		void* m_ShutdownContext{ nullptr };
		ThreadEntry m_ShutdownEntry{ nullptr };
		Flag m_CanDetach{ Disallow };

		RegistryEntry() noexcept {
			m_State.store(static_cast<uint32_t>(ThreadState::REAPED), Intrinsic::MemoryOrder::RELAXED);
		}

		size_t allocateToken() noexcept {
			for (;;) {
				uint64_t mask = m_TokenMask.load(Intrinsic::MemoryOrder::RELAXED);
				uint64_t freeMask = ~mask;
				if (freeMask == 0u) return INVALID_THREAD_SLOT;

				unsigned long bit = 0;
#if defined(SPECTRA_COMPILER_MSVC)
				_BitScanForward64(&bit, freeMask);
#else
				return INVALID_THREAD_SLOT;
#endif

				uint64_t expected = mask;
				uint64_t desired = mask | (1ull << bit);
				m_TokenMask.compareExchange(
					&expected,
					desired,
					Intrinsic::MemoryOrder::ACQ_REL,
					Intrinsic::MemoryOrder::ACQUIRE);
				if (expected == mask) return static_cast<size_t>(bit);
			}
		}

		void releaseToken(size_t v_Token) noexcept {
			uint64_t bit = 1ull << v_Token;
			for (;;) {
				uint64_t mask = m_TokenMask.load(Intrinsic::MemoryOrder::RELAXED);
				uint64_t expected = mask;
				uint64_t desired = mask & ~bit;
				m_TokenMask.compareExchange(
					&expected,
					desired,
					Intrinsic::MemoryOrder::ACQ_REL,
					Intrinsic::MemoryOrder::ACQUIRE);
				if (expected == mask) return;
			}
		}

		bool hasOutstandingTokens() const noexcept {
			return m_TokenMask.load(Intrinsic::MemoryOrder::ACQUIRE) != 0u;
		}

		bool hasToken(size_t v_Token) const noexcept {
			if (v_Token >= 64u) return false;
			const uint64_t bit = 1ull << v_Token;
			return (m_TokenMask.load(Intrinsic::MemoryOrder::ACQUIRE) & bit) != 0u;
		}

		ThreadState state() const noexcept {
			return static_cast<ThreadState>(m_State.load(Intrinsic::MemoryOrder::ACQUIRE));
		}

		void setState(ThreadState v_State) noexcept {
			m_State.store(static_cast<uint32_t>(v_State), Intrinsic::MemoryOrder::RELEASE);
		}
	};

	namespace {
		RegistryEntry g_Registry[SPECTRA_PLATFORM_MAX_THREADS];

		size_t findFreeSlot() noexcept {
			for (size_t i = 0; i < SPECTRA_PLATFORM_MAX_THREADS; ++i) {
				if (g_Registry[i].state() == ThreadState::REAPED && !g_Registry[i].hasOutstandingTokens()) return i;
			}
			return INVALID_THREAD_SLOT;
		}
	}

	bool PlatformThread::isValidHandle(const ThreadHandle& ro_Handle) noexcept {
		const size_t slot = ro_Handle.m_ThreadID;
		if (slot >= SPECTRA_PLATFORM_MAX_THREADS) return false;

		const RegistryEntry& entry = g_Registry[slot];
		if (entry.m_Generation != ro_Handle.m_Generation) return false;
		if (entry.state() == ThreadState::REAPED) return false;
		if (!entry.hasToken(ro_Handle.m_AccessToken)) return false;
		return true;
	}

	// PlatformThreadLaunchHelper — owns the Win32 thunk so DWORD/WINAPI never
	// appear in the public header. Friended by ThreadHandle for private
	// constructor access (see PlatformThreadUtils.h).

	namespace Internal {
		struct PlatformThreadLaunchHelper final {
			static DWORD WINAPI winThreadThunk(void* p_Raw) noexcept;
		};
	}

	DWORD WINAPI Internal::PlatformThreadLaunchHelper::winThreadThunk(void* p_Raw) noexcept {
#if defined(SPECTRA_COMPILER_MSVC)
		auto* entry = static_cast<RegistryEntry*>(p_Raw);
		const size_t slot = static_cast<size_t>(entry - g_Registry);

		entry->setState(ThreadState::RUNNING);

		this_platform_thread::t_MyHandle = ThreadHandle(
			slot,
			entry->m_Generation,
			0u,
			ThreadState::RUNNING);
		this_platform_thread::t_MyParkingPermit = ParkHandle{ 0u };

		if (entry->m_StartupEntry) entry->m_StartupEntry(entry->m_StartupContext);
		if (entry->m_StartEntry)   entry->m_StartEntry(entry->m_StartContext);
		if (entry->m_ShutdownEntry) entry->m_ShutdownEntry(entry->m_ShutdownContext);

		entry->setState(ThreadState::SEALED);
		return 0;
#else
		(void)p_Raw;
		return 0;
#endif
	}

	// PlatformThread — method implementations

	ThreadHandle PlatformThread::createThread(const ThreadLaunchDesc& ro_LaunchDesc, const ThreadLaunchExecDesc& ro_ExecDesc) noexcept {
		if (!validateLaunchExecDesc(ro_ExecDesc)) return ThreadHandle::getInvalidHandle();

		const size_t slot = findFreeSlot();
		if (slot == INVALID_THREAD_SLOT) return ThreadHandle::getInvalidHandle();

		RegistryEntry& entry = g_Registry[slot];
		const size_t token = entry.allocateToken();
		if (token == INVALID_THREAD_SLOT) return ThreadHandle::getInvalidHandle();

#if defined(SPECTRA_COMPILER_MSVC)
		const uint32_t attributeCount = (ro_ExecDesc.m_SupportsThreadGroup ? 1u : 0u)
			+ (ro_ExecDesc.m_SupportsIdealProcessor ? 1u : 0u);

		LPPROC_THREAD_ATTRIBUTE_LIST attributeList = nullptr;
		LPVOID attributeListBuf = nullptr;

		if (attributeCount > 0) {
			SIZE_T attributeListSize = 0;
			InitializeProcThreadAttributeList(nullptr, attributeCount, 0, &attributeListSize);
			attributeListBuf = HeapAlloc(GetProcessHeap(), 0, attributeListSize);
			if (!attributeListBuf) {
				entry.releaseToken(token);
				return ThreadHandle::getInvalidHandle();
			}
			attributeList = static_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attributeListBuf);
			InitializeProcThreadAttributeList(attributeList, attributeCount, 0, &attributeListSize);

			if (ro_ExecDesc.m_SupportsThreadGroup) {
				GROUP_AFFINITY groupAffinity{};
				groupAffinity.Mask = static_cast<KAFFINITY>(ro_ExecDesc.m_Desc.m_AffMask);
				groupAffinity.Group = static_cast<WORD>(ro_ExecDesc.m_Desc.m_GroupId);
				UpdateProcThreadAttribute(attributeList, 0, PROC_THREAD_ATTRIBUTE_GROUP_AFFINITY,
					&groupAffinity, sizeof(groupAffinity), nullptr, nullptr);
			}
			if (ro_ExecDesc.m_SupportsIdealProcessor) {
				PROCESSOR_NUMBER processorNumber{};
				processorNumber.Group = static_cast<WORD>(ro_ExecDesc.m_Desc.m_GroupId);
				processorNumber.Number = static_cast<BYTE>(ro_ExecDesc.m_IdealProcessor);
				UpdateProcThreadAttribute(attributeList, 0, PROC_THREAD_ATTRIBUTE_IDEAL_PROCESSOR,
					&processorNumber, sizeof(processorNumber), nullptr, nullptr);
			}
		}

		DWORD createFlags = ro_LaunchDesc.m_StartsSuspended ? CREATE_SUSPENDED : 0u;
		SIZE_T stackSize = 0;
		if (ro_ExecDesc.m_ReserveSize != 0) {
			stackSize = static_cast<SIZE_T>(ro_ExecDesc.m_ReserveSize);
			createFlags |= STACK_SIZE_PARAM_IS_A_RESERVATION;
		} else if (ro_ExecDesc.m_CommitSize != 0) {
			stackSize = static_cast<SIZE_T>(ro_ExecDesc.m_CommitSize);
		}

		DWORD osThreadId = 0;
		HANDLE osHandle = CreateRemoteThreadEx(
			GetCurrentProcess(),
			nullptr,
			stackSize,
			Internal::PlatformThreadLaunchHelper::winThreadThunk,
			&entry,
			createFlags,
			attributeList,
			&osThreadId);

		if (attributeList) {
			DeleteProcThreadAttributeList(attributeList);
			HeapFree(GetProcessHeap(), 0, attributeListBuf);
		}

		if (!osHandle || osHandle == INVALID_HANDLE_VALUE) {
			entry.releaseToken(token);
			return ThreadHandle::getInvalidHandle();
		}

		int winPriority = Spectra::Platform::Runtime::Internal::ThreadMappings::toWin32Priority(ro_ExecDesc.m_BasePriority);
		SetThreadPriority(osHandle, winPriority);
		if (!ro_ExecDesc.m_PriorityBoost) SetThreadPriorityBoost(osHandle, TRUE);

		entry.m_OsHandle = osHandle;
		entry.m_OsThreadId = osThreadId;
		entry.m_StartContext = ro_LaunchDesc.m_StartContext;
		entry.m_StartEntry = ro_LaunchDesc.m_StartEntry;
		entry.m_StartupContext = ro_LaunchDesc.m_StartupContext;
		entry.m_StartupEntry = ro_LaunchDesc.m_StartupEntry;
		entry.m_ShutdownContext = ro_LaunchDesc.m_ShutdownContext;
		entry.m_ShutdownEntry = ro_LaunchDesc.m_ShutdownEntry;
		entry.m_CanDetach = ro_ExecDesc.m_CanDetach;
		++entry.m_Generation;
		entry.setState(ThreadState::CREATED);

		return ThreadHandle(slot, entry.m_Generation, token, ThreadState::CREATED);
#else
		(void)ro_LaunchDesc;
		entry.releaseToken(token);
		return ThreadHandle::getInvalidHandle();
#endif
	}

	bool PlatformThread::detachThread(ThreadHandle v_Handle) noexcept {
		if (!isValidHandle(v_Handle)) return false;
		RegistryEntry& entry = g_Registry[v_Handle.m_ThreadID];
		if (!entry.m_CanDetach) return false;

		entry.releaseToken(v_Handle.m_AccessToken);

		if (!entry.hasOutstandingTokens()) {
#if defined(SPECTRA_COMPILER_MSVC)
			if (entry.m_OsHandle != INVALID_HANDLE_VALUE) {
				CloseHandle(entry.m_OsHandle);
				entry.m_OsHandle = INVALID_HANDLE_VALUE;
				entry.m_OsThreadId = 0;
			}
#endif
		}
		return true;
	}

	bool PlatformThread::closeHandle(ThreadHandle v_Handle) noexcept {
		if (!isValidHandle(v_Handle)) return false;
		RegistryEntry& entry = g_Registry[v_Handle.m_ThreadID];

		entry.releaseToken(v_Handle.m_AccessToken);

		if (!entry.hasOutstandingTokens()) {
#if defined(SPECTRA_COMPILER_MSVC)
			if (entry.m_OsHandle != INVALID_HANDLE_VALUE) {
				CloseHandle(entry.m_OsHandle);
				entry.m_OsHandle = INVALID_HANDLE_VALUE;
				entry.m_OsThreadId = 0;
			}
#endif
			entry.setState(ThreadState::REAPED);
		}
		return true;
	}

	ThreadHandle PlatformThread::duplicateHandle(ThreadHandle v_handle) noexcept {
		if (!isValidHandle(v_handle)) return ThreadHandle::getInvalidHandle();
		RegistryEntry& entry = g_Registry[v_handle.m_ThreadID];

		const size_t newToken = entry.allocateToken();
		if (newToken == INVALID_THREAD_SLOT) return ThreadHandle::getInvalidHandle();

		return ThreadHandle(v_handle.m_ThreadID, entry.m_Generation, newToken, entry.state());
	}

	bool PlatformThread::isAlive(ThreadHandle v_Handle) noexcept {
		if (!isValidHandle(v_Handle)) return false;
#if defined(SPECTRA_COMPILER_MSVC)
		const RegistryEntry& entry = g_Registry[v_Handle.m_ThreadID];
		if (entry.m_OsHandle == INVALID_HANDLE_VALUE) return false;

		DWORD exitCode = 0;
		if (!GetExitCodeThread(entry.m_OsHandle, &exitCode)) return false;
		return exitCode == STILL_ACTIVE;
#else
		return false;
#endif
	}

	ProcessorIdx PlatformThread::getThreadID(ThreadHandle v_Handle) noexcept {
		if (!isValidHandle(v_Handle)) return 0;
		return static_cast<ProcessorIdx>(g_Registry[v_Handle.m_ThreadID].m_OsThreadId);
	}

	bool PlatformThread::suspendThread(ThreadHandle v_Handle) noexcept {
		if (!isValidHandle(v_Handle)) return false;
#if defined(SPECTRA_COMPILER_MSVC)
		const HANDLE osHandle = g_Registry[v_Handle.m_ThreadID].m_OsHandle;
		if (osHandle == INVALID_HANDLE_VALUE) return false;
		return ::SuspendThread(osHandle) != static_cast<DWORD>(-1);
#else
		return false;
#endif
	}

	bool PlatformThread::resumeThread(ThreadHandle v_Handle) noexcept {
		if (!isValidHandle(v_Handle)) return false;
#if defined(SPECTRA_COMPILER_MSVC)
		const HANDLE osHandle = g_Registry[v_Handle.m_ThreadID].m_OsHandle;
		if (osHandle == INVALID_HANDLE_VALUE) return false;
		return ::ResumeThread(osHandle) != static_cast<DWORD>(-1);
#else
		return false;
#endif
	}

	bool PlatformThread::terminateThread(ThreadHandle v_Handle) noexcept {
		if (!isValidHandle(v_Handle)) return false;
#if defined(SPECTRA_COMPILER_MSVC)
		RegistryEntry& entry = g_Registry[v_Handle.m_ThreadID];
		if (entry.m_OsHandle == INVALID_HANDLE_VALUE) return false;

		const bool ok = TerminateThread(entry.m_OsHandle, 0) != FALSE;
		if (ok) entry.setState(ThreadState::SEALED);
		return ok;
#else
		return false;
#endif
	}

	bool PlatformThread::joinThread(ThreadHandle v_Handle) noexcept {
		if (!isValidHandle(v_Handle)) return false;
#if defined(SPECTRA_COMPILER_MSVC)
		const HANDLE osHandle = g_Registry[v_Handle.m_ThreadID].m_OsHandle;
		if (osHandle == INVALID_HANDLE_VALUE) return false;
		return WaitForSingleObject(osHandle, INFINITE) == WAIT_OBJECT_0;
#else
		return false;
#endif
	}

	void PlatformThread::waitOnAddress(ParkHandle& ro_Permit) noexcept {
#if defined(SPECTRA_COMPILER_MSVC)
		uint32_t expected = 0u;
		WaitOnAddress(ro_Permit.m_ParkingPermit.data(), &expected, sizeof(uint32_t), INFINITE);
#else
		(void)ro_Permit;
#endif
	}

	void PlatformThread::wakeOnAddress(ParkHandle& ro_Permit) noexcept {
#if defined(SPECTRA_COMPILER_MSVC)
		ro_Permit.m_ParkingPermit.store(1u, Intrinsic::MemoryOrder::RELEASE);
		WakeByAddressSingle(ro_Permit.m_ParkingPermit.data());
#else
		(void)ro_Permit;
#endif
	}

	void PlatformThread::wakeAllOnAddress(ParkHandle& ro_Permit) noexcept {
#if defined(SPECTRA_COMPILER_MSVC)
		ro_Permit.m_ParkingPermit.store(1u, Intrinsic::MemoryOrder::RELEASE);
		WakeByAddressAll(ro_Permit.m_ParkingPermit.data());
#else
		(void)ro_Permit;
#endif
	}

	void PlatformThread::waitOnAddressFor(ParkHandle& ro_Permit, uint32_t v_TimeoutMs) noexcept {
#if defined(SPECTRA_COMPILER_MSVC)
		uint32_t expected = 0u;
		WaitOnAddress(ro_Permit.m_ParkingPermit.data(), &expected, sizeof(uint32_t), static_cast<DWORD>(v_TimeoutMs));
#else
		(void)ro_Permit;
		(void)v_TimeoutMs;
#endif
	}

	bool PlatformThread::setAffinity(ThreadHandle v_Handle, AffinityDesc& ro_Desc) {
#if defined(SPECTRA_COMPILER_MSVC)
		if (!isValidHandle(v_Handle)) return false;
		const HANDLE osHandle = g_Registry[v_Handle.m_ThreadID].m_OsHandle;
		if (osHandle == INVALID_HANDLE_VALUE) return false;

		GROUP_AFFINITY affinity = Spectra::Platform::Runtime::Internal::ThreadMappings::fromAffinityDesc(ro_Desc);
		return ::SetThreadGroupAffinity(osHandle, &affinity, nullptr) != FALSE;
#else
		(void)v_Handle;
		(void)ro_Desc;
		return false;
#endif
	}

	AffinityDesc PlatformThread::getAffinity(ThreadHandle v_Handle) {
		AffinityDesc desc{};
#if defined(SPECTRA_COMPILER_MSVC)
		if (!isValidHandle(v_Handle)) return desc;
		const HANDLE osHandle = g_Registry[v_Handle.m_ThreadID].m_OsHandle;
		if (osHandle == INVALID_HANDLE_VALUE) return desc;

		GROUP_AFFINITY affinity{};
		if (!::GetThreadGroupAffinity(osHandle, &affinity)) return desc;
		desc = Runtime::Internal::ThreadMappings::toAffinityDesc(affinity);
		return desc;
#else
		(void)v_Handle;
		return desc;
#endif
	}

	bool PlatformThread::setPriority(ThreadHandle v_Handle, Priority v_Priority) noexcept {
#if defined(SPECTRA_COMPILER_MSVC)
		if (!isValidHandle(v_Handle)) return false;
		const HANDLE osHandle = g_Registry[v_Handle.m_ThreadID].m_OsHandle;
		if (osHandle == INVALID_HANDLE_VALUE) return false;

		const int winPriority = Spectra::Platform::Runtime::Internal::ThreadMappings::toWin32Priority(v_Priority);
		return ::SetThreadPriority(osHandle, winPriority) != FALSE;
#else
		(void)v_Handle;
		(void)v_Priority;
		return false;
#endif
	}

	Priority PlatformThread::getPriority(ThreadHandle v_Handle) noexcept {
#if defined(SPECTRA_COMPILER_MSVC)
		if (!isValidHandle(v_Handle)) return Priority::PRIORITY_NORMAL;
		const HANDLE osHandle = g_Registry[v_Handle.m_ThreadID].m_OsHandle;
		if (osHandle == INVALID_HANDLE_VALUE) return Priority::PRIORITY_NORMAL;

		const int winPriority = ::GetThreadPriority(osHandle);
		if (winPriority == THREAD_PRIORITY_ERROR_RETURN) return Priority::PRIORITY_NORMAL;
		return Spectra::Platform::Runtime::Internal::ThreadMappings::fromWin32Priority(winPriority);
#else
		(void)v_Handle;
		return Priority::PRIORITY_NORMAL;
#endif
	}

	bool PlatformThread::setPriorityBoost(ThreadHandle v_Handle, Flag v_Permission) noexcept {
#if defined(SPECTRA_COMPILER_MSVC)
		if (!isValidHandle(v_Handle)) return false;
		const HANDLE osHandle = g_Registry[v_Handle.m_ThreadID].m_OsHandle;
		if (osHandle == INVALID_HANDLE_VALUE) return false;

		// SetThreadPriorityBoost takes "disable" semantics - inverted from v_Permission.
		return ::SetThreadPriorityBoost(osHandle, v_Permission ? FALSE : TRUE) != FALSE;
#else
		(void)v_Handle;
		(void)v_Permission;
		return false;
#endif
	}

	Flag PlatformThread::getPriorityBoost(ThreadHandle v_Handle) noexcept {
#if defined(SPECTRA_COMPILER_MSVC)
		if (!isValidHandle(v_Handle)) return Disallow;
		const HANDLE osHandle = g_Registry[v_Handle.m_ThreadID].m_OsHandle;
		if (osHandle == INVALID_HANDLE_VALUE) return Disallow;

		BOOL disabled = FALSE;
		if (!::GetThreadPriorityBoost(osHandle, &disabled)) return Disallow;
		return disabled == FALSE;
#else
		(void)v_Handle;
		return Disallow;
#endif
	}
}
