#pragma once
#include "SpectraMemory.h"
#include "MemoryUtils.h"

namespace Spectra::Memory {
	using namespace Platform::Runtime::Memory;

	struct SPEC_MEM_RUNTIME_API MemoryHandle final {
		VirtualMemoryHandle m_Memory;
		size_t m_CommittedSize;
		size_t m_NumaNode;
	};

	// Raw VA allocation: reserve/commit/decommit/release/protect/query on a VirtualMemoryHandle,
	// a thin 1:1 wrapper over PlatformVirtualMemory.
	class SPEC_MEM_RUNTIME_API Memory final {
	public:
		static void init();
		static bool isInit();

		static MemoryHandle reserve(VirtualMemoryDesc& ro_Desc);
		static void commit(MemoryHandle& ro_Handle, VirtualMemoryDesc& ro_Desc);
		static void decommit(MemoryHandle& ro_Handle, VirtualMemoryDesc& ro_Desc);
		static void release(MemoryHandle& ro_Handle, VirtualMemoryDesc& ro_Desc);
		static void protectRegion(MemoryHandle& ro_Handle, VirtualMemoryDesc& ro_Desc);
		static PageInfo query(const MemoryQueryDesc& ro_Desc);
	};
}
