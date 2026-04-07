#define ALLOW_HELPERS
#include "OptixInternalHelpers.h"

namespace Spectra::Cuda::Internal {

#ifdef SPECTRA_OPTIX_AVAILABLE

	OptixCompileOptimizationLevel Optix_InternalHelpers::toOptixCompileOptimizationLevel(Utils::OptixCompileOptimizationLevel v_Level) {
		switch (v_Level) {
		case Utils::OptixCompileOptimizationLevel::LEVEL_0: return OPTIX_COMPILE_OPTIMIZATION_LEVEL_0;
		case Utils::OptixCompileOptimizationLevel::LEVEL_1: return OPTIX_COMPILE_OPTIMIZATION_LEVEL_1;
		case Utils::OptixCompileOptimizationLevel::LEVEL_2: return OPTIX_COMPILE_OPTIMIZATION_LEVEL_2;
		case Utils::OptixCompileOptimizationLevel::LEVEL_3: return OPTIX_COMPILE_OPTIMIZATION_LEVEL_3;
		}
		return OPTIX_COMPILE_OPTIMIZATION_DEFAULT;
	}

	OptixCompileDebugLevel Optix_InternalHelpers::toOptixCompileDebugLevel(Utils::OptixCompileDebugLevel v_Level) {
		switch (v_Level) {
		case Utils::OptixCompileDebugLevel::LEVEL_NONE:     return OPTIX_COMPILE_DEBUG_LEVEL_NONE;
		case Utils::OptixCompileDebugLevel::LEVEL_MINIMAL:  return OPTIX_COMPILE_DEBUG_LEVEL_MINIMAL;
		case Utils::OptixCompileDebugLevel::LEVEL_MODERATE: return OPTIX_COMPILE_DEBUG_LEVEL_MODERATE;
		case Utils::OptixCompileDebugLevel::LEVEL_FULL:     return OPTIX_COMPILE_DEBUG_LEVEL_FULL;
		}
		return OPTIX_COMPILE_DEBUG_LEVEL_DEFAULT;
	}

#endif

}
