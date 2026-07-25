#include <SpectraPlatformRuntime.h>
#include <PlatformThread.h>
#include <PlatformThreadUtils.h>
#include <SpectraCompiler.h>

#define ALLOW_SYSCALL
#include <SpectraSyscalls.h>

#if defined(SPECTRA_COMPILER_MSVC)
// TODO : Have to move to CMake
#pragma comment(lib, "synchronization.lib")
#endif

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

		RegistryEntry() noexcept {
			m_State.store(static_cast<uint32_t>(ThreadState::REAPED), Intrinsic::MemoryOrder::RELAXED);
		}

		size_t allocateToken() noexcept {
			for (;;) {
				uint64_t v_Mask = m_TokenMask.load(Intrinsic::MemoryOrder::RELAXED);
				uint64_t v_Free = ~v_Mask;
				if (v_Free == 0u) return INVALID_THREAD_SLOT;

				unsigned long v_Bit = 0;
#if defined(SPECTRA_COMPILER_MSVC)
				_BitScanForward64(&v_Bit, v_Free);
#else
				return INVALID_THREAD_SLOT;
#endif

				uint64_t v_Expected = v_Mask;
				uint64_t v_Desired = v_Mask | (1ull << v_Bit);
				m_TokenMask.compareExchange(
					&v_Expected,
					v_Desired,
					Intrinsic::MemoryOrder::ACQ_REL,
					Intrinsic::MemoryOrder::ACQUIRE);
				if (v_Expected == v_Mask) return static_cast<size_t>(v_Bit);
			}
		}

		void releaseToken(size_t v_Token) noexcept {
			uint64_t v_Bit = 1ull << v_Token;
			for (;;) {
				uint64_t v_Mask = m_TokenMask.load(Intrinsic::MemoryOrder::RELAXED);
				uint64_t v_Expected = v_Mask;
				uint64_t v_Desired = v_Mask & ~v_Bit;
				m_TokenMask.compareExchange(
					&v_Expected,
					v_Desired,
					Intrinsic::MemoryOrder::ACQ_REL,
					Intrinsic::MemoryOrder::ACQUIRE);
				if (v_Expected == v_Mask) return;
			}
		}

		bool hasOutstandingTokens() const noexcept {
			return m_TokenMask.load(Intrinsic::MemoryOrder::ACQUIRE) != 0u;
		}

		bool hasToken(size_t v_Token) const noexcept {
			if (v_Token >= 64u) return false;
			const uint64_t v_Bit = 1ull << v_Token;
			return (m_TokenMask.load(Intrinsic::MemoryOrder::ACQUIRE) & v_Bit) != 0u;
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
		const size_t v_Slot = ro_Handle.m_ThreadID;
		if (v_Slot >= SPECTRA_PLATFORM_MAX_THREADS) return false;

		const RegistryEntry& ro_Entry = g_Registry[v_Slot];
		if (ro_Entry.m_Generation != ro_Handle.m_Generation) return false;
		if (ro_Entry.state() == ThreadState::REAPED) return false;
		if (!ro_Entry.hasToken(ro_Handle.m_AccessToken)) return false;
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
		auto* p_Entry = static_cast<RegistryEntry*>(p_Raw);
		const size_t v_Slot = static_cast<size_t>(p_Entry - g_Registry);

		p_Entry->setState(ThreadState::RUNNING);

		this_platform_thread::t_MyHandle = ThreadHandle(
			v_Slot,
			p_Entry->m_Generation,
			0u,
			ThreadState::RUNNING);
		this_platform_thread::t_MyParkingPermit = ParkHandle{ 0u };

		if (p_Entry->m_StartupEntry) p_Entry->m_StartupEntry(p_Entry->m_StartupContext);
		if (p_Entry->m_StartEntry)   p_Entry->m_StartEntry(p_Entry->m_StartContext);
		if (p_Entry->m_ShutdownEntry) p_Entry->m_ShutdownEntry(p_Entry->m_ShutdownContext);

		p_Entry->setState(ThreadState::SEALED);
		return 0;
#else
		(void)p_Raw;
		return 0;
#endif
	}

	// PlatformThread — method implementations

	ThreadHandle PlatformThread::createThread(const ThreadLaunchDesc& ro_LaunchDesc, const ThreadLaunchExecDesc& ro_ExecDesc) noexcept {
		if (!validateLaunchExecDesc(ro_ExecDesc)) return ThreadHandle::getInvalidHandle();

		const size_t v_Slot = findFreeSlot();
		if (v_Slot == INVALID_THREAD_SLOT) return ThreadHandle::getInvalidHandle();

		RegistryEntry& r_Entry = g_Registry[v_Slot];
		const size_t v_Token = r_Entry.allocateToken();
		if (v_Token == INVALID_THREAD_SLOT) return ThreadHandle::getInvalidHandle();

#if defined(SPECTRA_COMPILER_MSVC)
		const uint32_t v_AttrCount = (ro_ExecDesc.m_SupportsThreadGroup ? 1u : 0u)
			+ (ro_ExecDesc.m_SupportsIdealProcessor ? 1u : 0u);

		LPPROC_THREAD_ATTRIBUTE_LIST p_AttrList = nullptr;
		LPVOID p_AttrListBuf = nullptr;

		if (v_AttrCount > 0) {
			SIZE_T v_AttrListSize = 0;
			InitializeProcThreadAttributeList(nullptr, v_AttrCount, 0, &v_AttrListSize);
			p_AttrListBuf = HeapAlloc(GetProcessHeap(), 0, v_AttrListSize);
			if (!p_AttrListBuf) {
				r_Entry.releaseToken(v_Token);
				return ThreadHandle::getInvalidHandle();
			}
			p_AttrList = static_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(p_AttrListBuf);
			InitializeProcThreadAttributeList(p_AttrList, v_AttrCount, 0, &v_AttrListSize);

			if (ro_ExecDesc.m_SupportsThreadGroup) {
				GROUP_AFFINITY v_GroupAffinity{};
				v_GroupAffinity.Mask = static_cast<KAFFINITY>(ro_ExecDesc.m_AffinityMask);
				v_GroupAffinity.Group = static_cast<WORD>(ro_ExecDesc.m_GroupID);
				UpdateProcThreadAttribute(p_AttrList, 0, PROC_THREAD_ATTRIBUTE_GROUP_AFFINITY,
					&v_GroupAffinity, sizeof(v_GroupAffinity), nullptr, nullptr);
			}
			if (ro_ExecDesc.m_SupportsIdealProcessor) {
				PROCESSOR_NUMBER v_ProcNum{};
				v_ProcNum.Group = static_cast<WORD>(ro_ExecDesc.m_GroupID);
				v_ProcNum.Number = static_cast<BYTE>(ro_ExecDesc.m_IdealProcessor);
				UpdateProcThreadAttribute(p_AttrList, 0, PROC_THREAD_ATTRIBUTE_IDEAL_PROCESSOR,
					&v_ProcNum, sizeof(v_ProcNum), nullptr, nullptr);
			}
		}

		DWORD v_CreateFlags = ro_LaunchDesc.m_StartsSuspended ? CREATE_SUSPENDED : 0u;
		SIZE_T v_StackSize = 0;
		if (ro_ExecDesc.m_ReserveSize != 0) {
			v_StackSize = static_cast<SIZE_T>(ro_ExecDesc.m_ReserveSize);
			v_CreateFlags |= STACK_SIZE_PARAM_IS_A_RESERVATION;
		} else if (ro_ExecDesc.m_CommitSize != 0) {
			v_StackSize = static_cast<SIZE_T>(ro_ExecDesc.m_CommitSize);
		}

		DWORD v_OsThreadId = 0;
		HANDLE v_OsHandle = CreateRemoteThreadEx(
			GetCurrentProcess(),
			nullptr,
			v_StackSize,
			Internal::PlatformThreadLaunchHelper::winThreadThunk,
			&r_Entry,
			v_CreateFlags,
			p_AttrList,
			&v_OsThreadId);

		if (p_AttrList) {
			DeleteProcThreadAttributeList(p_AttrList);
			HeapFree(GetProcessHeap(), 0, p_AttrListBuf);
		}

		if (!v_OsHandle || v_OsHandle == INVALID_HANDLE_VALUE) {
			r_Entry.releaseToken(v_Token);
			return ThreadHandle::getInvalidHandle();
		}

		int v_WinPriority = THREAD_PRIORITY_NORMAL;
		switch (ro_ExecDesc.m_BasePriority) {
		case Priority::PRIORITY_IDLE:          v_WinPriority = THREAD_PRIORITY_IDLE;          break;
		case Priority::PRIORITY_LOWEST:        v_WinPriority = THREAD_PRIORITY_LOWEST;        break;
		case Priority::PRIORITY_BELOW_NORMAL:  v_WinPriority = THREAD_PRIORITY_BELOW_NORMAL;  break;
		case Priority::PRIORITY_NORMAL:        v_WinPriority = THREAD_PRIORITY_NORMAL;        break;
		case Priority::PRIORITY_ABOVE_NORMAL:  v_WinPriority = THREAD_PRIORITY_ABOVE_NORMAL;  break;
		case Priority::PRIORITY_HIGHEST:       v_WinPriority = THREAD_PRIORITY_HIGHEST;       break;
		case Priority::PRIORITY_TIME_CRITICAL: v_WinPriority = THREAD_PRIORITY_TIME_CRITICAL; break;
		}
		SetThreadPriority(v_OsHandle, v_WinPriority);
		if (!ro_ExecDesc.m_PriorityBoost) SetThreadPriorityBoost(v_OsHandle, TRUE);

		r_Entry.m_OsHandle = v_OsHandle;
		r_Entry.m_OsThreadId = v_OsThreadId;
		r_Entry.m_StartContext = ro_LaunchDesc.m_StartContext;
		r_Entry.m_StartEntry = ro_LaunchDesc.m_StartEntry;
		r_Entry.m_StartupContext = ro_LaunchDesc.m_StartupContext;
		r_Entry.m_StartupEntry = ro_LaunchDesc.m_StartupEntry;
		r_Entry.m_ShutdownContext = ro_LaunchDesc.m_ShutdownContext;
		r_Entry.m_ShutdownEntry = ro_LaunchDesc.m_ShutdownEntry;
		++r_Entry.m_Generation;
		r_Entry.setState(ThreadState::CREATED);

		return ThreadHandle(v_Slot, r_Entry.m_Generation, v_Token, ThreadState::CREATED);
#else
		(void)ro_LaunchDesc;
		r_Entry.releaseToken(v_Token);
		return ThreadHandle::getInvalidHandle();
#endif
	}

	bool PlatformThread::detachThread(ThreadHandle v_Handle) noexcept {
		if (!isValidHandle(v_Handle)) return false;
		RegistryEntry& r_Entry = g_Registry[v_Handle.m_ThreadID];

		r_Entry.releaseToken(v_Handle.m_AccessToken);

		if (!r_Entry.hasOutstandingTokens()) {
#if defined(SPECTRA_COMPILER_MSVC)
			if (r_Entry.m_OsHandle != INVALID_HANDLE_VALUE) {
				CloseHandle(r_Entry.m_OsHandle);
				r_Entry.m_OsHandle = INVALID_HANDLE_VALUE;
				r_Entry.m_OsThreadId = 0;
			}
#endif
		}
		return true;
	}

	bool PlatformThread::closeHandle(ThreadHandle v_Handle) noexcept {
		if (!isValidHandle(v_Handle)) return false;
		RegistryEntry& r_Entry = g_Registry[v_Handle.m_ThreadID];

		r_Entry.releaseToken(v_Handle.m_AccessToken);

		if (!r_Entry.hasOutstandingTokens()) {
#if defined(SPECTRA_COMPILER_MSVC)
			if (r_Entry.m_OsHandle != INVALID_HANDLE_VALUE) {
				CloseHandle(r_Entry.m_OsHandle);
				r_Entry.m_OsHandle = INVALID_HANDLE_VALUE;
				r_Entry.m_OsThreadId = 0;
			}
#endif
			r_Entry.setState(ThreadState::REAPED);
		}
		return true;
	}

	ThreadHandle PlatformThread::duplicateHandle(ThreadHandle v_handle) noexcept {
		if (!isValidHandle(v_handle)) return ThreadHandle::getInvalidHandle();
		RegistryEntry& r_Entry = g_Registry[v_handle.m_ThreadID];

		const size_t v_NewToken = r_Entry.allocateToken();
		if (v_NewToken == INVALID_THREAD_SLOT) return ThreadHandle::getInvalidHandle();

		return ThreadHandle(v_handle.m_ThreadID, r_Entry.m_Generation, v_NewToken, r_Entry.state());
	}

	bool PlatformThread::isAlive(ThreadHandle v_Handle) noexcept {
		if (!isValidHandle(v_Handle)) return false;
#if defined(SPECTRA_COMPILER_MSVC)
		const RegistryEntry& ro_Entry = g_Registry[v_Handle.m_ThreadID];
		if (ro_Entry.m_OsHandle == INVALID_HANDLE_VALUE) return false;

		DWORD v_ExitCode = 0;
		if (!GetExitCodeThread(ro_Entry.m_OsHandle, &v_ExitCode)) return false;
		return v_ExitCode == STILL_ACTIVE;
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
		const HANDLE v_OsHandle = g_Registry[v_Handle.m_ThreadID].m_OsHandle;
		if (v_OsHandle == INVALID_HANDLE_VALUE) return false;
		return ::SuspendThread(v_OsHandle) != static_cast<DWORD>(-1);
#else
		return false;
#endif
	}

	bool PlatformThread::resumeThread(ThreadHandle v_Handle) noexcept {
		if (!isValidHandle(v_Handle)) return false;
#if defined(SPECTRA_COMPILER_MSVC)
		const HANDLE v_OsHandle = g_Registry[v_Handle.m_ThreadID].m_OsHandle;
		if (v_OsHandle == INVALID_HANDLE_VALUE) return false;
		return ::ResumeThread(v_OsHandle) != static_cast<DWORD>(-1);
#else
		return false;
#endif
	}

	bool PlatformThread::terminateThread(ThreadHandle v_Handle) noexcept {
		if (!isValidHandle(v_Handle)) return false;
#if defined(SPECTRA_COMPILER_MSVC)
		RegistryEntry& r_Entry = g_Registry[v_Handle.m_ThreadID];
		if (r_Entry.m_OsHandle == INVALID_HANDLE_VALUE) return false;

		const bool v_Ok = TerminateThread(r_Entry.m_OsHandle, 0) != FALSE;
		if (v_Ok) r_Entry.setState(ThreadState::SEALED);
		return v_Ok;
#else
		return false;
#endif
	}

	bool PlatformThread::joinThread(ThreadHandle v_Handle) noexcept {
		if (!isValidHandle(v_Handle)) return false;
#if defined(SPECTRA_COMPILER_MSVC)
		const HANDLE v_OsHandle = g_Registry[v_Handle.m_ThreadID].m_OsHandle;
		if (v_OsHandle == INVALID_HANDLE_VALUE) return false;
		return WaitForSingleObject(v_OsHandle, INFINITE) == WAIT_OBJECT_0;
#else
		return false;
#endif
	}

	void PlatformThread::waitOnAddress(ParkHandle& ro_Permit) noexcept {
#if defined(SPECTRA_COMPILER_MSVC)
		uint32_t v_Expected = 0u;
		WaitOnAddress(ro_Permit.m_ParkingPermit.data(), &v_Expected, sizeof(uint32_t), INFINITE);
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
		uint32_t v_Expected = 0u;
		WaitOnAddress(ro_Permit.m_ParkingPermit.data(), &v_Expected, sizeof(uint32_t), static_cast<DWORD>(v_TimeoutMs));
#else
		(void)ro_Permit;
		(void)v_TimeoutMs;
#endif
	}
}
