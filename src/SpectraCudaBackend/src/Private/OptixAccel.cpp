#include "OptixAccel.h"

#define ALLOW_SYSCALL
#include "SpecCudaSyscall.h"

#define ALLOW_HELPERS
#include "OptixInternalHelpers.h"
// No standard headers

namespace Spectra::Cuda::Optix {
	OptixAccelBufferSizes DeviceOptixAccel::computeMemoryUsage(
		const GpuOptixContext& ro_Context,
		const OptixAccelBuildOptions& ro_Options,
		const OptixBuildInputDesc* p_Inputs,
		uint32_t v_NumInputs)
	{
		OptixAccelBufferSizes ret{};
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_Context.isValid());
		SPEC_CUDA_BK_ASSERT(p_Inputs != nullptr);

		::OptixAccelBuildOptions nativeOptions{};
		nativeOptions.buildFlags = ro_Options.m_BuildFlags;
		nativeOptions.operation = ro_Options.m_Operation == OptixBuildOperation::BUILD ? OPTIX_BUILD_OPERATION_BUILD : OPTIX_BUILD_OPERATION_UPDATE;

		::OptixBuildInput* nativeInputs = nullptr;
		if (v_NumInputs > 0) {
			nativeInputs = new ::OptixBuildInput[v_NumInputs];
			Internal::Optix_PackingFunctions::packBuildInputs(p_Inputs, v_NumInputs, nativeInputs);
		}

		::OptixAccelBufferSizes nativeSizes{};
		OPTIX_ERROR_TRAP(optixAccelComputeMemoryUsage(
			static_cast<OptixDeviceContext>(ro_Context.m_Handle),
			&nativeOptions,
			nativeInputs,
			v_NumInputs,
			&nativeSizes
		));

		ret.m_OutputSizeInBytes = nativeSizes.outputSizeInBytes;
		ret.m_TempSizeInBytes = nativeSizes.tempSizeInBytes;
		ret.m_TempUpdateSizeInBytes = nativeSizes.tempUpdateSizeInBytes;
		
		if (nativeInputs) {
			delete[] nativeInputs;
		}
#endif
		return ret;
	}

	GpuOptixTraversableHandle DeviceOptixAccel::build(
		const GpuOptixContext& ro_Context,
		const GpuStream& ro_Stream,
		const OptixAccelBuildOptions& ro_Options,
		const OptixBuildInputDesc* p_Inputs,
		uint32_t v_NumInputs,
		uint64_t v_TempBufferAddress,
		size_t v_TempBufferSize,
		uint64_t v_OutputBufferAddress,
		size_t v_OutputBufferSize,
		uint64_t v_CompactedSizePropAddress)
	{
		GpuOptixTraversableHandle handle{};
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_Context.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());
		
		::OptixAccelBuildOptions nativeOptions{};
		nativeOptions.buildFlags = ro_Options.m_BuildFlags;
		nativeOptions.operation = ro_Options.m_Operation == OptixBuildOperation::BUILD ? OPTIX_BUILD_OPERATION_BUILD : OPTIX_BUILD_OPERATION_UPDATE;

		::OptixBuildInput* nativeInputs = nullptr;
		if (v_NumInputs > 0) {
			nativeInputs = new ::OptixBuildInput[v_NumInputs];
			Internal::Optix_PackingFunctions::packBuildInputs(p_Inputs, v_NumInputs, nativeInputs);
		}

		// Emit description if compaction requested
		::OptixAccelEmitDesc emitDesc{};
		emitDesc.type = OPTIX_PROPERTY_TYPE_COMPACTED_SIZE;
		emitDesc.result = static_cast<CUdeviceptr>(v_CompactedSizePropAddress);

		const ::OptixAccelEmitDesc* emitDescPtr = (v_CompactedSizePropAddress != 0) ? &emitDesc : nullptr;
		uint32_t numEmit = (v_CompactedSizePropAddress != 0) ? 1 : 0;

		::OptixTraversableHandle nativeHandle = 0;
		OPTIX_ERROR_TRAP(optixAccelBuild(
			static_cast<OptixDeviceContext>(ro_Context.m_Handle),
			static_cast<CUstream>(ro_Stream.m_StreamHandle),
			&nativeOptions,
			nativeInputs,
			v_NumInputs,
			static_cast<CUdeviceptr>(v_TempBufferAddress),
			v_TempBufferSize,
			static_cast<CUdeviceptr>(v_OutputBufferAddress),
			v_OutputBufferSize,
			&nativeHandle,
			emitDescPtr,
			numEmit
		));

		handle.m_Handle = nativeHandle;

		if (nativeInputs) {
			delete[] nativeInputs;
		}
#endif
		return handle;
	}

	GpuOptixTraversableHandle DeviceOptixAccel::compact(
		const GpuOptixContext& ro_Context,
		const GpuStream& ro_Stream,
		const GpuOptixTraversableHandle& ro_InputHandle,
		uint64_t v_OutputBufferAddress,
		size_t v_OutputBufferSize)
	{
		GpuOptixTraversableHandle handle{};
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_Context.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());
		SPEC_CUDA_BK_ASSERT(ro_InputHandle.isValid());

		::OptixTraversableHandle nativeHandle = 0;
		OPTIX_ERROR_TRAP(optixAccelCompact(
			static_cast<OptixDeviceContext>(ro_Context.m_Handle),
			static_cast<CUstream>(ro_Stream.m_StreamHandle),
			static_cast<::OptixTraversableHandle>(ro_InputHandle.m_Handle),
			static_cast<CUdeviceptr>(v_OutputBufferAddress),
			v_OutputBufferSize,
			&nativeHandle
		));

		handle.m_Handle = nativeHandle;
#endif
		return handle;
	}

	GpuOptixRelocationInfo DeviceOptixAccel::getRelocationInfo(
		const GpuOptixContext& ro_Context,
		const GpuOptixTraversableHandle& ro_Handle) {
		GpuOptixRelocationInfo info{};
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_Context.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Handle.isValid());

		OPTIX_ERROR_TRAP(optixAccelGetRelocationInfo(
			static_cast<OptixDeviceContext>(ro_Context.m_Handle),
			static_cast<::OptixTraversableHandle>(ro_Handle.m_Handle),
			reinterpret_cast<::OptixRelocationInfo*>(info.m_Info)
		));
#endif
		return info;
	}

	GpuOptixTraversableHandle DeviceOptixAccel::relocate(
		const GpuOptixContext& ro_Context,
		const GpuStream& ro_Stream,
		const GpuOptixRelocationInfo& ro_Info,
		uint64_t v_TargetRelocateBufferAddress,
		size_t v_TargetRelocateBufferSize) {
		GpuOptixTraversableHandle handle{};
#ifdef SPECTRA_OPTIX_AVAILABLE
		SPEC_CUDA_BK_ASSERT(ro_Context.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Stream.isValid());
		SPEC_CUDA_BK_ASSERT(ro_Info.isValid());

		::OptixTraversableHandle nativeHandle = 0;
		OPTIX_ERROR_TRAP(optixAccelRelocate(
			static_cast<OptixDeviceContext>(ro_Context.m_Handle),
			static_cast<CUstream>(ro_Stream.m_StreamHandle),
			reinterpret_cast<const ::OptixRelocationInfo*>(ro_Info.m_Info),
			nullptr,
			0,
			static_cast<CUdeviceptr>(v_TargetRelocateBufferAddress),
			v_TargetRelocateBufferSize,
			&nativeHandle
		));

		handle.m_Handle = nativeHandle;
#endif
		return handle;
	}

}
