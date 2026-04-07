#pragma once

#include "SpectraCudaBackend.h"
#include "OptixUtils.h"

namespace Spectra::Cuda::Optix {
	using namespace Utils;

	class SPEC_CUDA_BK_RUNTIME_API DeviceOptixPipeline final {
	public:
		// -------------------------------------------------------------------------
		// Module Compilation
		// -------------------------------------------------------------------------
		// Creates an OptiX module from a PTX string.
		// p_PtxCode must be a null-terminated string.
		static GpuOptixModule createModule(
			const GpuOptixContext& ro_Context,
			const OptixModuleCompileOptions& ro_ModuleOptions,
			const OptixPipelineCompileOptions& ro_PipelineOptions,
			const char* p_PtxCode
		);

		static void destroyModule(GpuOptixModule& ro_Module);

		// -------------------------------------------------------------------------
		// Program Groups
		// -------------------------------------------------------------------------
		// Creates an array of program groups based on the provided descriptors.
		// p_OutGroups must be pre-allocated to hold v_Count handles.
		static void createProgramGroups(
			const GpuOptixContext& ro_Context,
			const OptixProgramGroupDesc* p_Descs,
			uint32_t v_Count,
			GpuOptixProgramGroup* p_OutGroups
		);

		static void destroyProgramGroup(GpuOptixProgramGroup& ro_Group);

		// Retrieves the stack size requirements for a specific program group.
		static OptixStackSizes getProgramGroupStackSize(const GpuOptixProgramGroup& ro_Group);

		// -------------------------------------------------------------------------
		// Pipeline
		// -------------------------------------------------------------------------
		static GpuOptixPipeline createPipeline(
			const GpuOptixContext& ro_Context,
			const OptixPipelineCompileOptions& ro_CompileOptions,
			const OptixPipelineLinkOptions& ro_LinkOptions,
			const GpuOptixProgramGroup* p_Groups,
			uint32_t v_GroupCount
		);

		static void destroyPipeline(GpuOptixPipeline& ro_Pipeline);

		// Configures the stack sizes for the entire pipeline dispatch.
		static void setPipelineStackSize(
			const GpuOptixPipeline& ro_Pipeline,
			uint32_t directCallableStackSizeFromTraversal,
			uint32_t directCallableStackSizeFromState,
			uint32_t continuationStackSize,
			uint32_t maxTraversableGraphDepth
		);
	};
}
