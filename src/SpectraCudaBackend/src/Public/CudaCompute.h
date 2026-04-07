#pragma once

#include "SpectraCudaBackend.h"
#include "SpecCudaCompiler.h"
#include "CudaUtils.h"

namespace Spectra::Cuda::Compute {
	using namespace Utils;

	class SPEC_CUDA_BK_RUNTIME_API DeviceCompute final {
	public:
		// -------------------------------------------------------------------------
		// Function Attributes & Configuration
		// -------------------------------------------------------------------------
		static void setFunctionAttribute(const GpuFunction& ro_Function, FunctionAttribute v_Attribute, int v_Value);
		static int  getFunctionAttribute(const GpuFunction& ro_Function, FunctionAttribute v_Attribute);
		
		static void setFunctionCacheConfig(const GpuFunction& ro_Function, FunctionCacheConfig v_Config);
		static void setFunctionSharedMemConfig(const GpuFunction& ro_Function, SharedMemConfig v_Config);

		// -------------------------------------------------------------------------
		// Occupancy Calculators
		// -------------------------------------------------------------------------
		static OccupancyMaxBlockSizeResult calculateMaxPotentialBlockSize(
			const GpuFunction& ro_Function, 
			size_t v_DynamicSMemSize, 
			int v_BlockSizeLimit = 0);

		static int calculateActiveBlocksPerMultiprocessor(
			const GpuFunction& ro_Function,
			int v_BlockSize,
			size_t v_DynamicSMemSize);

		// -------------------------------------------------------------------------
		// Kernel Launch
		// -------------------------------------------------------------------------
		static void launchKernel(
			const GpuFunction& ro_Function,
			const LaunchDimension& ro_GridDim,
			const LaunchDimension& ro_BlockDim,
			uint32_t v_SharedMemBytes,
			const GpuStream& ro_Stream,
			void** pp_KernelParams,
			void** pp_Extra = nullptr);

		static void launchCooperativeKernel(
			const GpuFunction& ro_Function,
			const LaunchDimension& ro_GridDim,
			const LaunchDimension& ro_BlockDim,
			uint32_t v_SharedMemBytes,
			const GpuStream& ro_Stream,
			void** pp_KernelParams);

		// -------------------------------------------------------------------------
		// Variadic Launch Helper
		// -------------------------------------------------------------------------
		template<typename... Args>
		static void launchKernelVariadic(
			const GpuFunction& ro_Function,
			const LaunchDimension& ro_GridDim,
			const LaunchDimension& ro_BlockDim,
			uint32_t v_SharedMemBytes,
			const GpuStream& ro_Stream,
			Args... args) 
		{
			// Unpack arguments into an array of void pointers
			constexpr size_t numArgs = sizeof...(Args);
			if constexpr (numArgs > 0) {
				void* kernelParams[numArgs] = { (void*)&args... };
				launchKernel(ro_Function, ro_GridDim, ro_BlockDim, v_SharedMemBytes, ro_Stream, kernelParams, nullptr);
			} else {
				launchKernel(ro_Function, ro_GridDim, ro_BlockDim, v_SharedMemBytes, ro_Stream, nullptr, nullptr);
			}
		}
	};
}
