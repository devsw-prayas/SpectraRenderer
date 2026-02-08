#pragma once
#include "SpectraPlatformRuntime.h"
#include "PlatformAtomics.h"

namespace Spectra::Platform::Runtime::Thread {
	using Dword = uint32_t;

	using Flag = bool;
	Flag constexpr Allow = true;
	Flag constexpr Disallow = false;

	using Bytes = size_t;
	using Mask = size_t;

	using Name = const char*;
	using ProcessorIdx = size_t;

	enum class SPECTRA_RUNTIME_API Priority : int8_t {
		PRIORITY_IDLE = -15,
		PRIORITY_LOWEST = -2,
		PRIORITY_BELOW_NORMAL = -1,
		PRIORITY_NORMAL = 0,
		PRIORITY_ABOVE_NORMAL = 1,
		PRIORITY_HIGHEST = 2,
		PRIORITY_TIME_CRITICAL = 15,
	};

	enum class SPECTRA_RUNTIME_API ThreadState : uint8_t {
		CREATED, RUNNING, SEALED, REAPED
	};

	// [Payload]: 25B [sizeof]: 32B
	struct alignas(32) SPECTRA_RUNTIME_API ThreadHandle final {
		friend struct PlatformThread;
	private:
		size_t m_ThreadID;
		size_t m_Generation;

		size_t m_AccessToken;
		ThreadState m_State;
		constexpr ThreadHandle(size_t v_ThreadID, size_t v_Generation, size_t v_AccessToken, ThreadState v_State) :
			m_ThreadID(v_ThreadID), m_Generation(v_Generation), m_AccessToken(v_AccessToken), m_State(v_State) {
		}
	public:
		ThreadState expectedState() const;
		size_t getThreadID() const;

		static SPECTRA_FORCEINLINE ThreadHandle getInavlidHandle() {
			return {0,0,0, ThreadState::REAPED};
		}
	};

	namespace this_platform_thread {
	}

	// [Payload]: 52B [sizeof]: 64B
	struct alignas(64) SPECTRA_RUNTIME_API ThreadLaunchDesc final {
		friend struct PlatformThread;
	private:
		using Entry = void(*)(void*);

		void* m_StartContext = nullptr;
		Entry m_StartEntry = nullptr;

		void* m_StartupContext = nullptr;
		Entry m_StartupEntry = nullptr;

		void* m_ShutdownContext = nullptr;
		Entry m_ShutdownEntry = nullptr;

		Flag m_StartsSuspended;
	public:
		ThreadLaunchDesc() : m_StartsSuspended(Disallow) {}

		void launchFunction(Entry p_Launch, void* p_Ctx = nullptr);
		void startupHook(Entry p_StartupHook = nullptr, void* p_Ctx = nullptr);
		void shutdownHook(Entry p_ShutdownHook = nullptr, void* p_Ctx = nullptr);

		Flag preSuspend(Flag v_SuspendState);
	};

	// [Payload]: 40B [sizeof]: 64B
	struct SPECTRA_RUNTIME_API alignas(32) ThreadLaunchExecDesc final {
		// TODO Missing Detachable behavior
		ProcessorIdx m_AffinityMask;
		Dword        m_GroupID;

		Bytes        m_CommitSize;
		Bytes        m_ReserveSize;

		Dword        m_IdealProcessor;
		Priority     m_BasePriority;

		Flag         m_GuardsEnabled;
		Flag         m_PriorityBoost;
		Flag         m_SupportsIdealProcessor;
		Flag         m_SupportsThreadGroup;
	};

	void SPECTRA_RUNTIME_API initLaunchExecDesc(ThreadLaunchExecDesc& ro_Desc);
	void SPECTRA_RUNTIME_API commitStackSize(ThreadLaunchExecDesc& ro_Desc, Bytes v_Size);
	void SPECTRA_RUNTIME_API reserveStackSize(ThreadLaunchExecDesc& ro_Desc, Bytes v_Size);

	void SPECTRA_RUNTIME_API enableGuardPage(ThreadLaunchExecDesc& ro_Desc, Flag v_Permission);

	void SPECTRA_RUNTIME_API supportIdealProcessor(ThreadLaunchExecDesc& ro_Desc, Flag v_Permission);
	void SPECTRA_RUNTIME_API idealProcessor(ThreadLaunchExecDesc& ro_Desc, Dword v_Processor);

	void SPECTRA_RUNTIME_API supportProcessorGroups(ThreadLaunchExecDesc& ro_Desc, Flag v_Permission);
	void SPECTRA_RUNTIME_API groupID(ThreadLaunchExecDesc& ro_Desc, Dword v_GroupID);

	void SPECTRA_RUNTIME_API affinityMask(ThreadLaunchExecDesc& ro_Desc, ProcessorIdx v_Mask);

	template<size_t N>
	void SPECTRA_RUNTIME_API affinityMaskFromFlags(ThreadLaunchExecDesc& ro_Desc, std::array<Flag, N> v_Cores [[maybe_unused]] ) {
		ProcessorIdx mask = 0;
		for (size_t i = 0; i < N; i++) if (v_Cores[i]) mask |= (static_cast<ProcessorIdx>(1) << i);
		affinityMask(ro_Desc, mask);
	}

	template<size_t N>
	void SPECTRA_RUNTIME_API affinityMaskFromCores(ThreadLaunchExecDesc& ro_Desc, std::array<Dword, N> v_Cores [[maybe_unused]] ) {
		ProcessorIdx mask = 0;
		for (auto core : v_Cores) mask |= (static_cast<ProcessorIdx>(1) << core);
		affinityMask(ro_Desc, mask);
	}

	void SPECTRA_RUNTIME_API priorityBoosting(ThreadLaunchExecDesc& ro_Desc, Flag v_Permission);
	void SPECTRA_RUNTIME_API basePriority(ThreadLaunchExecDesc& ro_Desc, Priority v_BasePriority);

	bool SPECTRA_RUNTIME_API validateLaunchExecDesc(const ThreadLaunchExecDesc& ro_Desc);

	// [Payload] : 32B [sizeof] : 32B
	struct SPECTRA_RUNTIME_API alignas(32) ParkHandle final {
		Atomic::Atomic32 m_ParkingPermit;

		explicit ParkHandle(Atomic::Integer32 v_Value) : m_ParkingPermit(v_Value) {}
		~ParkHandle() = default;

		ParkHandle(const ParkHandle&) = default;
		ParkHandle& operator=(const ParkHandle&) = default;

		ParkHandle(ParkHandle&&) noexcept = default;
		ParkHandle& operator=(ParkHandle&&) noexcept = default;
	};
}
