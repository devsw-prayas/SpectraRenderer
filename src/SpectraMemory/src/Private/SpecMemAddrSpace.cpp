#include "SpectraMemory.h"
#include "SpecMemAddrSpace.h"
#include "ProcessEnvironment.h"

namespace Spectra::Memory::Internal {
	using namespace Platform::Runtime::Environment;

	// -------------------------------------------------------------------------
	// Per-node base reservations
	// -------------------------------------------------------------------------

	MemoryHandle g_NodeMemory[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// Top-level VA regions
	// -------------------------------------------------------------------------

	VARegion g_LowerNullGuard[MAX_NUMA_NODES];
	VARegion g_RuntimeObjects[MAX_NUMA_NODES];
	VARegion g_RuntimeObjectsGuard[MAX_NUMA_NODES];
	VARegion g_PoolReserve[MAX_NUMA_NODES];
	VARegion g_PoolReserveGuard[MAX_NUMA_NODES];
	VARegion g_MemMapFileRegion[MAX_NUMA_NODES];
	VARegion g_MemMapFileRegionGuard[MAX_NUMA_NODES];
	VARegion g_InstrumentationRegion[MAX_NUMA_NODES];
	VARegion g_InstrumentationRegionGuard[MAX_NUMA_NODES];
	VARegion g_UiDisplayRegion[MAX_NUMA_NODES];
	VARegion g_UiDisplayRegionGuard[MAX_NUMA_NODES];
	VARegion g_ReservedVA[MAX_NUMA_NODES];
	VARegion g_UpperNullGuard[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// RuntimeObjects sub-regions
	// -------------------------------------------------------------------------

	VARegion g_TlsfHeap[MAX_NUMA_NODES];
	VARegion g_TlsfHeapGuard[MAX_NUMA_NODES];
	VARegion g_ScratchBuffers[MAX_NUMA_NODES];
	VARegion g_ScratchBuffersGuard[MAX_NUMA_NODES];
	VARegion g_TlsRegion[MAX_NUMA_NODES];
	VARegion g_TlsRegionGuard[MAX_NUMA_NODES];
	VARegion g_EbrRegion[MAX_NUMA_NODES];
	VARegion g_EbrRegionGuard[MAX_NUMA_NODES];
	VARegion g_SmartPtrControlBlocks[MAX_NUMA_NODES];
	VARegion g_SmartPtrControlBlocksGuard[MAX_NUMA_NODES];
	VARegion g_ClosureRegion[MAX_NUMA_NODES];
	VARegion g_ClosureRegionGuard[MAX_NUMA_NODES];
	VARegion g_RuntimeCoreObjects[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// init() — carves g_NodeMemory[node] into the regions declared in
	// SpecMemAddrSpace.h, in the same on-disk order as the box comments there.
	// Per SpectraMemory_VA_Layout_Addendum.md §2, node slots beyond the
	// physically present NUMA node count are never initialized.
	// -------------------------------------------------------------------------

	bool init() {
		Memory::init();
		PlatformProbe::init();

		const uint32_t physicalNodes = PlatformProbe::getCpuInfo().m_NumaNodeCount;
		const uint32_t activeNodes = physicalNodes < MAX_NUMA_NODES ? physicalNodes : static_cast<uint32_t>(MAX_NUMA_NODES);

		for (uint32_t node = 0; node < activeNodes; ++node) {
			VirtualMemoryDesc desc{};
			initMemoryDesc(desc);
			setSize(desc, g_TotalVA);
			setNumaNode(desc, node);
			setMemoryState(desc, MemoryState::RESERVE);

			g_NodeMemory[node] = Memory::reserve(desc);

			VARegionSlicer slicer{ g_NodeMemory[node] };

			// +----------------------------------------------------------------+
			// | LOWER NULL GUARD (2 MiB)                                       |
			g_LowerNullGuard[node] = slicer.slice(NullGuardSize);
			// +----------------------------------------------------------------+

			// +======================================================================+
			//   RUNTIME OBJECTS  (96 GiB)
			g_RuntimeObjects[node] = slicer.slice(RuntimeObjectsSize);
			// +======================================================================+

			// +----------------------------------------------------------------+
			// | Guard (2 MiB)                                                  |
			g_RuntimeObjectsGuard[node] = slicer.slice(SectionGuardSize);
			// +----------------------------------------------------------------+

			// +======================================================================+
			//   POOL RESERVATION  (32 GiB)
			g_PoolReserve[node] = slicer.slice(PoolReserveSize);
			// +======================================================================+

			// +----------------------------------------------------------------+
			// | Guard (2 MiB)                                                  |
			g_PoolReserveGuard[node] = slicer.slice(SectionGuardSize);
			// +----------------------------------------------------------------+

			// +----------------------------------------------------------------+
			// | MEMORY-MAPPED FILES / FILE STAGING (32 GiB)                    |
			g_MemMapFileRegion[node] = slicer.slice(MemMapFileRegionSize);
			// +----------------------------------------------------------------+

			// +----------------------------------------------------------------+
			// | Guard (2 MiB)                                                  |
			g_MemMapFileRegionGuard[node] = slicer.slice(SectionGuardSize);
			// +----------------------------------------------------------------+

			// +----------------------------------------------------------------+
			// | INSTRUMENTATION / DEBUG (16 GiB)                               |
			g_InstrumentationRegion[node] = slicer.slice(InstrumentationRegionSize);
			// +----------------------------------------------------------------+

			// +----------------------------------------------------------------+
			// | Guard (2 MiB)                                                  |
			g_InstrumentationRegionGuard[node] = slicer.slice(SectionGuardSize);
			// +----------------------------------------------------------------+

			// +----------------------------------------------------------------+
			// | UI / DISPLAY STAGING (4 GiB)                                   |
			g_UiDisplayRegion[node] = slicer.slice(UiDisplayRegionSize);
			// +----------------------------------------------------------------+

			// +----------------------------------------------------------------+
			// | Guard (2 MiB)                                                  |
			g_UiDisplayRegionGuard[node] = slicer.slice(SectionGuardSize);
			// +----------------------------------------------------------------+

			// +----------------------------------------------------------------+
			// | RESERVED / FUTURE VA (~75.99 GiB)                              |
			g_ReservedVA[node] = slicer.slice(Reserve);
			// +----------------------------------------------------------------+

			// +----------------------------------------------------------------+
			// | UPPER NULL GUARD (2 MiB)                                       |
			g_UpperNullGuard[node] = slicer.slice(slicer.remaining());
			// +----------------------------------------------------------------+

			// --- RuntimeObjects sub-regions ---------------------------------

			{
				VARegionSlicer rt{ g_RuntimeObjects[node] };

				//   +------------------------------------------------------------+
				//   | GENERAL HEAP / TLSF (24 GiB)                                |
				g_TlsfHeap[node] = rt.slice(TlsfHeapSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | Guard (2 MiB)                                               |
				g_TlsfHeapGuard[node] = rt.slice(SectionGuardSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | SCRATCH BUFFERS (4 GiB)                                     |
				g_ScratchBuffers[node] = rt.slice(ScratchBufferSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | Guard (2 MiB)                                               |
				g_ScratchBuffersGuard[node] = rt.slice(SectionGuardSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | THREAD-LOCAL / FIBER-LOCAL (2 GiB)                          |
				g_TlsRegion[node] = rt.slice(TlsRegionSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | Guard (2 MiB)                                               |
				g_TlsRegionGuard[node] = rt.slice(SectionGuardSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | EBR (2 GiB)                                                 |
				g_EbrRegion[node] = rt.slice(EbrRegionSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | Guard (2 MiB)                                               |
				g_EbrRegionGuard[node] = rt.slice(SectionGuardSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | SMARTPTR CONTROL BLOCKS (16 GiB)                            |
				g_SmartPtrControlBlocks[node] = rt.slice(SmartPointerControlBlockSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | Guard (2 MiB)                                               |
				g_SmartPtrControlBlocksGuard[node] = rt.slice(SectionGuardSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | CLOSURE REGION (4 GiB)                                      |
				g_ClosureRegion[node] = rt.slice(ClosureRegionSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | Guard (2 MiB)                                               |
				g_ClosureRegionGuard[node] = rt.slice(SectionGuardSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | RUNTIMECOREOBJECTS — remaining ~43.99 GiB                  |
				g_RuntimeCoreObjects[node] = rt.slice(rt.remaining());
				//   +------------------------------------------------------------+
			}

			// --- Lock guard regions for this node --------------------------------

			lockGuard(g_LowerNullGuard[node]);
			lockGuard(g_RuntimeObjectsGuard[node]);
			lockGuard(g_PoolReserveGuard[node]);
			lockGuard(g_MemMapFileRegionGuard[node]);
			lockGuard(g_InstrumentationRegionGuard[node]);
			lockGuard(g_UiDisplayRegionGuard[node]);
			lockGuard(g_UpperNullGuard[node]);

			lockGuard(g_TlsfHeapGuard[node]);
			lockGuard(g_ScratchBuffersGuard[node]);
			lockGuard(g_TlsRegionGuard[node]);
			lockGuard(g_EbrRegionGuard[node]);
			lockGuard(g_SmartPtrControlBlocksGuard[node]);
			lockGuard(g_ClosureRegionGuard[node]);
		}

		return true;
	}

} 
