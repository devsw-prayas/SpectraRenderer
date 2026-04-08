#pragma once

#include "SpectraCudaBackend.h"
#include "OptixUtils.h"
#include "SpecCudaDiagnostics.h"

#define ALLOW_SYSCALL
#include "SpecCudaSyscall.h"

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
		static OptixCompileOptimizationLevel toOptixCompileOptimizationLevel(Utils::OptixCompileOptiLevel v_Level);
		static OptixCompileDebugLevel toOptixCompileDebugLevel(Utils::OptixCompileDebugLvl v_Level);
	};

	class Optix_PackingFunctions final {
	public:
		static void packBuildInputs(const Utils::OptixBuildInputDesc* p_Inputs, uint32_t v_NumInputs, ::OptixBuildInput* p_NativeInputs);
	};

#endif

}
#else
#error "This is an internal backend header. To use, define ALLOW_HELPERS before inclusion"
#endif
