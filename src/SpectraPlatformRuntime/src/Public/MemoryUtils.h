#pragma once
#include "SpectraPlatformRuntime.h"

namespace Spectra::Platform::Runtime::Memory {

	constexpr uint32_t INVALID_NUMA_NODE = UINT32_MAX;

	enum class SPECTRA_RUNTIME_API MemoryState final : uint8_t {
		UNINITIALIZED, RESERVE, COMMIT, DECOMMIT, RELEASE, PROTECT
	};

	enum class SPECTRA_RUNTIME_API MemoryProtect final : uint8_t {
		NO_ACCESS, READ_ONLY, READ_WRITE, EXECUTE, EXECUTE_READ, EXECUTE_READ_WRITE, GUARD
	};

	enum class SPECTRA_RUNTIME_API MemoryFlags final : uint8_t {
		NONE, LARGE_PAGES
	};

	struct alignas(16) SPECTRA_RUNTIME_API VirtualMemoryHandle final {
		void* m_BaseAddress;
		size_t m_TotalSize;

		VirtualMemoryHandle() = default;
		~VirtualMemoryHandle() = default;

		VirtualMemoryHandle(const VirtualMemoryHandle&) = default;
		VirtualMemoryHandle& operator=(const VirtualMemoryHandle&) = default;

		VirtualMemoryHandle(VirtualMemoryHandle&&) noexcept = default;
		VirtualMemoryHandle& operator=(VirtualMemoryHandle&&) noexcept = default;
	};

	constexpr VirtualMemoryHandle INVALID_HANDLE{};

	bool SPECTRA_RUNTIME_API isValid(const VirtualMemoryHandle& ro_Handle) noexcept;

	struct alignas(32) SPECTRA_RUNTIME_API VirtualMemoryDesc final {
		void* m_TargetAddress;
		size_t m_Size;
		uint32_t m_NumaNode;
		MemoryState m_State;
		MemoryProtect m_Protect;
		MemoryFlags m_Flags;

		VirtualMemoryDesc() = default;
		~VirtualMemoryDesc() = default;

		VirtualMemoryDesc(const VirtualMemoryDesc&) = default;
		VirtualMemoryDesc& operator=(const VirtualMemoryDesc&) = default;

		VirtualMemoryDesc(VirtualMemoryDesc&&) noexcept = default;
		VirtualMemoryDesc& operator=(VirtualMemoryDesc&&) noexcept = default;
	};

	void SPECTRA_RUNTIME_API initMemoryDesc(VirtualMemoryDesc& ro_Desc) noexcept;
	void SPECTRA_RUNTIME_API setTargetAddress(VirtualMemoryDesc& ro_Desc, void* p_Target) noexcept;
	void SPECTRA_RUNTIME_API setSize(VirtualMemoryDesc& ro_Desc, size_t size) noexcept;
	void SPECTRA_RUNTIME_API setNumaNode(VirtualMemoryDesc& ro_Desc, uint32_t v_Node) noexcept;
	void SPECTRA_RUNTIME_API setMemoryState(VirtualMemoryDesc& ro_Desc, MemoryState v_State) noexcept;
	void SPECTRA_RUNTIME_API setProtection(VirtualMemoryDesc& ro_Desc, MemoryProtect v_Prot) noexcept;
	void SPECTRA_RUNTIME_API setFlags(VirtualMemoryDesc& ro_Desc, MemoryFlags v_Flags) noexcept;

	struct alignas(32) PageInfo final {
		void* m_PageBaseAddr;
		size_t m_RegionSize;
		uint32_t m_NumaNode;
		MemoryState m_State;
		MemoryProtect m_Protect;

		PageInfo() = default;
		~PageInfo() = default;

		PageInfo(const PageInfo&) = default;
		PageInfo& operator=(const PageInfo&) = default;

		PageInfo(PageInfo&&) noexcept = default;
		PageInfo& operator=(PageInfo&&) noexcept = default;
	};

	struct alignas(32) PlatformMemoryInfo final {
		size_t m_PageSize;
		size_t m_AllocationGranularity;
		size_t m_TotalPhysicalMemory;
		size_t m_AvailableMemory;

		PlatformMemoryInfo() = default;
		~PlatformMemoryInfo() = default;

		PlatformMemoryInfo(const PlatformMemoryInfo&) = default;
		PlatformMemoryInfo& operator=(const PlatformMemoryInfo&) = default;

		PlatformMemoryInfo(PlatformMemoryInfo&&) noexcept = default;
		PlatformMemoryInfo& operator=(PlatformMemoryInfo&&) noexcept = default;
	};

	struct alignas(32) PlatformMemoryCapabilities final {
		size_t m_LargePageSize;
		uint32_t m_MaxNumaNodes;
		uint32_t m_ActiveNumaNodes;
		bool m_SupportsLargePages;
		bool m_SupportsNumaNodes;

		PlatformMemoryCapabilities() = default;
		~PlatformMemoryCapabilities() = default;

		PlatformMemoryCapabilities(const PlatformMemoryCapabilities&) = default;
		PlatformMemoryCapabilities& operator=(const PlatformMemoryCapabilities&) = default;

		PlatformMemoryCapabilities(PlatformMemoryCapabilities&&) noexcept = default;
		PlatformMemoryCapabilities& operator=(PlatformMemoryCapabilities&&) noexcept = default;
	};

	struct alignas(8) MemoryQueryDesc final {
		void* m_TargetAddress;

		MemoryQueryDesc() = default;
		~MemoryQueryDesc() = default;

		MemoryQueryDesc(const MemoryQueryDesc&) = default;
		MemoryQueryDesc& operator=(const MemoryQueryDesc&) = default;

		MemoryQueryDesc(MemoryQueryDesc&&) noexcept = default;
		MemoryQueryDesc& operator=(MemoryQueryDesc&&) noexcept = default;
	};
}
