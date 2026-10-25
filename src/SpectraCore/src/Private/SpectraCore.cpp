#include "SpectraCore.h"
#include "SpecCoreDiagnostic.h"
#include "SpecCoreThreading.h"
#include "SpecCoreBoot.h"

#include <ProcessEnvironment.h>
#include <PlatformMemory.h>
#include <PlatformFileSystem.h>
#include <SpecMemAddrSpace.h>
#include <SpectraHeap.h>
#include <SpecMemRegistry.h>
#include <CoriumRuntime.h>

namespace {
	bool g_Booted = false;
}

namespace Spectra::Core {
	bool boot() noexcept {
		SPEC_CORE_ASSERT(!g_Booted);
		if (g_Booted) return true;

		Platform::Runtime::Environment::PlatformProbe::init();
		Platform::Runtime::Memory::PlatformVirtualMemory::init();
		Platform::Runtime::File::Files::init();

		if (!Memory::Internal::init()) return false;
		Memory::Heap::HeapRegistry::init(&Platform::Runtime::Environment::PlatformProbe::getCurrentNumaNode);
		Memory::AllocatorRegistry::init();

		Corium::CoriumRuntime::initRuntime();

		g_Booted = true;
		return true;
	}

	void shutdown() noexcept {
		g_Booted = false;
	}

	bool isBooted() noexcept {
		return g_Booted;
	}
}

namespace Spectra::Core::Threads::this_thread {
	ThreadKind kind() noexcept {
		ThreadKind result = ThreadKind::FOREIGN;
		if (Platform::Runtime::Thread::this_platform_thread::isSpectraThread()) result |= ThreadKind::SPECTRA;
		if (Corium::Core::this_thread::currentHandle().expectedState() != Corium::Core::ThreadState::REAPED) result |= ThreadKind::CORIUM;
		return result;
	}
}
