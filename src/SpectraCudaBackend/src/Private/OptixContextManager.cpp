#include "OptixContextManager.h"
#define ALLOW_HELPERS
#include "OptixInternalHelpers.h"

// The OptiX function table definition must be instantiated in exactly one translation unit.
#ifdef SPECTRA_OPTIX_AVAILABLE
#include <optix_function_table_definition.h>
#endif

namespace Spectra::Cuda::Optix {
	bool DeviceOptixContext::initOptix() {
#ifdef SPECTRA_OPTIX_AVAILABLE
		OptixResult res = optixInit();
		if (res != OPTIX_SUCCESS) {
			return false;
		}
		return true;
#else
		return false;
#endif
	}

	GpuOptixContext DeviceOptixContext::createContext(const CudaContext& ro_DeviceCtx, const OptixContextOptions& ro_Options) {
		GpuOptixContext ctx{};
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_DeviceCtx.isValid() && "Invalid CUDA context provided for OptiX");

		OptixDeviceContextOptions options{};
		options.logCallbackFunction = ro_Options.m_LogCallbackFunction;
		options.logCallbackData = ro_Options.m_LogCallbackData;
		options.logCallbackLevel = ro_Options.m_LogCallbackLevel;
		options.validationMode = ro_Options.m_Validation == OptixValidationMode::VALIDATION_ON ? 
									OPTIX_DEVICE_CONTEXT_VALIDATION_MODE_ALL : 
									OPTIX_DEVICE_CONTEXT_VALIDATION_MODE_OFF;

		OptixDeviceContext nativeCtx = nullptr;
		OPTIX_ERROR_TRAP(optixDeviceContextCreate(
			static_cast<CUcontext>(ro_DeviceCtx.m_ContextHandle),
			&options,
			&nativeCtx
		));
		
		ctx.m_Handle = static_cast<void*>(nativeCtx);
#endif
		return ctx;
	}

	void DeviceOptixContext::destroyContext(GpuOptixContext& ro_Context) {
#ifdef SPECTRA_OPTIX_AVAILABLE
		if (ro_Context.isValid()) {
			OPTIX_ERROR_TRAP(optixDeviceContextDestroy(static_cast<OptixDeviceContext>(ro_Context.m_Handle)));
			ro_Context.m_Handle = nullptr;
		}
#endif
	}

	void DeviceOptixContext::setLogCallback(const GpuOptixContext& ro_Context, OptixContextOptions::LogCallback p_Callback, void* p_CallbackData, uint32_t v_CallbackLevel) {
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_Context.isValid());
		OPTIX_ERROR_TRAP(optixDeviceContextSetLogCallback(
			static_cast<OptixDeviceContext>(ro_Context.m_Handle),
			p_Callback,
			p_CallbackData,
			v_CallbackLevel
		));
#endif
	}

	void DeviceOptixContext::setCacheEnabled(const GpuOptixContext& ro_Context, int v_Enabled) {
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_Context.isValid());
		OPTIX_ERROR_TRAP(optixDeviceContextSetCacheEnabled(
			static_cast<OptixDeviceContext>(ro_Context.m_Handle),
			v_Enabled
		));
#endif
	}

	void DeviceOptixContext::setCacheDatabaseSizes(const GpuOptixContext& ro_Context, size_t v_LowWatermark, size_t v_HighWatermark) {
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_Context.isValid());
		OPTIX_ERROR_TRAP(optixDeviceContextSetCacheDatabaseSizes(
			static_cast<OptixDeviceContext>(ro_Context.m_Handle),
			v_LowWatermark,
			v_HighWatermark
		));
#endif
	}
}
