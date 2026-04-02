#include "SpectraCudaBackend.h"
#include "CudaManagedMemory.h"

#define ALLOW_SYSCALL
#include "SpecCudaSyscall.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Memory {
	GpuAddress ManagedMemory::allocManaged(size_t v_Bytes, uint32_t v_Flags) {
		GpuAddress addr{};
		CUdeviceptr ptr;
		SPEC_CUDA_BK_ASSERT(v_Bytes > 0);
		const CUresult result = cuMemAllocManaged(&ptr, v_Bytes, v_Flags);
		if (result == CUDA_SUCCESS) {
			addr.m_GpuAddr = ptr;
			return addr;
		}
		CUDA_ERROR_TRAP(result)
			return addr;
	}

	void ManagedMemory::adviseMemory(
		const GpuAddress& ro_Addr, size_t v_Count, MemoryAdvise v_Advise,
		const DeviceHandle& ro_Handle) {
		SPEC_CUDA_BK_ASSERT(ro_Addr.isValid());
		SPEC_CUDA_BK_ASSERT(v_Count > 0);
		const CUresult result = cuMemAdvise(
			ro_Addr.m_GpuAddr, v_Count,
			Internal::CUDA_InternalHelpers::toMemAdviseEnum(v_Advise), ro_Handle.m_HandleValue);
		if (result == CUDA_SUCCESS) return;
		CUDA_ERROR_TRAP(result)
	}

	void ManagedMemory::prefetchAsync(
		const GpuAddress& ro_Addr, size_t v_Count,
		const DeviceHandle& ro_Handle, const Streams::GpuStream& ro_Stream) {
		SPEC_CUDA_BK_ASSERT(ro_Addr.isValid());
		SPEC_CUDA_BK_ASSERT(v_Count > 0);
		const CUresult result = cuMemPrefetchAsync(
			ro_Addr.m_GpuAddr, v_Count,
			ro_Handle.m_HandleValue, static_cast<CUstream>(ro_Stream.m_StreamHandle));
		if (result == CUDA_SUCCESS) return;
		CUDA_ERROR_TRAP(result)
	}
}