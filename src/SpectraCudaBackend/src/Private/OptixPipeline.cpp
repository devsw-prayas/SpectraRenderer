#include "OptixPipeline.h"
#define ALLOW_HELPERS
#include "OptixInternalHelpers.h"

#define ALLOW_SYSCALL
#include "SpecCudaSyscall.h"

// No standard library includes in the backend

namespace Spectra::Cuda::Optix {

	GpuOptixModule DeviceOptixPipeline::createModule(
		const GpuOptixContext& ro_Context,
		const OptixModuleCompileOptions& ro_ModuleOptions,
		const OptixPipelineCompileOptions& ro_PipelineOptions,
		const char* p_PtxCode) 
	{
		GpuOptixModule moduleHandle{};
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_Context.isValid());
		SPEC_CUDA_BK_ASSERT(p_PtxCode != nullptr);

		::OptixModuleCompileOptions nativeModuleOptions{};
		nativeModuleOptions.maxRegisterCount = ro_ModuleOptions.m_MaxRegisterCount;
		nativeModuleOptions.optLevel = Internal::Optix_InternalHelpers::toOptixCompileOptimizationLevel(ro_ModuleOptions.m_OptLevel);
		nativeModuleOptions.debugLevel = Internal::Optix_InternalHelpers::toOptixCompileDebugLevel(ro_ModuleOptions.m_DebugLevel);
		nativeModuleOptions.numBoundValues = ro_ModuleOptions.m_BoundValuesCount;

		::OptixPipelineCompileOptions nativePipelineOptions{};
		nativePipelineOptions.usesMotionBlur = ro_PipelineOptions.m_UsesMotionBlur ? 1 : 0;
		nativePipelineOptions.traversableGraphFlags = ro_PipelineOptions.m_TraversableGraphFlags;
		nativePipelineOptions.numPayloadValues = ro_PipelineOptions.m_NumPayloadValues;
		nativePipelineOptions.numAttributeValues = ro_PipelineOptions.m_NumAttributeValues;
		nativePipelineOptions.exceptionFlags = ro_PipelineOptions.m_ExceptionFlags;
		nativePipelineOptions.pipelineLaunchParamsVariableName = ro_PipelineOptions.m_PipelineLaunchParamsVariableName;
		nativePipelineOptions.usesPrimitiveTypeFlags = ro_PipelineOptions.m_UsesPrimitiveTypeFlags;

		::OptixModule nativeModule = nullptr;

		char log[2048];
		size_t sizeofLog = sizeof(log);

		size_t ptxLen = 0;
		while (p_PtxCode[ptxLen] != '\0') {
			ptxLen++;
		}

		OPTIX_ERROR_TRAP(optixModuleCreate(
			static_cast<OptixDeviceContext>(ro_Context.m_Handle),
			&nativeModuleOptions,
			&nativePipelineOptions,
			p_PtxCode,
			ptxLen,
			log,
			&sizeofLog,
			&nativeModule
		));

		moduleHandle.m_Handle = static_cast<void*>(nativeModule);
#endif
		return moduleHandle;
	}

	void DeviceOptixPipeline::destroyModule(GpuOptixModule& ro_Module) {
#ifdef SPECTRA_OPTIX_AVAILABLE
		if (ro_Module.isValid()) {
			OPTIX_ERROR_TRAP(optixModuleDestroy(static_cast<::OptixModule>(ro_Module.m_Handle)));
			ro_Module.m_Handle = nullptr;
		}
#endif
	}

	void DeviceOptixPipeline::createProgramGroups(
		const GpuOptixContext& ro_Context,
		const OptixProgramGroupDesc* p_Descs,
		uint32_t v_Count,
		GpuOptixProgramGroup* p_OutGroups) 
	{
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_Context.isValid());
		SPEC_CUDA_BK_ASSERT(p_Descs != nullptr);
		SPEC_CUDA_BK_ASSERT(p_OutGroups != nullptr);

		::OptixProgramGroupDesc* nativeDescs = new ::OptixProgramGroupDesc[v_Count];
		
		for (uint32_t i = 0; i < v_Count; ++i) {
			nativeDescs[i] = {}; // Zero initialize
			
			switch (p_Descs[i].m_Kind) {
			case OptixProgramGroupKind::RAYGEN:
				nativeDescs[i].kind = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;
				nativeDescs[i].raygen.module = static_cast<::OptixModule>(p_Descs[i].m_ModuleRaygen.m_Handle);
				nativeDescs[i].raygen.entryFunctionName = p_Descs[i].m_EntryFunctionNameRaygen;
				break;
			case OptixProgramGroupKind::MISS:
				nativeDescs[i].kind = OPTIX_PROGRAM_GROUP_KIND_MISS;
				nativeDescs[i].miss.module = static_cast<::OptixModule>(p_Descs[i].m_ModuleMiss.m_Handle);
				nativeDescs[i].miss.entryFunctionName = p_Descs[i].m_EntryFunctionNameMiss;
				break;
			case OptixProgramGroupKind::EXCEPTION:
				nativeDescs[i].kind = OPTIX_PROGRAM_GROUP_KIND_EXCEPTION;
				nativeDescs[i].exception.module = static_cast<::OptixModule>(p_Descs[i].m_ModuleException.m_Handle);
				nativeDescs[i].exception.entryFunctionName = p_Descs[i].m_EntryFunctionNameException;
				break;
			case OptixProgramGroupKind::HITGROUP:
				nativeDescs[i].kind = OPTIX_PROGRAM_GROUP_KIND_HITGROUP;
				nativeDescs[i].hitgroup.moduleCH = static_cast<::OptixModule>(p_Descs[i].m_ModuleHitgroupCH.m_Handle);
				nativeDescs[i].hitgroup.entryFunctionNameCH = p_Descs[i].m_EntryFunctionNameCH;
				nativeDescs[i].hitgroup.moduleAH = static_cast<::OptixModule>(p_Descs[i].m_ModuleHitgroupAH.m_Handle);
				nativeDescs[i].hitgroup.entryFunctionNameAH = p_Descs[i].m_EntryFunctionNameAH;
				nativeDescs[i].hitgroup.moduleIS = static_cast<::OptixModule>(p_Descs[i].m_ModuleHitgroupIS.m_Handle);
				nativeDescs[i].hitgroup.entryFunctionNameIS = p_Descs[i].m_EntryFunctionNameIS;
				break;
			case OptixProgramGroupKind::CALLABLES:
				nativeDescs[i].kind = OPTIX_PROGRAM_GROUP_KIND_CALLABLES;
				nativeDescs[i].callables.moduleDC = static_cast<::OptixModule>(p_Descs[i].m_ModuleCallables.m_Handle);
				nativeDescs[i].callables.entryFunctionNameDC = p_Descs[i].m_EntryFunctionNameDC;
				nativeDescs[i].callables.moduleCC = static_cast<::OptixModule>(p_Descs[i].m_ModuleCallables.m_Handle);
				nativeDescs[i].callables.entryFunctionNameCC = p_Descs[i].m_EntryFunctionNameCC;
				break;
			}
		}

		::OptixProgramGroupOptions nativeOptions{}; // reserved = 0 in OptiX 8
		
		::OptixProgramGroup* nativeGroups = new ::OptixProgramGroup[v_Count];
		for (uint32_t i = 0; i < v_Count; ++i) nativeGroups[i] = nullptr;
		
		char log[2048];
		size_t sizeofLog = sizeof(log);

		OPTIX_ERROR_TRAP(optixProgramGroupCreate(
			static_cast<OptixDeviceContext>(ro_Context.m_Handle),
			nativeDescs,
			v_Count,
			&nativeOptions,
			log,
			&sizeofLog,
			nativeGroups
		));

		for (uint32_t i = 0; i < v_Count; ++i) {
			p_OutGroups[i].m_Handle = static_cast<void*>(nativeGroups[i]);
		}
		
		delete[] nativeDescs;
		delete[] nativeGroups;
#endif
	}

	void DeviceOptixPipeline::destroyProgramGroup(GpuOptixProgramGroup& ro_Group) {
#ifdef SPECTRA_OPTIX_AVAILABLE
		if (ro_Group.isValid()) {
			OPTIX_ERROR_TRAP(optixProgramGroupDestroy(static_cast<::OptixProgramGroup>(ro_Group.m_Handle)));
			ro_Group.m_Handle = nullptr;
		}
#endif
	}

	OptixStackSizes DeviceOptixPipeline::getProgramGroupStackSize(const GpuOptixProgramGroup& ro_Group) {
		OptixStackSizes sizes{};
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_Group.isValid());
		::OptixStackSizes nativeSizes{};
		OPTIX_ERROR_TRAP(optixProgramGroupGetStackSize(
			static_cast<::OptixProgramGroup>(ro_Group.m_Handle),
			&nativeSizes
		));
		sizes.m_CssRG = nativeSizes.cssRG;
		sizes.m_CssMS = nativeSizes.cssMS;
		sizes.m_CssCH = nativeSizes.cssCH;
		sizes.m_CssAH = nativeSizes.cssAH;
		sizes.m_CssIS = nativeSizes.cssIS;
		sizes.m_CssCC = nativeSizes.cssCC;
		sizes.m_DssDC = nativeSizes.dssDC;
#endif
		return sizes;
	}

	GpuOptixPipeline DeviceOptixPipeline::createPipeline(
		const GpuOptixContext& ro_Context,
		const OptixPipelineCompileOptions& ro_CompileOptions,
		const OptixPipelineLinkOptions& ro_LinkOptions,
		const GpuOptixProgramGroup* p_Groups,
		uint32_t v_GroupCount) 
	{
		GpuOptixPipeline pipeline{};
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_Context.isValid());
		SPEC_CUDA_BK_ASSERT(p_Groups != nullptr || v_GroupCount == 0);

		::OptixPipelineCompileOptions nativePipelineOptions{};
		nativePipelineOptions.usesMotionBlur = ro_CompileOptions.m_UsesMotionBlur ? 1 : 0;
		nativePipelineOptions.traversableGraphFlags = ro_CompileOptions.m_TraversableGraphFlags;
		nativePipelineOptions.numPayloadValues = ro_CompileOptions.m_NumPayloadValues;
		nativePipelineOptions.numAttributeValues = ro_CompileOptions.m_NumAttributeValues;
		nativePipelineOptions.exceptionFlags = ro_CompileOptions.m_ExceptionFlags;
		nativePipelineOptions.pipelineLaunchParamsVariableName = ro_CompileOptions.m_PipelineLaunchParamsVariableName;
		nativePipelineOptions.usesPrimitiveTypeFlags = ro_CompileOptions.m_UsesPrimitiveTypeFlags;

		::OptixPipelineLinkOptions nativeLinkOptions{};
		nativeLinkOptions.maxTraceDepth = ro_LinkOptions.m_MaxTraceDepth;

		::OptixProgramGroup* nativeGroups = nullptr;
		if (v_GroupCount > 0) {
			nativeGroups = new ::OptixProgramGroup[v_GroupCount];
			for (uint32_t i = 0; i < v_GroupCount; ++i) {
				nativeGroups[i] = static_cast<::OptixProgramGroup>(p_Groups[i].m_Handle);
			}
		}

		::OptixPipeline nativePipeline = nullptr;
		char log[2048];
		size_t sizeofLog = sizeof(log);

		OPTIX_ERROR_TRAP(optixPipelineCreate(
			static_cast<OptixDeviceContext>(ro_Context.m_Handle),
			&nativePipelineOptions,
			&nativeLinkOptions,
			nativeGroups,
			v_GroupCount,
			log,
			&sizeofLog,
			&nativePipeline
		));

		pipeline.m_Handle = static_cast<void*>(nativePipeline);

		if (nativeGroups) {
			delete[] nativeGroups;
		}
#endif
		return pipeline;
	}

	void DeviceOptixPipeline::destroyPipeline(GpuOptixPipeline& ro_Pipeline) {
#ifdef SPECTRA_OPTIX_AVAILABLE
		if (ro_Pipeline.isValid()) {
			OPTIX_ERROR_TRAP(optixPipelineDestroy(static_cast<::OptixPipeline>(ro_Pipeline.m_Handle)));
			ro_Pipeline.m_Handle = nullptr;
		}
#endif
	}

	void DeviceOptixPipeline::setPipelineStackSize(
		const GpuOptixPipeline& ro_Pipeline,
		uint32_t directCallableStackSizeFromTraversal,
		uint32_t directCallableStackSizeFromState,
		uint32_t continuationStackSize,
		uint32_t maxTraversableGraphDepth) 
	{
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_Pipeline.isValid());
		OPTIX_ERROR_TRAP(optixPipelineSetStackSize(
			static_cast<::OptixPipeline>(ro_Pipeline.m_Handle),
			directCallableStackSizeFromTraversal,
			directCallableStackSizeFromState,
			continuationStackSize,
			maxTraversableGraphDepth
		));
#endif
	}

}
