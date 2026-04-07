#pragma once

#include "SpectraCudaBackend.h"
#include "OptixUtils.h"
#include "CudaUtils.h"

namespace Spectra::Cuda::Optix {
	using namespace Utils;

	class SPEC_CUDA_BK_RUNTIME_API DeviceOptixContext final {
	public:
		// Initializes the OptiX driver API by loading the function table. Returns true on success.
		static bool initOptix();

		// Creates an OptiX context tied to the given CUDA context.
		static GpuOptixContext createContext(const CudaContext& ro_DeviceCtx, const OptixContextOptions& ro_Options);
		
		// Destroys the OptiX context.
		static void destroyContext(GpuOptixContext& ro_Context);

		// Runtime cache/logging management
		static void setLogCallback(const GpuOptixContext& ro_Context, OptixContextOptions::LogCallback p_Callback, void* p_CallbackData, uint32_t v_CallbackLevel);
		static void setCacheEnabled(const GpuOptixContext& ro_Context, int v_Enabled);
		static void setCacheDatabaseSizes(const GpuOptixContext& ro_Context, size_t v_LowWatermark, size_t v_HighWatermark);
	};
}
