#pragma once
#include "SpectraMemory.h"
#include "VAUtils.h"
#include "MemoryRegion.h"

namespace Spectra::Memory::Internal {
	using namespace Spectra::Memory::Literals;

	// SPECTRA VIRTUAL ADDRESS SPACE — 256 GiB per NUMA node, up to 4 nodes (1 TiB reference
	// total at 4 nodes). Each node owns an independent VA reservation via VirtualAllocExNuma,
	// so physical pages committed later stay node-local. Layout is identical per node. Every
	// region below is documented immediately above the extern(s) that back it, in on-disk
	// (low -> high address) order — the box IS the layout doc, there is no separate diagram
	// to keep in sync.

	// -------------------------------------------------------------------------
	// NUMA topology constants
	// -------------------------------------------------------------------------

	constexpr size_t MAX_NUMA_NODES = SPECTRA_VA_MAX_NUMA_NODES;
	constexpr size_t g_TotalVA = SPECTRA_VA_PER_NODE_GIB * 1_GiB; // 256 GiB per node

	// -------------------------------------------------------------------------
	// Guard and section sizes
	// -------------------------------------------------------------------------

	constexpr size_t NullGuardSize = 2_MiB;
	constexpr size_t SectionGuardSize = 2_MiB;

	// -------------------------------------------------------------------------
	// Per-node region sizes — locked, see SpectraMemory_VA_Layout_Addendum.md §3
	// -------------------------------------------------------------------------

	constexpr size_t TlsfHeapSize = 24_GiB;
	constexpr size_t ScratchBufferSize = 4_GiB;
	constexpr size_t TlsRegionSize = 2_GiB;
	constexpr size_t EbrRegionSize = 2_GiB;
	constexpr size_t PoolReserveSize = 32_GiB;
	constexpr size_t SmartPointerControlBlockSize = 16_GiB;
	constexpr size_t ClosureRegionSize = 4_GiB; // ~8M closures @ 512B/closure
	constexpr size_t MemMapFileRegionSize = 32_GiB;
	constexpr size_t InstrumentationRegionSize = 16_GiB;
	constexpr size_t UiDisplayRegionSize = 4_GiB;

	// RuntimeObjects umbrella (96 GiB) — bundles TLSF Heap, Scratch Buffers,
	// Thread-Local, EBR, SmartPtr ControlBlocks, and the Closure Region under one
	// top-level region, matching Corium's Runtime/Infra VA pattern. RuntimeCoreObjects
	// absorbs whatever's left inside the umbrella after the six named sub-regions and
	// their 6 internal guards — analogous to Corium's RuntimeCoreObjects catch-all.
	constexpr size_t RuntimeObjectsSize = 96_GiB;
	constexpr size_t RuntimeCoreObjectsSize = RuntimeObjectsSize
		- (TlsfHeapSize + ScratchBufferSize + TlsRegionSize + EbrRegionSize + SmartPointerControlBlockSize + ClosureRegionSize)
		- 6 * SectionGuardSize;

	// Reserved/Future VA per node — computed as whatever is left after all
	// top-level named regions and guards (7 guards total: 5 inter-region + 2
	// outer null). Sub-region guards inside RuntimeObjects are accounted for
	// separately, out of RuntimeObjectsSize's own budget (see RuntimeCoreObjectsSize).
	constexpr size_t Reserve = g_TotalVA
		- (RuntimeObjectsSize + PoolReserveSize + MemMapFileRegionSize
			+ InstrumentationRegionSize + UiDisplayRegionSize)
		- 7 * NullGuardSize;

	// -------------------------------------------------------------------------
	// Per-node base reservations — one independent VA block per NUMA node
	// -------------------------------------------------------------------------

	extern MemoryHandle g_NodeMemory[MAX_NUMA_NODES];

	// ===========================================================================
	// Top-level VA regions — indexed by NUMA node, in on-disk order  (DO NOT TOUCH!)
	// ===========================================================================

	// +----------------------------------------------------------------+
	// | LOWER NULL GUARD (2 MiB) — unmapped, catches underflow         |
	extern VARegion g_LowerNullGuard[MAX_NUMA_NODES];
	// +----------------------------------------------------------------+

	// +======================================================================+
	//   RUNTIME OBJECTS  (96 GiB)
	//   Umbrella region bundling TLSF Heap, Scratch Buffers, Thread-Local,
	//   EBR, and SmartPtr ControlBlocks — mirrors Corium's Runtime/Infra VA.
	extern VARegion g_RuntimeObjects[MAX_NUMA_NODES];

	//   +------------------------------------------------------------+
	//   | GENERAL HEAP / TLSF (24 GiB)                                |
	//   | Global SpectraHeap instance. Persistent trait.              |
	extern VARegion g_TlsfHeap[MAX_NUMA_NODES];
	//   +------------------------------------------------------------+

	//   +------------------------------------------------------------+
	//   | Guard (2 MiB)                                               |
	extern VARegion g_TlsfHeapGuard[MAX_NUMA_NODES];
	//   +------------------------------------------------------------+

	//   +------------------------------------------------------------+
	//   | SCRATCH BUFFERS (4 GiB)                                    |
	//   | LinearArena / StackArena scratch space. Scratch trait.     |
	//   | Adjacent to General Heap by design.                        |
	extern VARegion g_ScratchBuffers[MAX_NUMA_NODES];
	//   +------------------------------------------------------------+

	//   +------------------------------------------------------------+
	//   | Guard (2 MiB)                                              |
	extern VARegion g_ScratchBuffersGuard[MAX_NUMA_NODES];
	//   +------------------------------------------------------------+

	//   +------------------------------------------------------------+
	//   | THREAD-LOCAL / FIBER-LOCAL (2 GiB)                         |
	//   | Fixed total pool. Per-fiber slice clamped to [2 MiB, 100 MB]|
	//   | ~1024 concurrent fiber estimate.                           |
	extern VARegion g_TlsRegion[MAX_NUMA_NODES];
	//   +------------------------------------------------------------+

	//   +------------------------------------------------------------+
	//   | Guard (2 MiB)                                              |
	extern VARegion g_TlsRegionGuard[MAX_NUMA_NODES];
	//   +------------------------------------------------------------+

	//   +------------------------------------------------------------+
	//   | EBR (2 GiB)                                                |
	//   | EBRInstance objects for epoch-based reclamation.           |
	extern VARegion g_EbrRegion[MAX_NUMA_NODES];
	//   +------------------------------------------------------------+

	//   +------------------------------------------------------------+
	//   | Guard (2 MiB)                                              |
	extern VARegion g_EbrRegionGuard[MAX_NUMA_NODES];
	//   +------------------------------------------------------------+

	//   +------------------------------------------------------------+
	//   | SMARTPTR CONTROL BLOCKS (16 GiB)                           |
	//   | 64B-aligned ControlBlocks for HazardPointer/EBR-tracked    |
	//   | smart pointers.                                            |
	extern VARegion g_SmartPtrControlBlocks[MAX_NUMA_NODES];
	//   +------------------------------------------------------------+

	//   +------------------------------------------------------------+
	//   | Guard (2 MiB)                                              |
	extern VARegion g_SmartPtrControlBlocksGuard[MAX_NUMA_NODES];
	//   +------------------------------------------------------------+

	//   +------------------------------------------------------------+
	//   | CLOSURE REGION (4 GiB)                                     |
	//   | Backing store for Corium-style ClosureFunction/FunctionView|
	//   | payloads (~8M closures @ 512B/closure).                    |
	extern VARegion g_ClosureRegion[MAX_NUMA_NODES];
	//   +------------------------------------------------------------+

	//   +------------------------------------------------------------+
	//   | Guard (2 MiB)                                              |
	extern VARegion g_ClosureRegionGuard[MAX_NUMA_NODES];
	//   +------------------------------------------------------------+

	//   +------------------------------------------------------------+
	//   | RUNTIMECOREOBJECTS — remaining ~43.99 GiB                  |
	//   | Spare headroom for runtime infra not yet designed          |
	//   | (schedulers/executors/pools/global allocators, etc.)       |
	extern VARegion g_RuntimeCoreObjects[MAX_NUMA_NODES];
	//   +------------------------------------------------------------+

	// +======================================================================+

	// +----------------------------------------------------------------+
	// | Guard (2 MiB)                                                  |
	extern VARegion g_RuntimeObjectsGuard[MAX_NUMA_NODES];
	// +----------------------------------------------------------------+

	// +======================================================================+
	//   POOL RESERVATION  (32 GiB)
	//   All PoolAllocator<T> instances and pool internals. Sub-region byte
	//   splits (Queue Allocations / Task Storage / Task I/O+Metadata) are
	//   deferred — see SpectraMemory_VA_Layout_Addendum.md §5.
	extern VARegion g_PoolReserve[MAX_NUMA_NODES];
	// +======================================================================+

	// +----------------------------------------------------------------+
	// | Guard (2 MiB)                                                  |
	extern VARegion g_PoolReserveGuard[MAX_NUMA_NODES];
	// +----------------------------------------------------------------+

	// +----------------------------------------------------------------+
	// | MEMORY-MAPPED FILES / FILE STAGING (32 GiB)                    |
	// | SpectraFileSystem-backed MemoryRegion payload mounting.        |
	extern VARegion g_MemMapFileRegion[MAX_NUMA_NODES];
	// +----------------------------------------------------------------+

	// +----------------------------------------------------------------+
	// | Guard (2 MiB)                                                  |
	extern VARegion g_MemMapFileRegionGuard[MAX_NUMA_NODES];
	// +----------------------------------------------------------------+

	// +----------------------------------------------------------------+
	// | INSTRUMENTATION / DEBUG (16 GiB)                               |
	// | Reserved at init; never committed in release builds.           |
	extern VARegion g_InstrumentationRegion[MAX_NUMA_NODES];
	// +----------------------------------------------------------------+

	// +----------------------------------------------------------------+
	// | Guard (2 MiB)                                                  |
	extern VARegion g_InstrumentationRegionGuard[MAX_NUMA_NODES];
	// +----------------------------------------------------------------+

	// +----------------------------------------------------------------+
	// | UI / DISPLAY STAGING (4 GiB)                                   |
	// | Vulkan host-side staging for UI/display assets.                |
	extern VARegion g_UiDisplayRegion[MAX_NUMA_NODES];
	// +----------------------------------------------------------------+

	// +----------------------------------------------------------------+
	// | Guard (2 MiB)                                                  |
	extern VARegion g_UiDisplayRegionGuard[MAX_NUMA_NODES];
	// +----------------------------------------------------------------+

	// +----------------------------------------------------------------+
	// | RESERVED / FUTURE VA (~75.99 GiB)                              |
	// | Unclaimed headroom for not-yet-designed consumers.             |
	extern VARegion g_ReservedVA[MAX_NUMA_NODES];
	// +----------------------------------------------------------------+

	// +----------------------------------------------------------------+
	// | UPPER NULL GUARD (2 MiB) — unmapped, catches overflow          |
	extern VARegion g_UpperNullGuard[MAX_NUMA_NODES];
	// +----------------------------------------------------------------+

	// Call this before anything else. Every subsystem depends on it.
	SPEC_MEM_RUNTIME_API bool init();
}
