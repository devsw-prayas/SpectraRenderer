#pragma once
#include "SpectraPlatformRuntime.h"
#include <cstdint>
#include <functional>

namespace Spectra::Platform::Runtime::Thread {									

	using Dword = uint32_t;

	using Flag = bool;
	Flag constexpr Allow = true;
	Flag constexpr Disallow = false;

	using Bytes = size_t;
	using Mask = size_t;

	using Name = const char*;
	using ProcessorIdx = size_t;

	enum class RUNTIME Priority : int8_t {
		PRIORITY_IDLE = -15,
		PRIORITY_LOWEST = -2,
		PRIORITY_BELOW_NORMAL = -1,
		PRIORITY_NORMAL = 0,
		PRIORITY_ABOVE_NORMAL = 1,
		PRIORITY_HIGHEST = 2,
		PRIORITY_TIME_CRITICAL = 15,
	};

	enum class RUNTIME ThreadState : uint8_t {
		CREATED, RUNNING, SEALED, REAPED
	};

	// [Payload]: 25B [sizeof]: 32B
	struct alignas(32) RUNTIME ThreadHandle final {
	private:
		size_t m_ThreadID;
		size_t m_Generation;

		size_t m_AccessToken;
		ThreadState m_State;
		ThreadHandle(size_t v_ThreadID, size_t v_Generation, size_t v_AccessToken, ThreadState v_State) :
			m_ThreadID(v_ThreadID), m_Generation(v_Generation), m_AccessToken(v_AccessToken), m_State(v_State) {
		}
	public:
		ThreadState expectedState() const;
		size_t getThreadID() const;
	};

	namespace this_platform_thread {

	}

	// [Payload]: 208B [sizeof]: 224B
	struct alignas(32) RUNTIME ThreadLaunchDesc final {
	private:
		std::function<void()> m_StartLocation;
		std::function<void()> m_StartupHook;
		std::function<void()> m_ShutdownHook;

		Flag m_StartsSuspended;

	public:
		ThreadLaunchDesc() : m_StartLocation(nullptr), m_StartupHook(nullptr), m_ShutdownHook(nullptr),
			m_StartsSuspended(Disallow) {}

		void launchFunction(auto&& u_StartLocation);
		void startupHook(auto&& u_StartupHook = nullptr);
		void shutdownHook(auto&& u_ShutdownHook = nullptr);

		Flag preSuspend(Flag v_SuspendState);
	};

	// [Payload]: 40B [sizeof]: 64B
	struct RUNTIME alignas(32) ThreadLaunchExecDesc final {
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

	void initLaunchExecDesc(ThreadLaunchExecDesc& ro_Desc);
	void commitStackSize(ThreadLaunchExecDesc& ro_Desc, Bytes v_Size);
	void reserveStackSize(ThreadLaunchExecDesc& ro_Desc, Bytes v_Size);

	void enableGuardPage(ThreadLaunchExecDesc& ro_Desc, Flag v_Permission);

	void supportIdealProcessor(ThreadLaunchExecDesc& ro_Desc, Flag v_Permission);
	void idealProcessor(ThreadLaunchExecDesc& ro_Desc, Dword v_Processor);

	void supportProcessorGroups(ThreadLaunchExecDesc& ro_Desc, Flag v_Permission);
	void groupID(ThreadLaunchExecDesc& ro_Desc, Dword v_GroupID);

	void affinityMask(ThreadLaunchExecDesc& ro_Desc, ProcessorIdx v_Mask);

	template<size_t N>
	void affinityMaskFromFlags(ThreadLaunchExecDesc& ro_Desc, std::array<Flag, N> v_Cores [[maybe_unused]]) {
		ProcessorIdx mask = 0;
		for (size_t i = 0; i < N; i++) if (v_Cores[i]) mask |= (static_cast<ProcessorIdx>(1) << i);
		affinityMask(ro_Desc, mask);
	}

	template<size_t N>
	void affinityMaskFromCores(ThreadLaunchExecDesc& ro_Desc, std::array<Dword, N> v_Cores [[maybe_unused]] ) {
		ProcessorIdx mask = 0;
		for (auto core : v_Cores) mask |= (static_cast<ProcessorIdx>(1) << core);
		affinityMask(ro_Desc, mask);
	}

	void priorityBoosting(ThreadLaunchExecDesc& ro_Desc, Flag v_Permission);
	void basePriority(ThreadLaunchExecDesc& ro_Desc, Priority v_BasePriority);

	bool validateLaunchExecDesc(const ThreadLaunchExecDesc& ro_Desc);

	struct RUNTIME ParkHandle final {
		
	};
}
