#include "SpectraInstrumentation.h"
#include "SpecInstAddrSpace.h"
#include "PlatformMemory.h"

namespace Spectra::Instrumentation::Internal {
	using namespace Spectra::Platform::Runtime::Memory;

	// -------------------------------------------------------------------------
	// Base reservation
	// -------------------------------------------------------------------------

	Utils::Region g_InstrumentationMemory;

	// -------------------------------------------------------------------------
	// Top-level VA regions
	// -------------------------------------------------------------------------

	Utils::Region g_LowerNullGuard;
	Utils::Region g_InstrumentationCore;
	Utils::Region g_UpperNullGuard;

	// -------------------------------------------------------------------------
	// init() -- carves g_InstrumentationMemory into the regions declared in
	// SpecInstAddrSpace.h, in the same on-disk order as the box comments there.
	// No NUMA targeting: this reservation has to exist before SpectraMemory's
	// NUMA-aware VA layer is up, so it's deliberately node-agnostic.
	// -------------------------------------------------------------------------

	bool init() {
		PlatformVirtualMemory::init();

		VirtualMemoryDesc desc{};
		initMemoryDesc(desc);
		setSize(desc, g_TotalVA);
		setNumaNode(desc, INVALID_NUMA_NODE);
		setMemoryState(desc, MemoryState::RESERVE);

		const VirtualMemoryHandle handle = PlatformVirtualMemory::reserve(desc);
		if (!isValid(handle)) return false;

		g_InstrumentationMemory = Utils::Region{ handle.m_BaseAddress, handle.m_TotalSize };

		// Utils::slice() carves from the top of whatever's left (see SpecInstUtility.cpp),
		// so building this low->high layout has to happen top-down: Upper guard first,
		// then Core, and whatever's left at the end (anchored at the true base) is the
		// Lower guard.
		Utils::Region cursor = g_InstrumentationMemory;

		// +----------------------------------------------------------------+
		// | UPPER NULL GUARD (2 MiB)                                       |
		g_UpperNullGuard = Utils::slice(cursor, GuardSize);
		// +----------------------------------------------------------------+

		// +----------------------------------------------------------------+
		// | INSTRUMENTATION CORE (~508 MiB) -- everything but the guards   |
		g_InstrumentationCore = Utils::slice(cursor, cursor.m_MaxSize - GuardSize);
		// +----------------------------------------------------------------+

		// +----------------------------------------------------------------+
		// | LOWER NULL GUARD (2 MiB) -- whatever's left, anchored at base  |
		g_LowerNullGuard = cursor;
		// +----------------------------------------------------------------+

		Utils::lockGuard(g_LowerNullGuard);
		Utils::lockGuard(g_UpperNullGuard);

		return true;
	}
}
