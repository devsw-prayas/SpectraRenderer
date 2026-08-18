#pragma once
#include "SpectraInstrumentation.h"
#include "SpecInstUtility.h"

namespace Spectra::Instrumentation::Internal {
	// SPECTRA INSTRUMENTATION VIRTUAL ADDRESS SPACE -- 512 MiB, single reservation,
	// no NUMA topology. This is core bootstrap: it has to be up before SpectraMemory
	// exists (deterministic instrumentation/profiling has to observe boot itself),
	// so it reserves its own VA directly off PlatformVirtualMemory instead of going
	// through SpectraMemory's VAUtils/MemoryRegion. Layout is documented immediately
	// above the extern(s) that back it, in on-disk (low -> high address) order.

	// -------------------------------------------------------------------------
	// Guard and total sizes
	// -------------------------------------------------------------------------

	constexpr size_t GuardSize = 2ULL * 1024ULL * 1024ULL; // 2 MiB
	constexpr size_t g_TotalVA = 512ULL * 1024ULL * 1024ULL; // 512 MiB

	// -------------------------------------------------------------------------
	// Base reservation
	// -------------------------------------------------------------------------

	extern Utils::Region g_InstrumentationMemory;

	// ===========================================================================
	// Top-level VA regions, in on-disk order  (DO NOT TOUCH!)
	// ===========================================================================

	// +----------------------------------------------------------------+
	// | LOWER NULL GUARD (2 MiB) -- unmapped, catches underflow        |
	extern Utils::Region g_LowerNullGuard;
	// +----------------------------------------------------------------+

	// +----------------------------------------------------------------+
	// | INSTRUMENTATION CORE (~508 MiB)                                |
	// | Catch-all placeholder -- not yet sliced into named sub-regions.|
	// | TODO: carve into real buffers (event ring, capture staging,    |
	// | etc.) once their sizes are locked.                             |
	extern Utils::Region g_InstrumentationCore;
	// +----------------------------------------------------------------+

	// +----------------------------------------------------------------+
	// | UPPER NULL GUARD (2 MiB) -- unmapped, catches overflow         |
	extern Utils::Region g_UpperNullGuard;
	// +----------------------------------------------------------------+

	// Call this before anything else. SpectraMemory's own init() runs after this.
	SPEC_INST_RUNTIME_API bool init();
}
