#include "SpectraMemory.h"
#include "VAUtils.h"
#include "PlatformMemory.h"

namespace Spectra::Memory {
	VARegion VARegionSlicer::slice(Bytes v_RequestedSize) noexcept {
		Bytes size = PlatformVirtualMemory::alignToPage(v_RequestedSize);

		SPEC_MEM_DEBUG_ASSERT(m_Cursor + size <= m_End);

		VARegion region{ m_Cursor, size };

		SPEC_MEM_DEBUG_ASSERT(PlatformVirtualMemory::isPointerPageAligned(region.m_Base));
		SPEC_MEM_DEBUG_ASSERT(PlatformVirtualMemory::isPageAligned(region.m_Size));

		m_Cursor += size;
		return region;
	}

	MemoryHandle segmentFromRegion(const VARegion& ro_Region) noexcept {
		SPEC_MEM_DEBUG_ASSERT(ro_Region.isValid());

		MemoryHandle handle;
		handle.m_Memory.m_BaseAddress = ro_Region.m_Base;
		handle.m_Memory.m_TotalSize = ro_Region.m_Size;
		handle.m_CommittedSize = 0;
		handle.m_NumaNode = 0;
		return handle;
	}

	void lockGuard(const VARegion& ro_Guard) noexcept {
		MemoryHandle handle = segmentFromRegion(ro_Guard);

		VirtualMemoryDesc desc{};
		initMemoryDesc(desc);
		setTargetAddress(desc, ro_Guard.m_Base);
		setSize(desc, ro_Guard.m_Size);
		setMemoryState(desc, MemoryState::PROTECT);
		setProtection(desc, MemoryProtect::NO_ACCESS);

		Memory::protectRegion(handle, desc);
	}

	bool commitPageIfNeeded(MemoryHandle& ro_Handle, size_t v_Offset) noexcept {
		if (v_Offset >= ro_Handle.m_Memory.m_TotalSize) return false;
		if (v_Offset < ro_Handle.m_CommittedSize) return true; // already covered, skip the query syscall

		MemoryQueryDesc queryDesc{};
		queryDesc.m_TargetAddress = static_cast<uint8_t*>(ro_Handle.m_Memory.m_BaseAddress) + v_Offset;
		const PageInfo info = Memory::query(queryDesc);

		if (info.m_State == MemoryState::COMMIT) return true;
		if (info.m_State != MemoryState::RESERVE) return false;

		VirtualMemoryDesc commitDesc{};
		initMemoryDesc(commitDesc);
		setTargetAddress(commitDesc, static_cast<uint8_t*>(ro_Handle.m_Memory.m_BaseAddress) + ro_Handle.m_CommittedSize);
		setSize(commitDesc, v_Offset - ro_Handle.m_CommittedSize);
		setNumaNode(commitDesc, static_cast<uint32_t>(ro_Handle.m_NumaNode));
		setMemoryState(commitDesc, MemoryState::COMMIT);
		setProtection(commitDesc, MemoryProtect::READ_WRITE);
		Memory::commit(ro_Handle, commitDesc);
		return true;
	}
}
