#include "SpectraMemory.h"
#include "MemoryRegion.h"
#include "PlatformMemory.h"
#include "ProcessEnvironment.h"

namespace Spectra::Memory {
	namespace {
		bool g_IsMemoryBackendInitialized = false;

		void checkInit() {
			if (!g_IsMemoryBackendInitialized)
				Platform::Runtime::Environment::PlatformTermination::terminate();
		}
	}

	void Memory::init() {
		if (!g_IsMemoryBackendInitialized) {
			PlatformVirtualMemory::init(); // Initialize underlying backend
			g_IsMemoryBackendInitialized = true;
		}
	}

	bool Memory::isInit() {
		return g_IsMemoryBackendInitialized;
	}

	MemoryHandle Memory::reserve(VirtualMemoryDesc& ro_Desc) {
		checkInit();
		ro_Desc.m_Size = PlatformVirtualMemory::alignToGranularity(ro_Desc.m_Size);
		MemoryHandle h{};
		h.m_Memory = PlatformVirtualMemory::reserve(ro_Desc);
		h.m_CommittedSize = 0;
		h.m_NumaNode = ro_Desc.m_NumaNode;
		return h;
	}

	void Memory::commit(MemoryHandle& ro_Handle, VirtualMemoryDesc& ro_Desc) {
		checkInit();

		ro_Desc.m_Size = PlatformVirtualMemory::alignToPage(ro_Desc.m_Size);
		PlatformVirtualMemory::commit(ro_Handle.m_Memory, ro_Desc);

		ro_Handle.m_CommittedSize += ro_Desc.m_Size;
		ro_Handle.m_NumaNode = ro_Desc.m_NumaNode;
	}

	void Memory::decommit(MemoryHandle& ro_Handle, VirtualMemoryDesc& ro_Desc) {
		checkInit();

		ro_Desc.m_Size = PlatformVirtualMemory::alignToPage(ro_Desc.m_Size);
		PlatformVirtualMemory::decommit(ro_Handle.m_Memory, ro_Desc);

		ro_Handle.m_CommittedSize -= ro_Desc.m_Size;
	}

	void Memory::release(MemoryHandle& ro_Handle, VirtualMemoryDesc& ro_Desc) {
		checkInit();

		PlatformVirtualMemory::release(ro_Handle.m_Memory, ro_Desc);

		ro_Handle.m_CommittedSize = 0;
		ro_Handle.m_NumaNode = 0;
	}

	void Memory::protectRegion(MemoryHandle& ro_Handle, VirtualMemoryDesc& ro_Desc) {
		checkInit();

		ro_Desc.m_Size = PlatformVirtualMemory::alignToPage(ro_Desc.m_Size);
		PlatformVirtualMemory::protect(ro_Handle.m_Memory, ro_Desc);
	}

	PageInfo Memory::query(const MemoryQueryDesc& ro_Desc) {
		checkInit();

		return PlatformVirtualMemory::query(ro_Desc);
	}
}
