#pragma once

#include "SpectraCudaBackend.h"
#include "OptixUtils.h"
#include "SpecCudaDiagnostics.h"

#define OPTIX_ERROR_TRAP(result) \
	do { \
		OptixResult res = (result); \
		if (res != OPTIX_SUCCESS) { \
			const char* errorStr = optixGetErrorString(res); \
			SPEC_CUDA_BK_ASSERT(false && errorStr); \
			SPEC_CUDA_BK_TRAP(); \
		} \
	} while (0)

#ifdef ALLOW_HELPERS
namespace Spectra::Cuda::Internal {

#ifdef SPECTRA_OPTIX_AVAILABLE

	class Optix_InternalHelpers final {
	public:
		static OptixCompileOptimizationLevel toOptixCompileOptimizationLevel(Utils::OptixCompileOptimizationLevel v_Level);
		static OptixCompileDebugLevel toOptixCompileDebugLevel(Utils::OptixCompileDebugLevel v_Level);
	};

#endif

}
#else
#error "This is an internal backend header. To use, define ALLOW_HELPERS before inclusion"
#endif
