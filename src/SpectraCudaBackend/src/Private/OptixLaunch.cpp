#include "OptixLaunch.h"
#define ALLOW_HELPERS
#include "OptixInternalHelpers.h"

namespace Spectra::Cuda::Optix {

	void DeviceOptixLaunch::packSbtRecordHeader(
		const GpuOptixProgramGroup& ro_ProgramGroup,
		void* p_SbtRecordHeaderDest)
	{
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_ProgramGroup.isValid());
		SPEC_CUDA_BK_ASSERT(p_SbtRecordHeaderDest != nullptr);

		OPTIX_ERROR_TRAP(optixSbtRecordPackHeader(
			static_cast<::OptixProgramGroup>(ro_ProgramGroup.m_Handle),
			p_SbtRecordHeaderDest
		));
#endif
	}

	void DeviceOptixLaunch::launch(
		const GpuOptixPipeline& ro_Pipeline,
		const GpuStream& ro_Stream,
		uint64_t v_PipelineParamsAddress,
		size_t v_PipelineParamsSize,
		const OptixShaderBindingTable& ro_Sbt,
		uint32_t v_Width,
		uint32_t v_Height,
		uint32_t v_Depth)
	{
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_Pipeline.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());

		::OptixShaderBindingTable nativeSbt{};
		nativeSbt.raygenRecord = static_cast<CUdeviceptr>(ro_Sbt.m_RaygenRecord);
		
		nativeSbt.exceptionRecordBase = static_cast<CUdeviceptr>(ro_Sbt.m_ExceptionRecordBase);
		nativeSbt.exceptionRecordStrideInBytes = ro_Sbt.m_ExceptionRecordStrideInBytes;
		nativeSbt.exceptionRecordCount = ro_Sbt.m_ExceptionRecordCount;
		
		nativeSbt.missRecordBase = static_cast<CUdeviceptr>(ro_Sbt.m_MissRecordBase);
		nativeSbt.missRecordStrideInBytes = ro_Sbt.m_MissRecordStrideInBytes;
		nativeSbt.missRecordCount = ro_Sbt.m_MissRecordCount;
		
		nativeSbt.hitgroupRecordBase = static_cast<CUdeviceptr>(ro_Sbt.m_HitgroupRecordBase);
		nativeSbt.hitgroupRecordStrideInBytes = ro_Sbt.m_HitgroupRecordStrideInBytes;
		nativeSbt.hitgroupRecordCount = ro_Sbt.m_HitgroupRecordCount;
		
		nativeSbt.callablesRecordBase = static_cast<CUdeviceptr>(ro_Sbt.m_CallablesRecordBase);
		nativeSbt.callablesRecordStrideInBytes = ro_Sbt.m_CallablesRecordStrideInBytes;
		nativeSbt.callablesRecordCount = ro_Sbt.m_CallablesRecordCount;

		OPTIX_ERROR_TRAP(optixLaunch(
			static_cast<::OptixPipeline>(ro_Pipeline.m_Handle),
			static_cast<CUstream>(ro_Stream.m_Handle),
			static_cast<CUdeviceptr>(v_PipelineParamsAddress),
			v_PipelineParamsSize,
			&nativeSbt,
			v_Width,
			v_Height,
			v_Depth
		));
#endif
	}

}
