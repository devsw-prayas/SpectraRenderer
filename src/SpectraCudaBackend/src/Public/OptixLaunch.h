#pragma once

#include "SpectraCudaBackend.h"
#include "OptixUtils.h"
#include "CudaUtils.h"

namespace Spectra::Cuda::Optix {
	using namespace Utils;

	struct ShaderBindingTable {
		uint64_t m_RaygenRecord = 0;
		
		uint64_t m_MissRecordBase = 0;
		uint32_t m_MissRecordStrideInBytes = 0;
		uint32_t m_MissRecordCount = 0;
		
		uint64_t m_HitgroupRecordBase = 0;
		uint32_t m_HitgroupRecordStrideInBytes = 0;
		uint32_t m_HitgroupRecordCount = 0;
		
		uint64_t m_CallablesRecordBase = 0;
		uint32_t m_CallablesRecordStrideInBytes = 0;
		uint32_t m_CallablesRecordCount = 0;
	};

	class SPEC_CUDA_BK_RUNTIME_API DeviceOptixLaunch final {
	public:
		// Packs the opaque 32-byte header into the host memory SBT record destination
		static void packSbtRecordHeader(
			const GpuOptixProgramGroup& ro_ProgramGroup,
			void* p_SbtRecordHeaderDest
		);

		// Dispatches rays
		static void launch(
			const GpuOptixPipeline& ro_Pipeline,
			const GpuStream& ro_Stream,
			uint64_t v_PipelineParamsAddress,
			size_t v_PipelineParamsSize,
			const ShaderBindingTable& ro_Sbt,
			uint32_t v_Width,
			uint32_t v_Height,
			uint32_t v_Depth
		);
	};
}
