#pragma once

#include "SpectraCudaBackend.h"
#include "OptixUtils.h"
#include "SpecCudaDiagnostics.h"
#include <ScopeObjects.h>

#define ALLOW_SYSCALL
#include "SpecCudaSyscall.h"

#ifdef ALLOW_HELPERS
namespace Spectra::Cuda::Internal {

#ifdef SPECTRA_OPTIX_AVAILABLE

	// otherwise(...) handler for staticSwitch(optixApiCall(...), caseOf<OPTIX_SUCCESS>(...), otherwise(trapOptixError))
	// Replaces the old OPTIX_ERROR_TRAP macro - same behavior, shared function instead of a repeated lambda.
	inline void trapOptixError(OptixResult res) {
		const char* errorStr = optixGetErrorString(res);
		SPEC_CUDA_BK_ASSERT(false && errorStr);
		SPEC_CUDA_BK_TRAP();
	}

	class Optix_InternalHelpers final {
	public:
		static OptixCompileOptimizationLevel toOptixCompileOptimizationLevel(Utils::OptixCompileOptiLevel v_Level);
		static OptixCompileDebugLevel toOptixCompileDebugLevel(Utils::OptixCompileDebugLvl v_Level);
	};

	class Optix_PackingFunctions final {
	public:
		static void packBuildInputs(const Utils::OptixBuildInputDesc* p_Inputs, uint32_t v_NumInputs, ::OptixBuildInput* p_NativeInputs);
		static void packBoundValues(
			const Utils::OptixBoundValueEntry* p_Entries,
			uint32_t v_Count,
			::OptixModuleCompileBoundValueEntry* p_NativeEntries
		);
	};

#endif

}
#else
#error "This is an internal backend header. To use, define ALLOW_HELPERS before inclusion"
#endif
