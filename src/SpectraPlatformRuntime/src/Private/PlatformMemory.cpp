#include "SpectraPlatformRuntime.h"
#define ALLOW_SYSCALL
#include "SpectraSyscalls.h"
#include "PlatformMemory.h"

#include "SpectraDiagnostics.h"

namespace Spectra::Platform::Runtime::Memory {
	namespace {
		PlatformMemoryCapabilities g_MemoryCapabilities{};
		PlatformMemoryInfo g_MemoryInfo{};
		bool g_IsVmInitialized = false;

		bool hasLargePagePrivilege() {
			HANDLE token = nullptr;

			if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
				return false;

			LUID luid;
			if (!LookupPrivilegeValue(nullptr, SE_LOCK_MEMORY_NAME, &luid)) {
				CloseHandle(token);
				return false;
			}

			DWORD size = 0;
			GetTokenInformation(token, TokenPrivileges, nullptr, 0, &size);

			auto* buffer = static_cast<TOKEN_PRIVILEGES*>(malloc(size));

			if (!GetTokenInformation(token, TokenPrivileges, buffer, size, &size)) {
				free(buffer);
				CloseHandle(token);
				return false;
			}

			bool enabled = false;

			for (DWORD i = 0; i < buffer->PrivilegeCount; ++i) {
				auto& p = buffer->Privileges[i];

				if (p.Luid.LowPart == luid.LowPart &&
					p.Luid.HighPart == luid.HighPart) {
					enabled = (p.Attributes & SE_PRIVILEGE_ENABLED);
					break;
				}
			}

			free(buffer);
			CloseHandle(token);

			return enabled;
		}

		uint32_t getActiveNumaNodes() {
			ULONG highestNode = 0;

			if (!GetNumaHighestNodeNumber(&highestNode)) {
				return 1; // fallback: single node system
			}

			uint32_t activeCount = 0;

			for (ULONG node = 0; node <= highestNode; ++node) {
				ULONGLONG mask = 0;

				if (GetNumaNodeProcessorMask(node, &mask) && mask != 0) {
					activeCount++;
				}
			}

			return activeCount;
		}
	}

	void PlatformVirtualMemory::init() {
		if (g_IsVmInitialized) return;

		SYSTEM_INFO info{};
		GetSystemInfo(&info);
		g_MemoryInfo.m_AllocationGranularity = info.dwAllocationGranularity;
		g_MemoryInfo.m_PageSize = info.dwPageSize;

		ULONGLONG memKB = 0;
		if (!GetPhysicallyInstalledSystemMemory(&memKB)) {
			// TODO Stuff
		}
		g_MemoryInfo.m_TotalPhysicalMemory = memKB * 1024;

		MEMORYSTATUSEX status{};
		status.dwLength = sizeof(status);

		if (!GlobalMemoryStatusEx(&status)) {
			//TODO Stuff
		}
		g_MemoryInfo.m_AvailableMemory = status.ullAvailPhys;

		SIZE_T size = GetLargePageMinimum();

		g_MemoryCapabilities.m_LargePageSize = size;
		g_MemoryCapabilities.m_SupportsLargePages = (size != 0) && hasLargePagePrivilege();
		ULONG highestNode = 0;
		if (!GetNumaHighestNodeNumber(&highestNode)) {
			g_MemoryCapabilities.m_MaxNumaNodes = 0;
			g_MemoryCapabilities.m_SupportsNumaNodes = false;
			g_MemoryCapabilities.m_ActiveNumaNodes = 1;
		} else {
			g_MemoryCapabilities.m_MaxNumaNodes = highestNode;
			g_MemoryCapabilities.m_SupportsNumaNodes = true;
			g_MemoryCapabilities.m_ActiveNumaNodes = getActiveNumaNodes();
		}
		g_IsVmInitialized = true;
	}

	const PlatformMemoryInfo& PlatformVirtualMemory::getMemoryInfo() {
		return g_MemoryInfo;
	}

	const PlatformMemoryCapabilities& PlatformVirtualMemory::getMemoryCapabilities() {
		return g_MemoryCapabilities;
	}

	size_t PlatformVirtualMemory::queryAvailableMemory() {
		MEMORYSTATUSEX status{};
		status.dwLength = sizeof(status);

		if (!GlobalMemoryStatusEx(&status)) {
			//TODO Stuff
		}

		return static_cast<size_t>(status.ullAvailPhys);
	}

	size_t PlatformVirtualMemory::alignToPage(size_t v_Size) {
		SPECTRA_ASSERT(g_IsVmInitialized);
		return (v_Size + g_MemoryInfo.m_PageSize - 1) & (~(g_MemoryInfo.m_PageSize - 1));
	}

	size_t PlatformVirtualMemory::alignToGranularity(size_t v_Size) {
		SPECTRA_ASSERT(g_IsVmInitialized);
		return (v_Size + g_MemoryInfo.m_AllocationGranularity - 1) & (~(g_MemoryInfo.m_AllocationGranularity - 1));
	}

	void* PlatformVirtualMemory::alignPointerToPage(void* p_MemoryAddr) {
		SPECTRA_ASSERT(g_IsVmInitialized);
		uintptr_t addr = reinterpret_cast<uintptr_t>(p_MemoryAddr);
		addr = (addr + g_MemoryInfo.m_PageSize - 1) & (~(g_MemoryInfo.m_PageSize - 1));
		return reinterpret_cast<void*>(addr);
	}

	void* PlatformVirtualMemory::alignPointerToGranularity(void* p_MemoryAddr) {
		SPECTRA_ASSERT(g_IsVmInitialized);
		uintptr_t addr = reinterpret_cast<uintptr_t>(p_MemoryAddr);
		addr = (addr + g_MemoryInfo.m_AllocationGranularity - 1) & (~(g_MemoryInfo.m_AllocationGranularity - 1));
		return reinterpret_cast<void*>(addr);
	}

	bool PlatformVirtualMemory::isPageAligned(size_t v_Size) {
		SPECTRA_ASSERT(g_IsVmInitialized);
		return (v_Size & (g_MemoryInfo.m_PageSize - 1)) == 0;
	}

	bool PlatformVirtualMemory::isGranularityAligned(size_t v_Size) {
		SPECTRA_ASSERT(g_IsVmInitialized);
		return (v_Size & (g_MemoryInfo.m_AllocationGranularity - 1)) == 0;
	}

	bool PlatformVirtualMemory::isPointerPageAligned(void* p_MemoryAddr) {
		SPECTRA_ASSERT(g_IsVmInitialized);
		uintptr_t addr = reinterpret_cast<uintptr_t>(p_MemoryAddr);
		return (addr & (g_MemoryInfo.m_PageSize - 1)) == 0;
	}

	bool PlatformVirtualMemory::isPointerGranularityAligned(void* p_MemoryAddr) {
		SPECTRA_ASSERT(g_IsVmInitialized);
		uintptr_t addr = reinterpret_cast<uintptr_t>(p_MemoryAddr);
		return (addr & (g_MemoryInfo.m_AllocationGranularity - 1)) == 0;
	}

	size_t PlatformVirtualMemory::getLargePageSize() {
		SPECTRA_ASSERT(g_IsVmInitialized);
		return g_MemoryCapabilities.m_LargePageSize;
	}

	bool PlatformVirtualMemory::supportsLargePages() {
		SPECTRA_ASSERT(g_IsVmInitialized);
		return g_MemoryCapabilities.m_SupportsLargePages;
	}

	uint32_t PlatformVirtualMemory::getMaxNumaNodes() {
		SPECTRA_ASSERT(g_IsVmInitialized);
		return g_MemoryCapabilities.m_MaxNumaNodes;
	}

	bool PlatformVirtualMemory::isValidNumaNode(uint32_t v_Node) {
		SPECTRA_ASSERT(g_IsVmInitialized);
		return g_MemoryCapabilities.m_SupportsNumaNodes && v_Node < g_MemoryCapabilities.m_MaxNumaNodes;
	}

	void PlatformVirtualMemory::validateAlignment(const VirtualMemoryDesc& ro_Desc) {
		SPECTRA_ASSERT(g_IsVmInitialized);
		if (ro_Desc.m_State == MemoryState::UNINITIALIZED) {
			// TODO
		}

		if (ro_Desc.m_Size == 0 && ro_Desc.m_State != MemoryState::RELEASE) {
			// TODO
		}

		switch (ro_Desc.m_State) {
		case MemoryState::RESERVE: break; // Can be anything
		case MemoryState::COMMIT:
		case MemoryState::DECOMMIT:
		case MemoryState::RELEASE:
			{
				if (ro_Desc.m_TargetAddress == nullptr) {
					//TODO
				}
			}
			break;
		case MemoryState::UNINITIALIZED:
		case MemoryState::PROTECT:
			{
				// TODO
			}
		}

		if (ro_Desc.m_NumaNode != INVALID_NUMA_NODE) {
			if (!g_MemoryCapabilities.m_SupportsNumaNodes) {
				// TODO
			}

			if (ro_Desc.m_NumaNode > g_MemoryCapabilities.m_MaxNumaNodes) {
				// TODO
			}
		}
	}

	void PlatformVirtualMemory::validateMemoryDesc(const VirtualMemoryDesc& ro_Desc) {
		SPECTRA_ASSERT(g_IsVmInitialized);
		switch (ro_Desc.m_State) {
		case MemoryState::RESERVE:
			if (ro_Desc.m_TargetAddress != nullptr &&
				!isPointerPageAligned(ro_Desc.m_TargetAddress)) {
				// TODO
			}

			if ((ro_Desc.m_Size & (g_MemoryInfo.m_AllocationGranularity - 1)) != 0) {
				// TODO
			}
			break;

		case MemoryState::COMMIT:
		case MemoryState::DECOMMIT:
			if (!isPointerPageAligned(ro_Desc.m_TargetAddress)) {
			}

			if ((ro_Desc.m_Size & (g_MemoryInfo.m_PageSize - 1)) != 0) {
				// TODO
			}
			break;

		case MemoryState::RELEASE:
			break;
		case MemoryState::UNINITIALIZED:
		case MemoryState::PROTECT:
			{
				//TODO
			}
		}
	}

	VirtualMemoryHandle PlatformVirtualMemory::reserve(const VirtualMemoryDesc& ro_Desc) {
		SPECTRA_ASSERT(g_IsVmInitialized);
		validateMemoryDesc(ro_Desc);
		validateAlignment(ro_Desc);

		if (ro_Desc.m_State != MemoryState::RESERVE) {
			// TODO
		}

		DWORD allocType = MEM_RESERVE;
		DWORD protect = PAGE_NOACCESS;

		if (ro_Desc.m_Flags == MemoryFlags::LARGE_PAGES && g_MemoryCapabilities.m_SupportsLargePages) {
			allocType |= MEM_LARGE_PAGES;
		}

		void* base = nullptr;

		if (g_MemoryCapabilities.m_SupportsNumaNodes && ro_Desc.m_NumaNode != INVALID_NUMA_NODE) {
			base = VirtualAllocExNuma(GetCurrentProcess(), ro_Desc.m_TargetAddress, ro_Desc.m_Size
									  , allocType, protect, ro_Desc.m_NumaNode);
		} else {
			base = VirtualAlloc(ro_Desc.m_TargetAddress, ro_Desc.m_Size, allocType, protect);
		}

		if (base == nullptr) {
			// TODO
		}

		VirtualMemoryHandle handle{};
		handle.m_TotalSize = ro_Desc.m_Size;
		handle.m_BaseAddress = base;
		return handle;
	}

	void PlatformVirtualMemory::commit(const VirtualMemoryHandle& ro_Handle, const VirtualMemoryDesc& ro_Desc) {
		SPECTRA_ASSERT(g_IsVmInitialized);
		validateAlignment(ro_Desc);
		validateMemoryDesc(ro_Desc);

		if (ro_Desc.m_State != MemoryState::COMMIT) {
			// TODO
		}

		if (!ro_Handle.m_BaseAddress || ro_Handle.m_TotalSize == 0) {
			// TODO
		}

		uintptr_t base = reinterpret_cast<uintptr_t>(ro_Handle.m_BaseAddress);
		uintptr_t target = reinterpret_cast<uintptr_t>(ro_Desc.m_TargetAddress);

		if (target < base || target + ro_Desc.m_Size > base + ro_Handle.m_TotalSize) {
			// TODO
		}

		DWORD protect = PAGE_READWRITE;
		switch (ro_Desc.m_Protect) {
		case MemoryProtect::NO_ACCESS:           protect = PAGE_NOACCESS; break;
		case MemoryProtect::READ_ONLY:           protect = PAGE_READONLY; break;
		case MemoryProtect::READ_WRITE:          protect = PAGE_READWRITE; break;
		case MemoryProtect::EXECUTE:             protect = PAGE_EXECUTE; break;
		case MemoryProtect::EXECUTE_READ:        protect = PAGE_EXECUTE_READ; break;
		case MemoryProtect::EXECUTE_READ_WRITE:  protect = PAGE_EXECUTE_READWRITE; break;
		case MemoryProtect::GUARD:               protect = PAGE_GUARD; break;
		default:
			{
				// TODO
			}
		}

		void* result = VirtualAlloc(
			ro_Desc.m_TargetAddress,
			ro_Desc.m_Size,
			MEM_COMMIT,
			protect
		);

		if (!result) {
			// TODO
		}

#ifdef SPECTRA_BUILD_DEBUG
		if (result != ro_Desc.m_TargetAddress) {
			// TODO
		}
#endif
	}

	void PlatformVirtualMemory::decommit(const VirtualMemoryHandle& ro_Handle, const VirtualMemoryDesc& ro_Desc) {
		SPECTRA_ASSERT(g_IsVmInitialized);
		validateAlignment(ro_Desc);
		validateMemoryDesc(ro_Desc);

		if (ro_Desc.m_State != MemoryState::DECOMMIT) {
			// TODO
		}

		if (!ro_Handle.m_BaseAddress || ro_Handle.m_TotalSize == 0) {
			// TODO
		}

		uintptr_t base = reinterpret_cast<uintptr_t>(ro_Handle.m_BaseAddress);
		uintptr_t target = reinterpret_cast<uintptr_t>(ro_Desc.m_TargetAddress);

		if (target < base || target + ro_Desc.m_Size > base + ro_Handle.m_TotalSize) {
			// TODO
		}

		BOOL ok = VirtualFree(
			ro_Desc.m_TargetAddress,
			ro_Desc.m_Size,
			MEM_DECOMMIT
		);
		if (!ok) {
			// TODO
		}
	}

	void PlatformVirtualMemory::release(VirtualMemoryHandle& ro_Handle, const VirtualMemoryDesc& ro_Desc) {
		SPECTRA_ASSERT(g_IsVmInitialized);

		validateMemoryDesc(ro_Desc);
		validateAlignment(ro_Desc);

		if (ro_Desc.m_State != MemoryState::RELEASE) {
			// TODO
		}

		if (!ro_Handle.m_BaseAddress || ro_Handle.m_TotalSize == 0) {
			// TODO
		}

		if (ro_Desc.m_TargetAddress != ro_Handle.m_BaseAddress) {
			// TODO
		}

		if (ro_Desc.m_Size != 0) {
			// TODO
		}

		BOOL ok = VirtualFree(
			ro_Handle.m_BaseAddress,
			0,
			MEM_RELEASE
		);

		if (!ok) {
			// TODO
		}

		ro_Handle.m_BaseAddress = nullptr;
		ro_Handle.m_TotalSize = 0;
	}

	void PlatformVirtualMemory::protect(const VirtualMemoryHandle& ro_Handle, const VirtualMemoryDesc& ro_Desc) {
		SPECTRA_ASSERT(g_IsVmInitialized);

		validateMemoryDesc(ro_Desc);
		validateAlignment(ro_Desc);

		if (ro_Desc.m_State != MemoryState::PROTECT) {
		}

		if (!ro_Handle.m_BaseAddress || ro_Handle.m_TotalSize == 0) {
		}

		uintptr_t base = reinterpret_cast<uintptr_t>(ro_Handle.m_BaseAddress);
		uintptr_t target = reinterpret_cast<uintptr_t>(ro_Desc.m_TargetAddress);

		if (target < base || target + ro_Desc.m_Size > base + ro_Handle.m_TotalSize) {
		}

		DWORD protect = 0;

		switch (ro_Desc.m_Protect) {
		case MemoryProtect::NO_ACCESS:           protect = PAGE_NOACCESS; break;
		case MemoryProtect::READ_ONLY:           protect = PAGE_READONLY; break;
		case MemoryProtect::READ_WRITE:          protect = PAGE_READWRITE; break;
		case MemoryProtect::EXECUTE:             protect = PAGE_EXECUTE; break;
		case MemoryProtect::EXECUTE_READ:        protect = PAGE_EXECUTE_READ; break;
		case MemoryProtect::EXECUTE_READ_WRITE:  protect = PAGE_EXECUTE_READWRITE; break;
		case MemoryProtect::GUARD:               protect = PAGE_READWRITE | PAGE_GUARD; break;
		default:
			{
				// TODO
			}
		}

		DWORD oldProtect = 0;

		BOOL ok = VirtualProtect(
			ro_Desc.m_TargetAddress,
			ro_Desc.m_Size,
			protect,
			&oldProtect
		);

		if (!ok) {
		}

#ifdef SPECTRA_BUILD_DEBUG
		// Optional: sanity check/log oldProtect if needed
#endif
	}

	PageInfo PlatformVirtualMemory::query(const MemoryQueryDesc& ro_Desc) {
		SPECTRA_ASSERT(g_IsVmInitialized);

		if (ro_Desc.m_TargetAddress == nullptr) {
			// TODO
		}

		MEMORY_BASIC_INFORMATION mbi{};

		SIZE_T result = VirtualQuery(
			ro_Desc.m_TargetAddress,
			&mbi,
			sizeof(mbi)
		);

		if (result == 0) {
			// TODO
		}

		PageInfo info{};

		// Base + size
		info.m_PageBaseAddr = mbi.BaseAddress;
		info.m_RegionSize = mbi.RegionSize;

		switch (mbi.State) {
		case MEM_FREE:
			info.m_State = MemoryState::UNINITIALIZED;
			break;

		case MEM_RESERVE:
			info.m_State = MemoryState::RESERVE;
			break;

		case MEM_COMMIT:
			info.m_State = MemoryState::COMMIT;
			break;

		default:
			{
				// TODO
			}
		}

		DWORD protect = mbi.Protect;

		if (protect & PAGE_NOACCESS) {
			info.m_Protect = MemoryProtect::NO_ACCESS;
		} else if (protect & PAGE_READONLY) {
			info.m_Protect = MemoryProtect::READ_ONLY;
		} else if (protect & PAGE_READWRITE) {
			info.m_Protect = MemoryProtect::READ_WRITE;
		} else if (protect & PAGE_EXECUTE) {
			info.m_Protect = MemoryProtect::EXECUTE;
		} else if (protect & PAGE_EXECUTE_READ) {
			info.m_Protect = MemoryProtect::EXECUTE_READ;
		} else if (protect & PAGE_EXECUTE_READWRITE) {
			info.m_Protect = MemoryProtect::EXECUTE_READ_WRITE;
		} else if (protect & PAGE_GUARD) {
			info.m_Protect = MemoryProtect::GUARD;
		} else {
			info.m_Protect = MemoryProtect::NO_ACCESS;
		}

		info.m_NumaNode = 0;

		return info;
	}
}