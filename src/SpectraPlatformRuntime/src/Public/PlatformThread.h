#pragma once
#include "SpectraPlatformRuntime.h"
#include "ThreadUtils.h"

#include <cstdint>

namespace Spectra::Runtime::Platform::Thread {

	class RUNTIME PlatformThread final {


		friend struct ThreadHandle;
		friend struct ThreadLaunchDesc;
		friend struct ThreadLaunchExecDesc;
	};
}