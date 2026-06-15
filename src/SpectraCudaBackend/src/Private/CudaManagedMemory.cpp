#include "SpectraCudaBackend.h"
#include "CudaManagedMemory.h"
#include "CudaBootstrap.h"

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
		CUDA_ERROR_TRAP(result);
			return addr;
	}

	void ManagedMemory::adviseMemory(
		const GpuAddress& ro_Addr, size_t v_Count, MemoryAdvise v_Advise,
		const DeviceHandle& ro_Handle) {
		SPEC_CUDA_BK_ASSERT(ro_Addr.isValid());
		SPEC_CUDA_BK_ASSERT(v_Count > 0);
		const CUdevice cudaDev = ro_Handle.isValid()
			? Internal::CUDA_DeviceRegistry::s_Devices[ro_Handle.m_HandleValue]
			: CU_DEVICE_CPU;									 
		if (ro_Handle.isValid()) {
			int concurrentAccess = 0;
			Bootstrap::CudaDeviceManager::getCudaDeviceAttribute(&concurrentAccess, Utils::CudaDeviceAttribute::CONCURRENT_MANAGED_ACCESS, ro_Handle);
			if (!concurrentAccess) return;
		}
		CUmemLocation loc{};
		loc.type = ro_Handle.isValid() ? CU_MEM_LOCATION_TYPE_DEVICE : CU_MEM_LOCATION_TYPE_HOST;
		loc.id   = ro_Handle.isValid() ? static_cast<int>(cudaDev) : 0;
		const CUresult result = cuMemAdvise(
			ro_Addr.m_GpuAddr, v_Count,
			Internal::CUDA_InternalHelpers::toMemAdviseEnum(v_Advise), loc);
		if (result == CUDA_SUCCESS) return;
		CUDA_ERROR_TRAP(result);
	}

	void ManagedMemory::prefetchAsync(
		const GpuAddress& ro_Addr, size_t v_Count,
		const DeviceHandle& ro_Handle, const GpuStream& ro_Stream) {
		SPEC_CUDA_BK_ASSERT(ro_Addr.isValid());
		SPEC_CUDA_BK_ASSERT(v_Count > 0);
		if (ro_Handle.isValid()) {
			int concurrentAccess = 0;
			Bootstrap::CudaDeviceManager::getCudaDeviceAttribute(&concurrentAccess, Utils::CudaDeviceAttribute::CONCURRENT_MANAGED_ACCESS, ro_Handle);
			if (!concurrentAccess) return;
		}
		const CUdevice cudaDev = ro_Handle.isValid()
			? Internal::CUDA_DeviceRegistry::s_Devices[ro_Handle.m_HandleValue]
			: CU_DEVICE_CPU;
		CUmemLocation prefetchLoc{};
		prefetchLoc.type = ro_Handle.isValid() ? CU_MEM_LOCATION_TYPE_DEVICE : CU_MEM_LOCATION_TYPE_HOST;
		prefetchLoc.id   = ro_Handle.isValid() ? static_cast<int>(cudaDev) : 0;
		const CUresult result = cuMemPrefetchAsync(
			ro_Addr.m_GpuAddr, v_Count,
			prefetchLoc, 0, static_cast<CUstream>(ro_Stream.m_StreamHandle));
		if (result == CUDA_SUCCESS) return;
		CUDA_ERROR_TRAP(result);
	}
}