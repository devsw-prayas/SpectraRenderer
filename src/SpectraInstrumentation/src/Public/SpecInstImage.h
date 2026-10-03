#pragma once
#include "SpectraInstrumentation.h"
#include "SpecInstCompiler.h"
#include <cstdint>

#if !defined(_WIN32)
#error "Image-relative label offsets rely on the MSVC linker's __ImageBase (project is Windows-locked)."
#endif

extern "C" const unsigned char __ImageBase;

namespace Spectra::Instrumentation {
	// Must stay header-inline: __ImageBase has to resolve in the caller's image, not this DLL's.
	SPEC_INST_NODISCARD SPEC_INST_FORCEINLINE uintptr_t currentImageBase() noexcept {
		return reinterpret_cast<uintptr_t>(&__ImageBase);
	}

	SPEC_INST_NODISCARD SPEC_INST_FORCEINLINE uint64_t imageRelativeOffset(const void* p_Label) noexcept {
		return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(p_Label) - currentImageBase());
	}
}
