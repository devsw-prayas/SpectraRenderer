#pragma once
#include "SpectraMemory.h"
#include "SpecMemCompiler.h"
#include "SpecMemAddrSpace.h"
#include "SpectraAllocators.h"
#include "PlatformAtomics.h"

namespace Spectra::Memory {
	// Owns the region-bound allocators, one per NUMA node. Init after HeapRegistry.
	class SPEC_MEM_RUNTIME_API AllocatorRegistry final {
		static Allocators::BlockedAllocator s_fileSystemBlocks[Internal::MAX_NUMA_NODES];
		static Allocators::BlockedAllocator s_stagingBlocks[Internal::MAX_NUMA_NODES];
		static Allocators::BlockedAllocator s_viewBlocks[Internal::MAX_NUMA_NODES];
		static size_t s_nodeCount;

		friend struct AtomicAllocators;

	public:
		static void init();
		SPEC_MEM_NODISCARD static bool isInit();
		SPEC_MEM_NODISCARD static size_t nodeCount();
	};

	// What subsystems reach for. Nodes past nodeCount clamp to 0.
	struct SPEC_MEM_RUNTIME_API AtomicAllocators final {
		Platform::Runtime::Atomic::AtomicPointer<Allocators::BlockedAllocator> s_FileSystemBlocks[Internal::MAX_NUMA_NODES];
		Platform::Runtime::Atomic::AtomicPointer<Allocators::BlockedAllocator> s_StagingBlocks[Internal::MAX_NUMA_NODES];
		Platform::Runtime::Atomic::AtomicPointer<Allocators::BlockedAllocator> s_ViewBlocks[Internal::MAX_NUMA_NODES];

		AtomicAllocators(const AtomicAllocators&) = delete;
		AtomicAllocators& operator=(const AtomicAllocators&) = delete;

		// Valid only after AllocatorRegistry::init().
		SPEC_MEM_NODISCARD static AtomicAllocators& instance();
		SPEC_MEM_NODISCARD static size_t clampNode(size_t v_Node);

		SPEC_MEM_NODISCARD Allocators::BlockedAllocator* fileSystemBlocks(size_t v_Node);
		SPEC_MEM_NODISCARD Allocators::BlockedAllocator* stagingBlocks(size_t v_Node);
		// Placeholder blocks for mapped views; nullptr when the OS has no placeholder support.
		// Every free block is its own placeholder.
		SPEC_MEM_NODISCARD Allocators::BlockedAllocator* viewBlocks(size_t v_Node);

	private:
		AtomicAllocators();
	};
}
