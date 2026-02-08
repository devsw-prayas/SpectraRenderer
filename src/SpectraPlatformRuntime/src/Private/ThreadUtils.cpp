#include "SpectraPlatformRuntime.h"
#include "ThreadUtils.h"

namespace Spectra::Platform::Runtime::Thread {
	ThreadState ThreadHandle::expectedState() const {
		return this->m_State;
	}

	size_t ThreadHandle::getThreadID() const {
		return this->m_ThreadID;
	}


	void ThreadLaunchDesc::launchFunction(Entry p_Launch, void* p_Ctx) {
		this->m_StartEntry = p_Launch;
		this->m_StartContext = p_Ctx;
	}

	void ThreadLaunchDesc::startupHook(Entry p_StartupHook, void* p_Ctx) {
		this->m_StartupEntry = p_StartupHook;
		this->m_StartupContext = p_Ctx;
	}

	void ThreadLaunchDesc::shutdownHook(Entry p_ShutdownHook, void* p_Ctx) {
		this->m_ShutdownEntry = p_ShutdownHook;
		this->m_ShutdownContext = p_Ctx;
	}

	Flag ThreadLaunchDesc::preSuspend(Flag v_SuspendState) {
		Flag temp = m_StartsSuspended;
		m_StartsSuspended = v_SuspendState;
		return temp;
	}

	// ------ ThreadLaunchExecDesc -------- (It's confusing to say the least, okay!)

	void SPECTRA_RUNTIME_API initLaunchExecDesc(ThreadLaunchExecDesc& ro_Desc) {
		ro_Desc.m_AffinityMask = 0;
		ro_Desc.m_BasePriority = Priority::PRIORITY_NORMAL;
		ro_Desc.m_CommitSize = 0;
		ro_Desc.m_GroupID = 0;
		ro_Desc.m_GuardsEnabled = Allow;
		ro_Desc.m_IdealProcessor = 0;
		ro_Desc.m_PriorityBoost = Disallow;
		ro_Desc.m_ReserveSize = 0;
		ro_Desc.m_SupportsIdealProcessor = Allow;
		ro_Desc.m_SupportsThreadGroup = Allow;
	}

	void SPECTRA_RUNTIME_API commitStackSize(ThreadLaunchExecDesc& ro_Desc, Bytes v_Size) {
		ro_Desc.m_CommitSize = v_Size;
		ro_Desc.m_ReserveSize = 0;
	}

	void SPECTRA_RUNTIME_API reserveStackSize(ThreadLaunchExecDesc& ro_Desc, Bytes v_Size) {
		ro_Desc.m_ReserveSize = v_Size;
		ro_Desc.m_CommitSize = 0;
	}

	void SPECTRA_RUNTIME_API enableGuardPage(ThreadLaunchExecDesc& ro_Desc, Flag v_Permission) {
		ro_Desc.m_GuardsEnabled = v_Permission;
	}

	void SPECTRA_RUNTIME_API supportIdealProcessor(ThreadLaunchExecDesc& ro_Desc, Flag v_Permission) {
		ro_Desc.m_SupportsIdealProcessor = v_Permission;
	}

	void SPECTRA_RUNTIME_API idealProcessor(ThreadLaunchExecDesc& ro_Desc, Dword v_Processor) {
		ro_Desc.m_IdealProcessor = v_Processor;
	}

	void SPECTRA_RUNTIME_API supportProcessorGroups(ThreadLaunchExecDesc& ro_Desc, Flag v_Permission) {
		ro_Desc.m_SupportsThreadGroup = v_Permission;
	}

	void SPECTRA_RUNTIME_API groupID(ThreadLaunchExecDesc& ro_Desc, Dword v_GroupID) {
		ro_Desc.m_GroupID = v_GroupID;
	}

	void SPECTRA_RUNTIME_API affinityMask(ThreadLaunchExecDesc& ro_Desc, ProcessorIdx v_Mask) {
		ro_Desc.m_AffinityMask = v_Mask;
	}

	void SPECTRA_RUNTIME_API priorityBoosting(ThreadLaunchExecDesc& ro_Desc, Flag v_Permission) {
		ro_Desc.m_PriorityBoost = v_Permission;
	}

	void SPECTRA_RUNTIME_API basePriority(ThreadLaunchExecDesc& ro_Desc, Priority v_BasePriority) {
		ro_Desc.m_BasePriority = v_BasePriority;
	}

	bool SPECTRA_RUNTIME_API validateLaunchExecDesc(const ThreadLaunchExecDesc& ro_Desc) {
		if ((ro_Desc.m_CommitSize == 0) == (ro_Desc.m_ReserveSize == 0)) return false;
		if (ro_Desc.m_IdealProcessor && !ro_Desc.m_SupportsIdealProcessor) return  false;
		if (ro_Desc.m_GroupID && !ro_Desc.m_SupportsThreadGroup) return false;
		if (ro_Desc.m_BasePriority == Priority::PRIORITY_TIME_CRITICAL 
			&& ro_Desc.m_PriorityBoost) return false;
		if (!ro_Desc.m_AffinityMask) return false;
		if (ro_Desc.m_IdealProcessor)
			if (!(ro_Desc.m_AffinityMask & 
				(static_cast<ProcessorIdx>(1) << ro_Desc.m_IdealProcessor))) return false;
		if (ro_Desc.m_GuardsEnabled && ro_Desc.m_CommitSize) return false;
		return true;
	}
}
