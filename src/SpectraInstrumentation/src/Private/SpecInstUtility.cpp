#include "SpectraInstrumentation.h"
#include "SpecInstUtility.h"
#include "PlatformMemory.h"

namespace Spectra::Instrumentation::Utils {
	Region slice(Region& ro_Region, size_t v_Len) {
		using Spectra::Platform::Runtime::Memory::PlatformVirtualMemory;

		if (!ro_Region.isValid()) return INVALID_REGION;

		const size_t v_AlignedLen = PlatformVirtualMemory::alignToPage(v_Len);
		if (v_AlignedLen > ro_Region.m_MaxSize) return INVALID_REGION;

		Region r{};
		r.m_BaseAddr =
			reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(ro_Region.m_BaseAddr)
				+ (ro_Region.m_MaxSize - v_AlignedLen));
		r.m_MaxSize = v_AlignedLen;

		ro_Region.m_MaxSize -= v_AlignedLen;
		return r;
	}

	void lockGuard(const Region& ro_Guard) {
		using namespace Spectra::Platform::Runtime::Memory;

		SPEC_INST_DEBUG_ASSERT(ro_Guard.isValid());

		VirtualMemoryHandle handle{};
		handle.m_BaseAddress = ro_Guard.m_BaseAddr;
		handle.m_TotalSize = ro_Guard.m_MaxSize;

		VirtualMemoryDesc desc{};
		initMemoryDesc(desc);
		setTargetAddress(desc, ro_Guard.m_BaseAddr);
		setSize(desc, ro_Guard.m_MaxSize);
		setMemoryState(desc, MemoryState::PROTECT);
		setProtection(desc, MemoryProtect::NO_ACCESS);

		PlatformVirtualMemory::protect(handle, desc);
	}

	bool commitPageIfNeeded(RegionHandle& ro_Handle, size_t v_Offset) {
		using namespace Spectra::Platform::Runtime::Memory;

		if (v_Offset >= ro_Handle.m_Memory.m_TotalSize) return false;
		if (v_Offset < ro_Handle.m_CommittedSize) return true; // already covered, skip the query syscall

		MemoryQueryDesc queryDesc{};
		queryDesc.m_TargetAddress = static_cast<uint8_t*>(ro_Handle.m_Memory.m_BaseAddress) + v_Offset;
		const PageInfo info = PlatformVirtualMemory::query(queryDesc);

		if (info.m_State == MemoryState::COMMIT) return true;
		if (info.m_State != MemoryState::RESERVE) return false;

		const size_t v_CommitSize = PlatformVirtualMemory::alignToPage(v_Offset - ro_Handle.m_CommittedSize);

		VirtualMemoryDesc commitDesc{};
		initMemoryDesc(commitDesc);
		setTargetAddress(commitDesc, static_cast<uint8_t*>(ro_Handle.m_Memory.m_BaseAddress) + ro_Handle.m_CommittedSize);
		setSize(commitDesc, v_CommitSize);
		setNumaNode(commitDesc, INVALID_NUMA_NODE);
		setMemoryState(commitDesc, MemoryState::COMMIT);
		setProtection(commitDesc, MemoryProtect::READ_WRITE);

		PlatformVirtualMemory::commit(ro_Handle.m_Memory, commitDesc);

		ro_Handle.m_CommittedSize += v_CommitSize;
		return true;
	}

	RegionHandle createHandle(Region& ro_Region) {
		return RegionHandle{ ro_Region };
	}

	void* InstrumentationAllocator::allocate(size_t v_Bytes) {
		SPEC_INST_ASSERT(m_Cursor + v_Bytes <= m_Handle.m_Memory.m_TotalSize);

		const size_t v_Offset = m_Cursor;
		const bool v_Committed = commitPageIfNeeded(m_Handle, v_Offset + v_Bytes - 1);
		SPEC_INST_ASSERT(v_Committed);

		auto* p_Ptr = static_cast<uint8_t*>(m_Handle.m_Memory.m_BaseAddress) + v_Offset;
		m_Cursor += v_Bytes;
		return p_Ptr;
	}
}
