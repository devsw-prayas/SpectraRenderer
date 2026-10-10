#include "SpectraPlatformRuntime.h"
#include "PlatformThreadUtils.h"

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

	// ------ ThreadAttrDesc --------

	void initAttrDesc(ThreadAttrDesc& ro_Desc) {
		ro_Desc.m_Desc.m_AffMask = 0;
		ro_Desc.m_BasePriority = Priority::PRIORITY_NORMAL;
		ro_Desc.m_CommitSize = 0;
		ro_Desc.m_Desc.m_GroupId = 0;
		ro_Desc.m_GuardsEnabled = Allow;
		ro_Desc.m_IdealProcessor = 0;
		ro_Desc.m_PriorityBoost = Disallow;
		ro_Desc.m_ReserveSize = 0;
		ro_Desc.m_SupportsIdealProcessor = Allow;
		ro_Desc.m_SupportsThreadGroup = Allow;
		ro_Desc.m_CanDetach = Disallow;
	}

	void commitStackSize(ThreadAttrDesc& ro_Desc, Bytes v_Size) {
		ro_Desc.m_CommitSize = v_Size;
		ro_Desc.m_ReserveSize = 0;
	}

	void reserveStackSize(ThreadAttrDesc& ro_Desc, Bytes v_Size) {
		ro_Desc.m_ReserveSize = v_Size;
		ro_Desc.m_CommitSize = 0;
	}

	void enableGuardPage(ThreadAttrDesc& ro_Desc, Flag v_Permission) {
		ro_Desc.m_GuardsEnabled = v_Permission;
	}

	void supportIdealProcessor(ThreadAttrDesc& ro_Desc, Flag v_Permission) {
		ro_Desc.m_SupportsIdealProcessor = v_Permission;
	}

	void idealProcessor(ThreadAttrDesc& ro_Desc, Dword v_Processor) {
		ro_Desc.m_IdealProcessor = v_Processor;
	}

	void supportProcessorGroups(ThreadAttrDesc& ro_Desc, Flag v_Permission) {
		ro_Desc.m_SupportsThreadGroup = v_Permission;
	}

	void groupID(ThreadAttrDesc& ro_Desc, Dword v_GroupID) {
		ro_Desc.m_Desc.m_GroupId = v_GroupID;
	}

	void affinityMask(ThreadAttrDesc& ro_Desc, ProcessorIdx v_Mask) {
		ro_Desc.m_Desc.m_AffMask = v_Mask;
	}

	void priorityBoosting(ThreadAttrDesc& ro_Desc, Flag v_Permission) {
		ro_Desc.m_PriorityBoost = v_Permission;
	}

	void basePriority(ThreadAttrDesc& ro_Desc, Priority v_BasePriority) {
		ro_Desc.m_BasePriority = v_BasePriority;
	}

	void	allowDetachable(ThreadAttrDesc& ro_Desc, Flag v_Permission) {
		ro_Desc.m_CanDetach = v_Permission;
	}

	bool validateAttrDesc(const ThreadAttrDesc& ro_Desc) {
		if (ro_Desc.m_CommitSize != 0 && ro_Desc.m_ReserveSize != 0) return false;
		if (ro_Desc.m_IdealProcessor && !ro_Desc.m_SupportsIdealProcessor) return  false;
		if (ro_Desc.m_Desc.m_GroupId && !ro_Desc.m_SupportsThreadGroup) return false;
		if (ro_Desc.m_BasePriority == Priority::PRIORITY_TIME_CRITICAL
			&& ro_Desc.m_PriorityBoost) return false;
		if (!ro_Desc.m_Desc.m_AffMask) return false;
		if (ro_Desc.m_IdealProcessor)
			if (!(ro_Desc.m_Desc.m_AffMask &
				(static_cast<ProcessorIdx>(1) << ro_Desc.m_IdealProcessor))) return false;
		if (ro_Desc.m_GuardsEnabled && ro_Desc.m_CommitSize) return false;
		return true;
	}

	void setAffinityMask(AffinityDesc& ro_Desc, Mask v_Mask) noexcept {
		ro_Desc.m_AffMask = v_Mask;
	}

	void setGroupId(AffinityDesc& ro_Desc, Dword v_GroupId) noexcept {
		ro_Desc.m_GroupId = v_GroupId;
	}
}