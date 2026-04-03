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

		case Utils::CudaDeviceAttribute::CONCURRENT_MANAGED_ACCESS:
			return CU_DEVICE_ATTRIBUTE_CONCURRENT_MANAGED_ACCESS;
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
		SPEC_CUDA_BK_ASSERT(false && "Invalild Context Scheduling Flags");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUctx_flags_enum CUDA_InternalHelpers::toCudaContextCreationFlags(Utils::ContextCreationFlags flag) {
		switch (flag) {
		case Utils::ContextCreationFlags::MAP_HOST: return CU_CTX_MAP_HOST;
		case Utils::ContextCreationFlags::LMEM_RESIZE_TO_MAX: return CU_CTX_LMEM_RESIZE_TO_MAX;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid Context Creation Flag");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	uint32_t CUDA_InternalHelpers::toHostAllocationFlags(Utils::HostAllocFlags flag) {
		switch (flag) {
		case Utils::HostAllocFlags::ALLOC_DEVICE_MAP: return CU_MEMHOSTALLOC_DEVICEMAP;
		case Utils::HostAllocFlags::ALLOC_PORTABLE:	return CU_MEMHOSTALLOC_PORTABLE;
		case Utils::HostAllocFlags::ALLOC_WRITE_COMBINED: return CU_MEMHOSTALLOC_WRITECOMBINED;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid Host Alloc Flags");
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
		SPEC_CUDA_BK_ASSERT(false && "Invalid Host Register Flags");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUmem_advise_enum CUDA_InternalHelpers::toMemAdviseEnum(Utils::MemoryAdvise flag) {
		switch (flag) {
		case Utils::MemoryAdvise::SET_ACCESSED_BY: return CU_MEM_ADVISE_SET_ACCESSED_BY;
		case Utils::MemoryAdvise::UNSET_ACCESSED_BY: return CU_MEM_ADVISE_UNSET_ACCESSED_BY;
		case Utils::MemoryAdvise::SET_READ_MOSTLY: return CU_MEM_ADVISE_SET_READ_MOSTLY;
		case Utils::MemoryAdvise::UNSET_READ_MOSTLY: return CU_MEM_ADVISE_UNSET_READ_MOSTLY;
		case Utils::MemoryAdvise::SET_PREFERRED_LOCATION: return CU_MEM_ADVISE_SET_PREFERRED_LOCATION;
		case Utils::MemoryAdvise::UNSET_PREFERRED_LOCATION: return CU_MEM_ADVISE_UNSET_PREFERRED_LOCATION;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid Memory Advise flags");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUmemLocationType CUDA_InternalHelpers::toCUlocation(Utils::DeviceLocation flag) {
		switch (flag) {
		case Utils::DeviceLocation::CPU: return CU_MEM_LOCATION_TYPE_HOST;
		case Utils::DeviceLocation::GPU: return CU_MEM_LOCATION_TYPE_DEVICE;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid Device location");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUmemAllocationType CUDA_InternalHelpers::toCuMemAllocationType(Utils::AllocationType flag) {
		switch (flag) {
		case Utils::AllocationType::INVALID: return CU_MEM_ALLOCATION_TYPE_INVALID;
		case Utils::AllocationType::PINNED: return CU_MEM_ALLOCATION_TYPE_PINNED;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid Allocation Type");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUmemAllocationHandleType CUDA_InternalHelpers::toCuMemAllocHandleType(Utils::AllocationHandleType flag) {
		switch (flag) {
		case Utils::AllocationHandleType::NONE: return CU_MEM_HANDLE_TYPE_NONE;
		case Utils::AllocationHandleType::WIN32_HANDLE: return CU_MEM_HANDLE_TYPE_WIN32;
		case Utils::AllocationHandleType::FABRIC_HANDLE: return CU_MEM_HANDLE_TYPE_FABRIC;
		}

		SPEC_CUDA_BK_ASSERT(false && "Invalid Handle type");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUmemAllocationGranularity_flags CUDA_InternalHelpers::toCuMemAllocGranularity(Utils::AllocationGranularityOption v_Option) {
		switch (v_Option) {
		case Utils::AllocationGranularityOption::MINIMUM:
			return CU_MEM_ALLOC_GRANULARITY_MINIMUM;

		case Utils::AllocationGranularityOption::RECOMMENDED:
			return CU_MEM_ALLOC_GRANULARITY_RECOMMENDED;
		}

		SPEC_CUDA_BK_ASSERT(false && "Invalid Allocation Granularity Option");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUarray_format CUDA_InternalHelpers::toArrayFormat(Utils::ArrayFormat v_Format) {
		switch (v_Format) {
		case Utils::ArrayFormat::FP32_ARRAY:   return CU_AD_FORMAT_FLOAT;
		case Utils::ArrayFormat::FP16_ARRAY:   return CU_AD_FORMAT_HALF;
		case Utils::ArrayFormat::UINT8_ARRAY:  return CU_AD_FORMAT_UNSIGNED_INT8;
		case Utils::ArrayFormat::UINT16_ARRAY: return CU_AD_FORMAT_UNSIGNED_INT16;
		case Utils::ArrayFormat::UINT32_ARRAY: return CU_AD_FORMAT_UNSIGNED_INT32;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid ArrayFormat");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	uint32_t CUDA_InternalHelpers::toArrayFlags(Utils::ArrayFlags v_Flags) {
		switch (v_Flags) {
		case Utils::ArrayFlags::TEXTURE_WRITE: return 0;
		case Utils::ArrayFlags::SURFACE_WRITE: return CUDA_ARRAY3D_SURFACE_LDST;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid ArrayFlags");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUresourcetype CUDA_InternalHelpers::toResourceType(Utils::ResourceType v_Type) {
		switch (v_Type) {
		case Utils::ResourceType::ARRAY:            return CU_RESOURCE_TYPE_ARRAY;
		case Utils::ResourceType::MIPMAPPED_ARRAY:  return CU_RESOURCE_TYPE_MIPMAPPED_ARRAY;
		case Utils::ResourceType::LINEAR:           return CU_RESOURCE_TYPE_LINEAR;
		case Utils::ResourceType::PITCH_2D:         return CU_RESOURCE_TYPE_PITCH2D;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid ResourceType");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUaddress_mode CUDA_InternalHelpers::toTexAddressMode(Utils::TexAddressMode v_Mode) {
		switch (v_Mode) {
		case Utils::TexAddressMode::WRAP:   return CU_TR_ADDRESS_MODE_WRAP;
		case Utils::TexAddressMode::CLAMP:  return CU_TR_ADDRESS_MODE_CLAMP;
		case Utils::TexAddressMode::MIRROR: return CU_TR_ADDRESS_MODE_MIRROR;
		case Utils::TexAddressMode::BORDER: return CU_TR_ADDRESS_MODE_BORDER;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid TexAddressMode");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUfilter_mode CUDA_InternalHelpers::toTexFilterMode(Utils::TexFilterMode v_Mode) {
		switch (v_Mode) {
		case Utils::TexFilterMode::POINT:         return CU_TR_FILTER_MODE_POINT;
		case Utils::TexFilterMode::LINEAR_FILTER: return CU_TR_FILTER_MODE_LINEAR;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid TexFilterMode");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUresourceViewFormat CUDA_InternalHelpers::toResourceViewFormat(Utils::ResourceViewFormat v_Format) {
		switch (v_Format) {
		case Utils::ResourceViewFormat::NONE:       return CU_RES_VIEW_FORMAT_NONE;
		case Utils::ResourceViewFormat::UINT_1X8:   return CU_RES_VIEW_FORMAT_UINT_1X8;
		case Utils::ResourceViewFormat::UINT_2X8:   return CU_RES_VIEW_FORMAT_UINT_2X8;
		case Utils::ResourceViewFormat::UINT_4X8:   return CU_RES_VIEW_FORMAT_UINT_4X8;
		case Utils::ResourceViewFormat::SINT_1X8:   return CU_RES_VIEW_FORMAT_SINT_1X8;
		case Utils::ResourceViewFormat::SINT_2X8:   return CU_RES_VIEW_FORMAT_SINT_2X8;
		case Utils::ResourceViewFormat::SINT_4X8:   return CU_RES_VIEW_FORMAT_SINT_4X8;
		case Utils::ResourceViewFormat::UINT_1X16:  return CU_RES_VIEW_FORMAT_UINT_1X16;
		case Utils::ResourceViewFormat::UINT_2X16:  return CU_RES_VIEW_FORMAT_UINT_2X16;
		case Utils::ResourceViewFormat::UINT_4X16:  return CU_RES_VIEW_FORMAT_UINT_4X16;
		case Utils::ResourceViewFormat::SINT_1X16:  return CU_RES_VIEW_FORMAT_SINT_1X16;
		case Utils::ResourceViewFormat::SINT_2X16:  return CU_RES_VIEW_FORMAT_SINT_2X16;
		case Utils::ResourceViewFormat::SINT_4X16:  return CU_RES_VIEW_FORMAT_SINT_4X16;
		case Utils::ResourceViewFormat::UINT_1X32:  return CU_RES_VIEW_FORMAT_UINT_1X32;
		case Utils::ResourceViewFormat::UINT_2X32:  return CU_RES_VIEW_FORMAT_UINT_2X32;
		case Utils::ResourceViewFormat::UINT_4X32:  return CU_RES_VIEW_FORMAT_UINT_4X32;
		case Utils::ResourceViewFormat::SINT_1X32:  return CU_RES_VIEW_FORMAT_SINT_1X32;
		case Utils::ResourceViewFormat::SINT_2X32:  return CU_RES_VIEW_FORMAT_SINT_2X32;
		case Utils::ResourceViewFormat::SINT_4X32:  return CU_RES_VIEW_FORMAT_SINT_4X32;
		case Utils::ResourceViewFormat::FLOAT_1X16: return CU_RES_VIEW_FORMAT_FLOAT_1X16;
		case Utils::ResourceViewFormat::FLOAT_2X16: return CU_RES_VIEW_FORMAT_FLOAT_2X16;
		case Utils::ResourceViewFormat::FLOAT_4X16: return CU_RES_VIEW_FORMAT_FLOAT_4X16;
		case Utils::ResourceViewFormat::FLOAT_1X32: return CU_RES_VIEW_FORMAT_FLOAT_1X32;
		case Utils::ResourceViewFormat::FLOAT_2X32: return CU_RES_VIEW_FORMAT_FLOAT_2X32;
		case Utils::ResourceViewFormat::FLOAT_4X32: return CU_RES_VIEW_FORMAT_FLOAT_4X32;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid ResourceViewFormat");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUmemAccess_flags CUDA_InternalHelpers::toAccessFlags(Utils::AccessFlagBits flag) {
		switch (flag) {
		case Utils::AccessFlagBits::READ:
			return CU_MEM_ACCESS_FLAGS_PROT_READ;

		case Utils::AccessFlagBits::READWRITE:
			return CU_MEM_ACCESS_FLAGS_PROT_READWRITE;

		case Utils::AccessFlagBits::NONE:
			return CU_MEM_ACCESS_FLAGS_PROT_NONE;
		}

		SPEC_CUDA_BK_ASSERT(false && "Invalid Access Flag");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUmemorytype CUDA_InternalHelpers::toCopyMemoryType(Utils::CopyMemoryType v_Type) {
		switch (v_Type) {
		case Utils::CopyMemoryType::HOST:   return CU_MEMORYTYPE_HOST;
		case Utils::CopyMemoryType::DEVICE: return CU_MEMORYTYPE_DEVICE;
		case Utils::CopyMemoryType::ARRAY:  return CU_MEMORYTYPE_ARRAY;
		}
		SPEC_CUDA_BK_ASSERT(false && "Invalid CopyMemoryType");
		SPEC_CUDA_BK_TRAP();
		SPEC_CUDA_BK_UNREACHABLE();
	}

	CUDA_MEMCPY3D CUDA_PackingFunctions::pack3dMemcpyDesc(const Utils::MemCpy3DDesc& ro_Desc) {
		CUDA_MEMCPY3D desc{};

		desc.srcMemoryType  = CUDA_InternalHelpers::toCopyMemoryType(ro_Desc.m_SrcType);
		desc.srcHost        = ro_Desc.m_SrcHost.m_GpuAddr;
		desc.srcDevice      = ro_Desc.m_SrcDevice.m_GpuAddr;
		desc.srcArray       = static_cast<CUarray>(ro_Desc.m_SrcArray.m_Array);
		desc.srcXInBytes    = ro_Desc.m_SrcXOffsetBytes;
		desc.srcY           = ro_Desc.m_SrcYOffset;
		desc.srcZ           = ro_Desc.m_SrcZOffset;
		desc.srcPitch       = ro_Desc.m_SrcPitch;
		desc.srcHeight      = ro_Desc.m_SrcHeight;

		desc.dstMemoryType  = CUDA_InternalHelpers::toCopyMemoryType(ro_Desc.m_DstType);
		desc.dstHost        = ro_Desc.m_DstHost.m_GpuAddr;
		desc.dstDevice      = ro_Desc.m_DstDevice.m_GpuAddr;
		desc.dstArray       = static_cast<CUarray>(ro_Desc.m_DstArray.m_Array);
		desc.dstXInBytes    = ro_Desc.m_DstXOffsetBytes;
		desc.dstY           = ro_Desc.m_DstYOffset;
		desc.dstZ           = ro_Desc.m_DstZOffset;
		desc.dstPitch       = ro_Desc.m_DstPitch;
		desc.dstHeight      = ro_Desc.m_DstHeight;

		desc.WidthInBytes   = ro_Desc.m_WidthInBytes;
		desc.Height         = ro_Desc.m_Height;
		desc.Depth          = ro_Desc.m_Depth;

		return desc;
	}

	CUDA_ARRAY3D_DESCRIPTOR CUDA_PackingFunctions::packArray3dDesc(const Utils::Array3dDesc& ro_Desc) {
		CUDA_ARRAY3D_DESCRIPTOR desc{};

		desc.Width       = ro_Desc.m_Width;
		desc.Height      = ro_Desc.m_Height;
		desc.Depth       = ro_Desc.m_Depth;
		desc.Format      = CUDA_InternalHelpers::toArrayFormat(ro_Desc.m_Format);
		desc.NumChannels = ro_Desc.m_Channels;
		desc.Flags       = ro_Desc.m_Flags;

		return desc;
	}

	CUDA_RESOURCE_DESC CUDA_PackingFunctions::packResourceDesc(const Utils::ResourceDesc& ro_Desc) {
		CUDA_RESOURCE_DESC desc{};
		desc.flags    = 0;
		desc.resType  = CUDA_InternalHelpers::toResourceType(ro_Desc.m_ResType);

		switch (ro_Desc.m_ResType) {
		case Utils::ResourceType::ARRAY:
			desc.res.array.hArray = static_cast<CUarray>(ro_Desc.m_Array.m_Array);
			break;
		case Utils::ResourceType::MIPMAPPED_ARRAY:
			desc.res.mipmap.hMipmappedArray = static_cast<CUmipmappedArray>(ro_Desc.m_MipArray.m_Array);
			break;
		case Utils::ResourceType::LINEAR:
			desc.res.linear.devPtr      = ro_Desc.m_LinearDevice.m_GpuAddr;
			desc.res.linear.format      = CUDA_InternalHelpers::toArrayFormat(ro_Desc.m_LinearFormat);
			desc.res.linear.numChannels = ro_Desc.m_LinearChannels;
			desc.res.linear.sizeInBytes = ro_Desc.m_LinearSizeBytes;
			break;
		case Utils::ResourceType::PITCH_2D:
			desc.res.pitch2D.devPtr       = ro_Desc.m_Pitch2DDevice.m_GpuAddr;
			desc.res.pitch2D.format       = CUDA_InternalHelpers::toArrayFormat(ro_Desc.m_Pitch2DFormat);
			desc.res.pitch2D.numChannels  = ro_Desc.m_Pitch2DChannels;
			desc.res.pitch2D.width        = ro_Desc.m_Pitch2DWidth;
			desc.res.pitch2D.height       = ro_Desc.m_Pitch2DHeight;
			desc.res.pitch2D.pitchInBytes = ro_Desc.m_Pitch2DPitch;
			break;
		}

		return desc;
	}

	CUDA_RESOURCE_VIEW_DESC CUDA_PackingFunctions::packResourceViewDesc(const Utils::ResourceViewDesc& ro_Desc) {
		CUDA_RESOURCE_VIEW_DESC desc{};

		desc.format           = CUDA_InternalHelpers::toResourceViewFormat(ro_Desc.m_Format);
		desc.width            = ro_Desc.m_Width;
		desc.height           = ro_Desc.m_Height;
		desc.depth            = ro_Desc.m_Depth;
		desc.firstMipmapLevel = ro_Desc.m_FirstMipmapLevel;
		desc.lastMipmapLevel  = ro_Desc.m_LastMipmapLevel;
		desc.firstLayer       = ro_Desc.m_FirstLayer;
		desc.lastLayer        = ro_Desc.m_LastLayer;

		return desc;
	}

	CUDA_TEXTURE_DESC CUDA_PackingFunctions::packTextureDesc(const Utils::TextureDesc& ro_Desc) {
		CUDA_TEXTURE_DESC desc{};

		desc.addressMode[0]       = CUDA_InternalHelpers::toTexAddressMode(ro_Desc.m_AddressModeU);
		desc.addressMode[1]       = CUDA_InternalHelpers::toTexAddressMode(ro_Desc.m_AddressModeV);
		desc.addressMode[2]       = CUDA_InternalHelpers::toTexAddressMode(ro_Desc.m_AddressModeW);
		desc.filterMode           = CUDA_InternalHelpers::toTexFilterMode(ro_Desc.m_FilterMode);
		desc.flags                = ro_Desc.m_Flags;
		desc.maxAnisotropy        = ro_Desc.m_MaxAnisotropy;
		desc.mipmapFilterMode     = CUDA_InternalHelpers::toTexFilterMode(ro_Desc.m_MipmapFilterMode);
		desc.mipmapLevelBias      = ro_Desc.m_MipmapLevelBias;
		desc.minMipmapLevelClamp  = ro_Desc.m_MinMipmapLevelClamp;
		desc.maxMipmapLevelClamp  = ro_Desc.m_MaxMipmapLevelClamp;
		desc.borderColor[0]       = ro_Desc.m_BorderColor[0];
		desc.borderColor[1]       = ro_Desc.m_BorderColor[1];
		desc.borderColor[2]       = ro_Desc.m_BorderColor[2];
		desc.borderColor[3]       = ro_Desc.m_BorderColor[3];

		return desc;
	}
}
#endif