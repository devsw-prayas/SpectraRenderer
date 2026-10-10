#pragma once
#include "SpectraCore.h"
#include "SpecCoreCompiler.h"

namespace Spectra::Core {
	// Brings up the standalones in dependency order. Call once, from the main thread, before any engine work.
	// Returns false on the first stage that fails; nothing after it has run.
	SPEC_CORE_NODISCARD SPEC_CORE_RUNTIME_API bool boot() noexcept;

	// Stub: teardown order is the reverse of boot() once the standalones grow shutdown paths.
	SPEC_CORE_RUNTIME_API void shutdown() noexcept;

	SPEC_CORE_NODISCARD SPEC_CORE_RUNTIME_API bool isBooted() noexcept;
}
