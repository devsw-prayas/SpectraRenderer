#pragma once
#include "SpectraPlatformRuntime.h"
#include "MemoryUtils.h"

namespace Spectra::Platform::Runtime::Memory {
	class SPECTRA_RUNTIME_API PlatformVirtualMemory final {
	public:
		static void init();
		static const PlatformMemoryInfo& getMemoryInfo();
		static const PlatformMemoryCapabilities& getMemoryCapabilities();
		static size_t queryAvailableMemory();

		static size_t alignToPage(size_t v_Size);
		static size_t alignToGranularity(size_t v_Size);
		static bool isPageAligned(size_t v_Size);
		static bool isGranularityAligned(size_t v_Size);

		static void* alignPointerToPage(void* p_MemoryAddr);
		static void* alignPointerToGranularity(void* p_MemoryAddr);
		static bool isPointerPageAligned(void* p_MemoryAddr);
		static bool isPointerGranularityAligned(void* p_MemoryAddr);

		static void validateMemoryDesc(const VirtualMemoryDesc& ro_Desc);
		static void validateAlignment(const VirtualMemoryDesc& ro_Desc);
		
		static size_t getLargePageSize();
		static bool supportsLargePages();

		static uint32_t getMaxNumaNodes();
		static bool isValidNumaNode(uint32_t v_Node);

		static VirtualMemoryHandle reserve(const VirtualMemoryDesc& v_Desc);
		static void commit(const VirtualMemoryHandle& ro_Handle, const VirtualMemoryDesc& ro_Desc);
		static void decommit(const VirtualMemoryHandle& ro_Handle, const VirtualMemoryDesc& ro_Desc);
		static void release(VirtualMemoryHandle& ro_Handle, const VirtualMemoryDesc& ro_Desc);

		static void protect(const VirtualMemoryHandle& ro_Handle, const VirtualMemoryDesc& ro_Desc);
		static PageInfo query(const MemoryQueryDesc& ro_Desc);
	};
}
