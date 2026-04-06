#pragma once
#include "SpectraCudaBackend.h"
#include "CudaUtils.h"

#define CUDA_ERROR_TRAP(result)	 \
	do{												 \
		const char* errorStr = nullptr;				   \
		cuGetErrorString(result, &errorStr);		   \
		SPEC_CUDA_BK_ASSERT(false && errorStr);		   \
		SPEC_CUDA_BK_TRAP();						   \
	}while(0); 										   \

#ifdef ALLOW_HELPERS
#include <cuda.h>

namespace Spectra::Cuda::Internal {
	class CUDA_DeviceRegistry {
	public:
		static constexpr int MAX_DEVICE_COUNT = 32;
		static inline CUdevice s_Devices[MAX_DEVICE_COUNT] = {};
		static inline int s_DeviceCount = 0;
		static constexpr size_t CUDA_UUID_LENGTH = 16;

		static bool validateDevice(int v_Ordinal) {
			if (v_Ordinal >= s_DeviceCount || v_Ordinal < 0) return false;
			return true;
		}
	};

	class CUDA_InternalHelpers final {
	public:
		static CUdevice_attribute toCudaAttr(Utils::CudaDeviceAttribute attr);
		static CUctx_flags_enum toCudaContextScheduleFlags(Utils::ContextSchedulingFlags flag);
		static CUctx_flags_enum toCudaContextCreationFlags(Utils::ContextCreationFlags flag);

		static uint32_t toHostAllocationFlags(Utils::HostAllocFlags flag);
		static uint32_t toHostRegisterFlags(Utils::HostRegisterFlags flag);
		static CUmem_advise_enum toMemAdviseEnum(Utils::MemoryAdvise flag);

		static CUmemLocationType toCUlocation(Utils::DeviceLocation flag);
		static CUmemAllocationType toCuMemAllocationType(Utils::AllocationType flag);
		static CUmemAllocationHandleType toCuMemAllocHandleType(Utils::AllocationHandleType flag);

		static CUmemAccess_flags toAccessFlags(Utils::AccessFlagBits flag);
		static CUmemAllocationGranularity_flags toCuMemAllocGranularity(Utils::AllocationGranularityOption v_Option);
		static CUarray_format toArrayFormat(Utils::ArrayFormat v_Format);

		static uint32_t toArrayFlags(Utils::ArrayFlags v_Flags);
		static CUresourcetype toResourceType(Utils::ResourceType v_Type);
		static CUaddress_mode toTexAddressMode(Utils::TexAddressMode v_Mode);

		static CUfilter_mode toTexFilterMode(Utils::TexFilterMode v_Mode);
		static CUresourceViewFormat toResourceViewFormat(Utils::ResourceViewFormat v_Format);
		static CUmemorytype toCopyMemoryType(Utils::CopyMemoryType v_Type);
		static CUstreamCaptureMode toCudaStreamCaptureMode(Utils::StreamCaptureMode v_Mode);
		static CUstreamCaptureStatus toCudaStreamCaptureStatus(Utils::StreamCaptureStatus v_Status);
		static uint32_t toStreamFlags(Utils::StreamFlags v_Flags);
		static uint32_t toEventFlags(Utils::EventFlags v_Flags);
	};

	class CUDA_PackingFunctions final {
	public:
		static CUDA_ARRAY3D_DESCRIPTOR    packArray3dDesc(const Utils::Array3dDesc& ro_Desc);       // → cuArray3DCreate
		static CUDA_MEMCPY3D              pack3dMemcpyDesc(const Utils::MemCpy3DDesc& ro_Desc);    // → cuMemcpy3DAsync, cuGraphAddMemcpyNode
		static CUDA_RESOURCE_DESC         packResourceDesc(const Utils::ResourceDesc& ro_Desc);    // → cuTexObjectCreate, cuSurfObjectCreate
		static CUDA_TEXTURE_DESC          packTextureDesc(const Utils::TextureDesc& ro_Desc);      // → cuTexObjectCreate
		static CUDA_RESOURCE_VIEW_DESC    packResourceViewDesc(const Utils::ResourceViewDesc& ro_Desc); // → cuTexObjectCreate
		static CUDA_KERNEL_NODE_PARAMS    packKernelNodeParams(const Utils::KernelNodeParams& ro_Params); // → cuGraphAddKernelNode
		static CUDA_MEMSET_NODE_PARAMS    packMemsetNodeParams(const Utils::MemsetNodeParams& ro_Params); // → cuGraphAddMemsetNode
	};
}

#else
#error "This is an internal backend header. To use, define ALLOW_SYSCALL before inclusion"
#endif
