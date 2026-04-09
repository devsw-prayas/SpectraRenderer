#define ALLOW_SYSCALL
#include "SpecCudaSyscall.h"
#define ALLOW_HELPERS
#include "OptixInternalHelpers.h"

namespace Spectra::Cuda::Internal {
#ifdef SPECTRA_OPTIX_AVAILABLE

	OptixCompileOptimizationLevel Optix_InternalHelpers::toOptixCompileOptimizationLevel(Utils::OptixCompileOptiLevel v_Level) {
		switch (v_Level) {
		case Utils::OptixCompileOptiLevel::LEVEL_0: return OPTIX_COMPILE_OPTIMIZATION_LEVEL_0;
		case Utils::OptixCompileOptiLevel::LEVEL_1: return OPTIX_COMPILE_OPTIMIZATION_LEVEL_1;
		case Utils::OptixCompileOptiLevel::LEVEL_2: return OPTIX_COMPILE_OPTIMIZATION_LEVEL_2;
		case Utils::OptixCompileOptiLevel::LEVEL_3: return OPTIX_COMPILE_OPTIMIZATION_LEVEL_3;
		}
		return OPTIX_COMPILE_OPTIMIZATION_DEFAULT;
	}

	OptixCompileDebugLevel Optix_InternalHelpers::toOptixCompileDebugLevel(Utils::OptixCompileDebugLvl v_Level) {
		switch (v_Level) {
		case Utils::OptixCompileDebugLvl::LEVEL_NONE:     return OPTIX_COMPILE_DEBUG_LEVEL_NONE;
		case Utils::OptixCompileDebugLvl::LEVEL_MINIMAL:  return OPTIX_COMPILE_DEBUG_LEVEL_MINIMAL;
		case Utils::OptixCompileDebugLvl::LEVEL_MODERATE: return OPTIX_COMPILE_DEBUG_LEVEL_MODERATE;
		case Utils::OptixCompileDebugLvl::LEVEL_FULL:     return OPTIX_COMPILE_DEBUG_LEVEL_FULL;
		}
		return OPTIX_COMPILE_DEBUG_LEVEL_DEFAULT;
	}

	void Optix_PackingFunctions::packBuildInputs(const Utils::OptixBuildInputDesc* p_Inputs, uint32_t v_NumInputs, ::OptixBuildInput* p_NativeInputs) {
		for (uint32_t i = 0; i < v_NumInputs; ++i) {
			p_NativeInputs[i] = {};
			switch (p_Inputs[i].m_Type) {
			case Utils::OptixBuildInType::TRIANGLES:
				p_NativeInputs[i].type = OPTIX_BUILD_INPUT_TYPE_TRIANGLES;
				p_NativeInputs[i].triangleArray.vertexBuffers = reinterpret_cast<CUdeviceptr*>(const_cast<uint64_t*>(p_Inputs[i].m_TriangleArray.m_VertexBuffers));
				p_NativeInputs[i].triangleArray.numVertices = p_Inputs[i].m_TriangleArray.m_NumVertices;
				p_NativeInputs[i].triangleArray.vertexFormat = static_cast<OptixVertexFormat>(p_Inputs[i].m_TriangleArray.m_VertexFormat);
				p_NativeInputs[i].triangleArray.vertexStrideInBytes = p_Inputs[i].m_TriangleArray.m_VertexStrideInBytes;
				p_NativeInputs[i].triangleArray.indexBuffer = static_cast<CUdeviceptr>(p_Inputs[i].m_TriangleArray.m_IndexBuffer);
				p_NativeInputs[i].triangleArray.numIndexTriplets = p_Inputs[i].m_TriangleArray.m_NumIndexTriplets;
				p_NativeInputs[i].triangleArray.indexFormat = static_cast<OptixIndicesFormat>(p_Inputs[i].m_TriangleArray.m_IndexFormat);
				p_NativeInputs[i].triangleArray.indexStrideInBytes = p_Inputs[i].m_TriangleArray.m_IndexStrideInBytes;
				p_NativeInputs[i].triangleArray.flags = p_Inputs[i].m_TriangleArray.m_Flags;
				p_NativeInputs[i].triangleArray.numSbtRecords = p_Inputs[i].m_TriangleArray.m_NumSbtRecords;
				p_NativeInputs[i].triangleArray.sbtIndexOffsetBuffer = static_cast<CUdeviceptr>(p_Inputs[i].m_TriangleArray.m_SbtIndexOffsetBuffer);
				p_NativeInputs[i].triangleArray.sbtIndexOffsetSizeInBytes = p_Inputs[i].m_TriangleArray.m_SbtIndexOffsetSizeInBytes;
				p_NativeInputs[i].triangleArray.sbtIndexOffsetStrideInBytes = p_Inputs[i].m_TriangleArray.m_SbtIndexOffsetStrideInBytes;
				break;
			case Utils::OptixBuildInType::INSTANCES:
				p_NativeInputs[i].type = OPTIX_BUILD_INPUT_TYPE_INSTANCES;
				p_NativeInputs[i].instanceArray.instances = static_cast<CUdeviceptr>(p_Inputs[i].m_InstanceArray.m_Instances);
				p_NativeInputs[i].instanceArray.numInstances = p_Inputs[i].m_InstanceArray.m_NumInstances;
				break;
			case Utils::OptixBuildInType::CUSTOM_PRIMITIVES:
			case Utils::OptixBuildInType::INSTANCE_POINTERS:
				// Typically less common, deferred parsing.
				SPEC_CUDA_BK_ASSERT(false && "OptixBuildInType not fully exposed yet");
				break;
			}
		}
	}

	void Optix_PackingFunctions::packBoundValues(
		const Utils::OptixBoundValueEntry* p_Entries,
		uint32_t v_Count,
		::OptixModuleCompileBoundValueEntry* p_NativeEntries) {
		for (uint32_t i = 0; i < v_Count; ++i) {
			p_NativeEntries[i].pipelineParamOffsetInBytes = p_Entries[i].m_PipelineParamOffsetInBytes;
			p_NativeEntries[i].sizeInBytes = p_Entries[i].m_SizeInBytes;
			p_NativeEntries[i].boundValuePtr = p_Entries[i].m_BoundValuePtr;
			p_NativeEntries[i].annotation = p_Entries[i].m_Annotation;
		}
	}
#endif
}