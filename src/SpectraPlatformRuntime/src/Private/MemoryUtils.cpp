#include "SpectraPlatformRuntime.h"
#include "MemoryUtils.h"

namespace Spectra::Platform::Runtime::Memory {

	bool isValid(const VirtualMemoryHandle& ro_Handle) noexcept {
		return ro_Handle.m_BaseAddress != nullptr && ro_Handle.m_TotalSize > 0;
	}

	void initMemoryDesc(VirtualMemoryDesc& ro_Desc) noexcept {
		ro_Desc.m_TargetAddress = nullptr;
		ro_Desc.m_Size = 0;
		ro_Desc.m_NumaNode = 0;
		ro_Desc.m_State = MemoryState::UNINITIALIZED;
		ro_Desc.m_Protect = MemoryProtect::NO_ACCESS;
		ro_Desc.m_Flags = MemoryFlags::NONE;
	}

	void setTargetAddress(VirtualMemoryDesc& ro_Desc, void* p_Target) noexcept {
		ro_Desc.m_TargetAddress = p_Target;
	}

	void setSize(VirtualMemoryDesc& ro_Desc, size_t size) noexcept {
		ro_Desc.m_Size = size;
	}

	void setNumaNode(VirtualMemoryDesc& ro_Desc, uint32_t v_Node) noexcept {
		ro_Desc.m_NumaNode = v_Node;
	}

	void setMemoryState(VirtualMemoryDesc& ro_Desc, MemoryState v_State) noexcept {
		ro_Desc.m_State = v_State;
	}

	void setProtection(VirtualMemoryDesc& ro_Desc, MemoryProtect v_Prot) noexcept {
		ro_Desc.m_Protect = v_Prot;
	}

	void setFlags(VirtualMemoryDesc& ro_Desc, MemoryFlags v_Flags) noexcept {
		ro_Desc.m_Flags = v_Flags;
	}

}