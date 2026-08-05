#pragma once

#include "SpectraPlatformRuntime.h"

namespace Spectra::Platform::Runtime::Windows{

	enum class SPECTRA_RUNTIME_API WindowLifecycle : uint8_t {
		CREATED, SHOWN, DESTROYED
	};

	enum class SPECTRA_RUNTIME_API WindowVisualState : uint8_t {
		RESTORED, MINIMIZED, MAXIMIZED, HIDDEN
	};
}
