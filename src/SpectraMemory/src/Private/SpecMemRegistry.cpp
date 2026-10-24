#include "SpectraMemory.h"
#include "SpecMemRegistry.h"
#include "SpecMemDiagnostics.h"
#include "PlatformMemory.h"

namespace Spectra::Memory {
	using Platform::Runtime::Intrinsic::MemoryOrder;

	Allocators::BlockedAllocator AllocatorRegistry::s_fileSystemBlocks[Internal::MAX_NUMA_NODES];
	Allocators::BlockedAllocator AllocatorRegistry::s_stagingBlocks[Internal::MAX_NUMA_NODES];
	Allocators::BlockedAllocator AllocatorRegistry::s_viewBlocks[Internal::MAX_NUMA_NODES];
	size_t AllocatorRegistry::s_nodeCount = 0;

	void AllocatorRegistry::init() {
		SPEC_MEM_ASSERT(s_nodeCount == 0);
		for (size_t node = 0; node < Internal::MAX_NUMA_NODES; ++node) {
			if (!Internal::g_FileSystemRegion[node].isValid()) break;
			MemoryHandle handle = segmentFromRegion(Internal::g_FileSystemRegion[node]);
			handle.m_NumaNode = node;
			s_fileSystemBlocks[node].init(&handle);

			MemoryHandle staging = segmentFromRegion(Internal::g_FileStagingRegion[node]);
			staging.m_NumaNode = node;
			s_stagingBlocks[node].init(&staging);

			if (Internal::g_MappedViewIsPlaceholder[node]) {
				MemoryHandle views = segmentFromRegion(Internal::g_MappedViewRegion[node]);
				views.m_NumaNode = node;
				s_viewBlocks[node].init(&views);
				s_viewBlocks[node].setPlaceholder(true);
				// One placeholder per block, so any free run can be coalesced exactly.
				uint8_t* base = Internal::g_MappedViewRegion[node].m_Base;
				for (size_t offset = 0; offset + Allocators::BLOCK_SIZE < Internal::g_MappedViewRegion[node].m_Size; offset += Allocators::BLOCK_SIZE) {
					const bool split = Platform::Runtime::Memory::PlatformPlaceholders::split(base + offset, Allocators::BLOCK_SIZE);
					SPEC_MEM_ASSERT(split);
				}
			}
			s_nodeCount = node + 1;
		}
		SPEC_MEM_ASSERT(s_nodeCount > 0);
		(void)AtomicAllocators::instance();
	}

	bool AllocatorRegistry::isInit() {
		return s_nodeCount != 0;
	}

	size_t AllocatorRegistry::nodeCount() {
		return s_nodeCount;
	}

	AtomicAllocators::AtomicAllocators() {
		for (size_t node = 0; node < AllocatorRegistry::s_nodeCount; ++node)
		{
			s_FileSystemBlocks[node].store(reinterpret_cast<uintptr_t>(&AllocatorRegistry::s_fileSystemBlocks[node]), MemoryOrder::RELEASE);
			s_StagingBlocks[node].store(reinterpret_cast<uintptr_t>(&AllocatorRegistry::s_stagingBlocks[node]), MemoryOrder::RELEASE);
			const bool views = Internal::g_MappedViewIsPlaceholder[node];
			s_ViewBlocks[node].store(views ? reinterpret_cast<uintptr_t>(&AllocatorRegistry::s_viewBlocks[node]) : 0, MemoryOrder::RELEASE);
		}
	}

	AtomicAllocators& AtomicAllocators::instance() {
		SPEC_MEM_ASSERT(AllocatorRegistry::isInit());
		static AtomicAllocators s_instance;
		return s_instance;
	}

	size_t AtomicAllocators::clampNode(size_t v_Node) {
		return v_Node < AllocatorRegistry::s_nodeCount ? v_Node : 0;
	}

	Allocators::BlockedAllocator* AtomicAllocators::fileSystemBlocks(size_t v_Node) {
		return reinterpret_cast<Allocators::BlockedAllocator*>(s_FileSystemBlocks[clampNode(v_Node)].load(MemoryOrder::ACQUIRE));
	}

	Allocators::BlockedAllocator* AtomicAllocators::stagingBlocks(size_t v_Node) {
		return reinterpret_cast<Allocators::BlockedAllocator*>(s_StagingBlocks[clampNode(v_Node)].load(MemoryOrder::ACQUIRE));
	}

	Allocators::BlockedAllocator* AtomicAllocators::viewBlocks(size_t v_Node) {
		return reinterpret_cast<Allocators::BlockedAllocator*>(s_ViewBlocks[clampNode(v_Node)].load(MemoryOrder::ACQUIRE));
	}
}
