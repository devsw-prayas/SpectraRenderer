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
}
