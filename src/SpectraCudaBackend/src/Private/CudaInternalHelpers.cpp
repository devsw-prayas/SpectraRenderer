#include "SpectraCudaBackend.h"
#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

#ifdef ALLOW_HELPERS
namespace Spectra::Cuda::Internal {
	CUdevice_attribute CUDA_InternalHelpers::toCudaAttr(Utils::CudaDeviceAttribute attr) {
		switch (attr) {
		case Utils::CudaDeviceAttribute::COMPUTE_CAPABILITY_MAJOR:
			return CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR;

		case Utils::CudaDeviceAttribute::COMPUTE_CAPABILITY_MINOR:
			return CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR;

		case Utils::CudaDeviceAttribute::MAX_THREADS_PER_BLOCK:
			return CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK;

		case Utils::CudaDeviceAttribute::MAX_GRID_DIM_X:
			return CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X;

		case Utils::CudaDeviceAttribute::MAX_GRID_DIM_Y:
			return CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y;

		case Utils::CudaDeviceAttribute::MAX_GRID_DIM_Z:
			return CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z;

		case Utils::CudaDeviceAttribute::MAX_SHARED_MEMORY_PER_BLOCK:
			return CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK;

		case Utils::CudaDeviceAttribute::WARP_SIZE:
			return CU_DEVICE_ATTRIBUTE_WARP_SIZE;

		case Utils::CudaDeviceAttribute::MEMORY_CLOCK_RATE:
			return CU_DEVICE_ATTRIBUTE_MEMORY_CLOCK_RATE;

		case Utils::CudaDeviceAttribute::GLOBAL_MEMORY_BUS_WIDTH:
			return CU_DEVICE_ATTRIBUTE_GLOBAL_MEMORY_BUS_WIDTH;

		case Utils::CudaDeviceAttribute::L2_CACHE_SIZE:
			return CU_DEVICE_ATTRIBUTE_L2_CACHE_SIZE;

		case Utils::CudaDeviceAttribute::UNIFIED_ADDRESSING:
			return CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING;

		case Utils::CudaDeviceAttribute::CONCURRENT_KERNELS:
			return CU_DEVICE_ATTRIBUTE_CONCURRENT_KERNELS;

		case Utils::CudaDeviceAttribute::CAN_USE_HOST_POINTER_FOR_REGISTERED_MEM:
			return CU_DEVICE_ATTRIBUTE_CAN_USE_HOST_POINTER_FOR_REGISTERED_MEM;

		case Utils::CudaDeviceAttribute::GPU_DIRECT_RDMA_SUPPORTED:
			return CU_DEVICE_ATTRIBUTE_GPU_DIRECT_RDMA_SUPPORTED;

		case Utils::CudaDeviceAttribute::VIRTUAL_MEMORY_MANAGEMENT_SUPPORTED:
			return CU_DEVICE_ATTRIBUTE_VIRTUAL_MEMORY_MANAGEMENT_SUPPORTED;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid CudaDeviceAttribute");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUctx_flags_enum CUDA_InternalHelpers::toCudaContextScheduleFlags(Utils::ContextSchedulingFlags flag) {
		switch (flag) {
		case Utils::ContextSchedulingFlags::SCHEDULE_AUTO: return CU_CTX_SCHED_AUTO;
		case Utils::ContextSchedulingFlags::SCHEDULE_SPIN: return CU_CTX_SCHED_SPIN;
		case Utils::ContextSchedulingFlags::SCHEDULE_YIELD: return CU_CTX_SCHED_YIELD;
		case Utils::ContextSchedulingFlags::SCHEDULE_BLOCKING_SYNC: return CU_CTX_BLOCKING_SYNC;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid CudaDeviceAttribute");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUctx_flags_enum CUDA_InternalHelpers::toCudaContextCreationFlags(Utils::ContextCreationFlags flag) {
		switch (flag) {
		case Utils::ContextCreationFlags::MAP_HOST: return CU_CTX_MAP_HOST;
		case Utils::ContextCreationFlags::LMEM_RESIZE_TO_MAX: return CU_CTX_LMEM_RESIZE_TO_MAX;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid CudaDeviceAttribute");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	uint32_t CUDA_InternalHelpers::toHostAllocationFlags(Utils::HostAllocFlags flag) {
		switch (flag)
		{
		case Utils::HostAllocFlags::ALLOC_DEVICE_MAP: return CU_MEMHOSTALLOC_DEVICEMAP;
		case Utils::HostAllocFlags::ALLOC_PORTABLE:	return CU_MEMHOSTALLOC_PORTABLE;
		case Utils::HostAllocFlags::ALLOC_WRITE_COMBINED: return CU_MEMHOSTALLOC_WRITECOMBINED;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid CudaDeviceAttribute");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	uint32_t CUDA_InternalHelpers::toHostRegisterFlags(Utils::HostRegisterFlags flag) {
		switch (flag) {
		case Utils::HostRegisterFlags::REG_DEVICE_MAP: return CU_MEMHOSTREGISTER_DEVICEMAP;
		case Utils::HostRegisterFlags::REG_IO_MEMORY: return CU_MEMHOSTREGISTER_IOMEMORY;
		case Utils::HostRegisterFlags::REG_PORTABLE: return CU_MEMHOSTREGISTER_PORTABLE;
		case Utils::HostRegisterFlags::REG_READ_ONLY: return CU_MEMHOSTREGISTER_READ_ONLY;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid CudaDeviceAttribute");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}
}
#endif