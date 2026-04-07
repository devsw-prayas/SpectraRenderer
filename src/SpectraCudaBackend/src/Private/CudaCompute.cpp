#include "CudaCompute.h"
#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Compute {
	void DeviceCompute::setFunctionAttribute(const GpuFunction& ro_Function, FunctionAttribute v_Attribute, int v_Value) {
		SPEC_CUDA_BK_ASSERT(ro_Function.isValid() && "Invalid Function Handle");
		CUDA_ERROR_TRAP(cuFuncSetAttribute(
			static_cast<CUfunction>(ro_Function.m_FunctionHandle),
			Internal::CUDA_InternalHelpers::toCudaFunctionAttr(v_Attribute),
			v_Value
		));
	}

	int DeviceCompute::getFunctionAttribute(const GpuFunction& ro_Function, FunctionAttribute v_Attribute) {
		SPEC_CUDA_BK_ASSERT(ro_Function.isValid() && "Invalid Function Handle");
		int value = 0;
		CUDA_ERROR_TRAP(cuFuncGetAttribute(
			&value,
			Internal::CUDA_InternalHelpers::toCudaFunctionAttr(v_Attribute),
			static_cast<CUfunction>(ro_Function.m_FunctionHandle)
		));
		return value;
	}

	void DeviceCompute::setFunctionCacheConfig(const GpuFunction& ro_Function, FunctionCacheConfig v_Config) {
		SPEC_CUDA_BK_ASSERT(ro_Function.isValid() && "Invalid Function Handle");
		CUDA_ERROR_TRAP(cuFuncSetCacheConfig(
			static_cast<CUfunction>(ro_Function.m_FunctionHandle),
			Internal::CUDA_InternalHelpers::toCudaCacheConfig(v_Config)
		));
	}

	void DeviceCompute::setFunctionSharedMemConfig(const GpuFunction& ro_Function, SharedMemConfig v_Config) {
		SPEC_CUDA_BK_ASSERT(ro_Function.isValid() && "Invalid Function Handle");
		CUDA_ERROR_TRAP(cuFuncSetSharedMemConfig(
			static_cast<CUfunction>(ro_Function.m_FunctionHandle),
			Internal::CUDA_InternalHelpers::toCudaSharedMemConfig(v_Config)
		));
	}

	OccupancyMaxBlockSizeResult DeviceCompute::calculateMaxPotentialBlockSize(
		const GpuFunction& ro_Function,
		size_t v_DynamicSMemSize,
		int v_BlockSizeLimit) 
	{
		SPEC_CUDA_BK_ASSERT(ro_Function.isValid() && "Invalid Function Handle");
		OccupancyMaxBlockSizeResult result;
		CUDA_ERROR_TRAP(cuOccupancyMaxPotentialBlockSize(
			&result.m_MinGridSize,
			&result.m_BlockSize,
			static_cast<CUfunction>(ro_Function.m_FunctionHandle),
			nullptr, // BlockSizeToDynamicSMemSize block size to dynamic smem mapping
			v_DynamicSMemSize,
			v_BlockSizeLimit
		));
		return result;
	}

	int DeviceCompute::calculateActiveBlocksPerMultiprocessor(
		const GpuFunction& ro_Function,
		int v_BlockSize,
		size_t v_DynamicSMemSize) 
	{
		SPEC_CUDA_BK_ASSERT(ro_Function.isValid() && "Invalid Function Handle");
		int numBlocks = 0;
		CUDA_ERROR_TRAP(cuOccupancyMaxActiveBlocksPerMultiprocessor(
			&numBlocks,
			static_cast<CUfunction>(ro_Function.m_FunctionHandle),
			v_BlockSize,
			v_DynamicSMemSize
		));
		return numBlocks;
	}

	void DeviceCompute::launchKernel(
		const GpuFunction& ro_Function,
		const LaunchDimension& ro_GridDim,
		const LaunchDimension& ro_BlockDim,
		uint32_t v_SharedMemBytes,
		const GpuStream& ro_Stream,
		void** pp_KernelParams,
		void** pp_Extra) 
	{
		SPEC_CUDA_BK_ASSERT(ro_Function.isValid() && "Invalid Function Handle");
		CUDA_ERROR_TRAP(cuLaunchKernel(
			static_cast<CUfunction>(ro_Function.m_FunctionHandle),
			ro_GridDim.x, ro_GridDim.y, ro_GridDim.z,
			ro_BlockDim.x, ro_BlockDim.y, ro_BlockDim.z,
			v_SharedMemBytes,
			static_cast<CUstream>(ro_Stream.m_StreamHandle),
			pp_KernelParams,
			pp_Extra
		));
	}

	void DeviceCompute::launchCooperativeKernel(
		const GpuFunction& ro_Function,
		const LaunchDimension& ro_GridDim,
		const LaunchDimension& ro_BlockDim,
		uint32_t v_SharedMemBytes,
		const GpuStream& ro_Stream,
		void** pp_KernelParams) 
	{
		SPEC_CUDA_BK_ASSERT(ro_Function.isValid() && "Invalid Function Handle");
		CUDA_ERROR_TRAP(cuLaunchCooperativeKernel(
			static_cast<CUfunction>(ro_Function.m_FunctionHandle),
			ro_GridDim.x, ro_GridDim.y, ro_GridDim.z,
			ro_BlockDim.x, ro_BlockDim.y, ro_BlockDim.z,
			v_SharedMemBytes,
			static_cast<CUstream>(ro_Stream.m_StreamHandle),
			pp_KernelParams
		));
	}
}
