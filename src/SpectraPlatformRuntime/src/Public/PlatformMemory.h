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

	// Placeholder reservations: address ranges a file view can later be mapped into at a chosen address.
	// Windows 10 1803+. Resolved at runtime, so there is no extra import library. Failures return false / nullptr.
	class SPECTRA_RUNTIME_API PlatformPlaceholders final {
	public:
		SPECTRA_NODISCARD static bool isSupported();

		SPECTRA_NODISCARD static void* reserve(size_t v_Size, uint32_t v_NumaNode);
		// Carves [p_Base, p_Base + v_Size) out of the placeholder that contains it.
		SPECTRA_NODISCARD static bool split(void* p_Base, size_t v_Size);
		// Merges the adjacent placeholders that exactly cover the range.
		SPECTRA_NODISCARD static bool coalesce(void* p_Base, size_t v_Size);
		// Turns a placeholder into an ordinary reservation at the same address.
		SPECTRA_NODISCARD static bool replaceWithReserve(void* p_Base, size_t v_Size, uint32_t v_NumaNode);

		// Maps a section view into a placeholder of exactly v_Size bytes.
		SPECTRA_NODISCARD static bool mapView(void* p_Section, uint64_t v_Offset, void* p_Base, size_t v_Size, bool v_Writable, uint32_t v_NumaNode);
		// Unmaps and leaves the range as a placeholder again.
		SPECTRA_NODISCARD static bool unmapView(void* p_Base);

		SPECTRA_NODISCARD static bool flushView(const void* p_Base, size_t v_Size);
		SPECTRA_NODISCARD static bool prefetch(const void* p_Base, size_t v_Size);
	};
}
