#include "SpectraInstrumentation.h"
#include "SpecInstUtility.h"
#include "PlatformMemory.h"
#include "ProcessEnvironment.h"

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

		// +1: v_Offset itself must be covered, else a byte that starts a fresh page stays reserved.
		const size_t v_CommitSize = PlatformVirtualMemory::alignToPage(v_Offset + 1 - ro_Handle.m_CommittedSize);

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

	Region InstrumentationVACarver::carve(size_t v_Size, size_t v_Alignment) {
		using Spectra::Platform::Runtime::Environment::PlatformTermination;
		SPEC_INST_ASSERT(v_Size != 0 && v_Alignment != 0 && (v_Alignment & (v_Alignment - 1)) == 0);

		const size_t offset = (m_Watermark + v_Alignment - 1) & ~(v_Alignment - 1);
		if (offset + v_Size > m_Handle.m_Memory.m_TotalSize || offset + v_Size < offset) PlatformTermination::terminate();
		if (!commitPageIfNeeded(m_Handle, offset + v_Size - 1)) PlatformTermination::terminate();

		m_Watermark = offset + v_Size;
		return Region{ static_cast<uint8_t*>(m_Handle.m_Memory.m_BaseAddress) + offset, v_Size };
	}
}
