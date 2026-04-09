#pragma once

#include "SpectraCudaBackend.h"
#include "OptixUtils.h"
#include "CudaUtils.h"

namespace Spectra::Cuda::Optix {
	using namespace Utils;

	class SPEC_CUDA_BK_RUNTIME_API DeviceOptixAccel final {
	public:
		// Computes the memory sizes required for temporary builder buffers and output buffers
		static OptixAccelBufferSizes computeMemoryUsage(
			const GpuOptixContext& ro_Context,
			const OptixAccelBuildOptions& ro_Options,
			const OptixBuildInputDesc* p_Inputs,
			uint32_t v_NumInputs
		);

		// Executes the acceleration structure build process on the given stream
		static GpuOptixTraversableHandle build(
			const GpuOptixContext& ro_Context,
			const GpuStream& ro_Stream,
			const OptixAccelBuildOptions& ro_Options,
			const OptixBuildInputDesc* p_Inputs,
			uint32_t v_NumInputs,
			uint64_t v_TempBufferAddress,
			size_t v_TempBufferSize,
			uint64_t v_OutputBufferAddress,
			size_t v_OutputBufferSize,
			uint64_t v_CompactedSizePropAddress // Optional pointer for emitted property
		);

		// Compacts an existing acceleration structure
		static GpuOptixTraversableHandle compact(
			const GpuOptixContext& ro_Context,
			const GpuStream& ro_Stream,
			const GpuOptixTraversableHandle& ro_InputHandle,
			uint64_t v_OutputBufferAddress,
			size_t v_OutputBufferSize
		);

		// Relocates an existing AS after a memory move without a full rebuild
		static GpuOptixTraversableHandle relocate(
			const GpuOptixContext& ro_Context,
			const GpuStream& ro_Stream,
			const GpuOptixRelocationInfo& ro_Info,
			uint64_t v_TargetRelocateBufferAddress,
			size_t v_TargetRelocateBufferSize
		);

		static GpuOptixRelocationInfo getRelocationInfo(
			const GpuOptixContext& ro_Context,
			const GpuOptixTraversableHandle& ro_Handle
		);
	};
}
