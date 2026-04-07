#include "SpectraCudaBackend.h"
#include "CudaUtils.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Spectra::Cuda::Utils {
	DeviceHandle DeviceHandle::makeCpu() {
		DeviceHandle handle;
		handle.m_HandleValue = CU_DEVICE_CPU;
		return handle;
	}

	uint32_t CudaHelpers::computeAllocFlag(std::initializer_list<HostAllocFlags> flags) {
		uint32_t mask = 0;
		for (const auto& each : flags)
			mask |= Internal::CUDA_InternalHelpers::toHostAllocationFlags(each);
		return mask;
	}

	uint32_t CudaHelpers::computeRegFlag(std::initializer_list<HostRegisterFlags> flags) {
		uint32_t mask = 0;
		for (const auto& each : flags)
			mask |= Internal::CUDA_InternalHelpers::toHostRegisterFlags(each);
		return mask;
	}

	uint32_t CudaHelpers::computeEventFlags(std::initializer_list<EventFlags> flags) {
		uint32_t mask = 0;
		for (const auto& each : flags)
			mask |= Internal::CUDA_InternalHelpers::toEventFlags(each);
		return mask;
	}

	SPEC_CUDA_BK_RUNTIME_API void initAllocDesc(AllocDesc& ro_Desc) {
		// Zero + explicit defaults (no memset, keep semantic clarity)
		ro_Desc.m_Type = AllocationType::INVALID;
		ro_Desc.m_HandleType = AllocationHandleType::NONE;

		ro_Desc.m_Loc.m_Location = DeviceLocation::GPU;
		ro_Desc.m_Loc.m_Handle = DeviceHandle{}; // default constructed

		ro_Desc.m_win32meta = nullptr;
	}

	SPEC_CUDA_BK_RUNTIME_API void setAllocationType(AllocDesc& ro_Desc, AllocationType v_Type) {
		ro_Desc.m_Type = v_Type;
	}

	SPEC_CUDA_BK_RUNTIME_API void setAllocationHandleType(AllocDesc& ro_Desc, AllocationHandleType v_Type) {
		ro_Desc.m_HandleType = v_Type;
	}

	SPEC_CUDA_BK_RUNTIME_API void setLocation(AllocDesc& ro_Desc, DeviceHandle& ro_Handle) {
		ro_Desc.m_Loc.m_Location = DeviceLocation::GPU;
		ro_Desc.m_Loc.m_Handle = ro_Handle;
	}

	SPEC_CUDA_BK_RUNTIME_API void initAccessDesc(AccessDesc& ro_Desc) {
		// Default: no access, GPU location
		ro_Desc.m_Loc.m_Location = DeviceLocation::GPU;
		ro_Desc.m_Loc.m_Handle = DeviceHandle{};
		ro_Desc.flags = 0;
	}

	SPEC_CUDA_BK_RUNTIME_API void setAccessLocation(AccessDesc& ro_Desc, DeviceHandle& ro_Handle) {
		ro_Desc.m_Loc.m_Location = DeviceLocation::GPU;
		ro_Desc.m_Loc.m_Handle = ro_Handle;
	}

	SPEC_CUDA_BK_RUNTIME_API void setAccessFlags(AccessDesc& ro_Desc, uint64_t v_Flags) {
		ro_Desc.flags = v_Flags;
	}

	SPEC_CUDA_BK_RUNTIME_API void initArray3dDesc(Array3dDesc& ro_Desc) {
		ro_Desc.m_Width    = 0;
		ro_Desc.m_Height   = 0;
		ro_Desc.m_Depth    = 0;
		ro_Desc.m_Channels = 0;
		ro_Desc.m_Flags    = 0;
		ro_Desc.m_Format   = ArrayFormat::FP32_ARRAY;
	}

	SPEC_CUDA_BK_RUNTIME_API void setArrayDimensions(Array3dDesc& ro_Desc, uint64_t v_Width, uint64_t v_Height, uint64_t v_Depth) {
		ro_Desc.m_Width  = v_Width;
		ro_Desc.m_Height = v_Height;
		ro_Desc.m_Depth  = v_Depth;
	}

	SPEC_CUDA_BK_RUNTIME_API void setArrayChannels(Array3dDesc& ro_Desc, uint32_t v_Channels) {
		ro_Desc.m_Channels = v_Channels;
	}

	SPEC_CUDA_BK_RUNTIME_API void setArrayFormat(Array3dDesc& ro_Desc, ArrayFormat v_Format) {
		ro_Desc.m_Format = v_Format;
	}

	SPEC_CUDA_BK_RUNTIME_API void setArrayFlags(Array3dDesc& ro_Desc, ArrayFlags v_Flags) {
		ro_Desc.m_Flags = Internal::CUDA_InternalHelpers::toArrayFlags(v_Flags);
	}

	SPEC_CUDA_BK_RUNTIME_API void initMemCpy3DDesc(MemCpy3DDesc& ro_Desc) {
		ro_Desc.m_SrcType         = CopyMemoryType::HOST;
		ro_Desc.m_SrcHost         = {};
		ro_Desc.m_SrcDevice       = {};
		ro_Desc.m_SrcArray        = {};
		ro_Desc.m_SrcPitch        = 0;
		ro_Desc.m_SrcHeight       = 0;
		ro_Desc.m_SrcXOffsetBytes = 0;
		ro_Desc.m_SrcYOffset      = 0;
		ro_Desc.m_SrcZOffset      = 0;

		ro_Desc.m_DstType         = CopyMemoryType::ARRAY;
		ro_Desc.m_DstHost         = {};
		ro_Desc.m_DstDevice       = {};
		ro_Desc.m_DstArray        = {};
		ro_Desc.m_DstPitch        = 0;
		ro_Desc.m_DstHeight       = 0;
		ro_Desc.m_DstXOffsetBytes = 0;
		ro_Desc.m_DstYOffset      = 0;
		ro_Desc.m_DstZOffset      = 0;

		ro_Desc.m_WidthInBytes    = 0;
		ro_Desc.m_Height          = 0;
		ro_Desc.m_Depth           = 0;
	}

	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DSrcHost(MemCpy3DDesc& ro_Desc, PinnedAddress v_Src) {
		ro_Desc.m_SrcType = CopyMemoryType::HOST;
		ro_Desc.m_SrcHost = v_Src;
	}

	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DSrcDevice(MemCpy3DDesc& ro_Desc, GpuAddress v_Src) {
		ro_Desc.m_SrcType   = CopyMemoryType::DEVICE;
		ro_Desc.m_SrcDevice = v_Src;
	}

	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DSrcArray(MemCpy3DDesc& ro_Desc, CudaArray v_Src) {
		ro_Desc.m_SrcType  = CopyMemoryType::ARRAY;
		ro_Desc.m_SrcArray = v_Src;
	}

	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DDstHost(MemCpy3DDesc& ro_Desc, PinnedAddress v_Dst) {
		ro_Desc.m_DstType = CopyMemoryType::HOST;
		ro_Desc.m_DstHost = v_Dst;
	}

	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DDstDevice(MemCpy3DDesc& ro_Desc, GpuAddress v_Dst) {
		ro_Desc.m_DstType   = CopyMemoryType::DEVICE;
		ro_Desc.m_DstDevice = v_Dst;
	}

	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DDstArray(MemCpy3DDesc& ro_Desc, CudaArray v_Dst) {
		ro_Desc.m_DstType  = CopyMemoryType::ARRAY;
		ro_Desc.m_DstArray = v_Dst;
	}

	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DDimensions(MemCpy3DDesc& ro_Desc, size_t v_WidthInBytes, size_t v_Height, size_t v_Depth) {
		ro_Desc.m_WidthInBytes = v_WidthInBytes;
		ro_Desc.m_Height       = v_Height;
		ro_Desc.m_Depth        = v_Depth;
	}

	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DSrcPitch(MemCpy3DDesc& ro_Desc, size_t v_Pitch, size_t v_Height) {
		ro_Desc.m_SrcPitch  = v_Pitch;
		ro_Desc.m_SrcHeight = v_Height;
	}

	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DDstPitch(MemCpy3DDesc& ro_Desc, size_t v_Pitch, size_t v_Height) {
		ro_Desc.m_DstPitch  = v_Pitch;
		ro_Desc.m_DstHeight = v_Height;
	}

	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DSrcOffsets(MemCpy3DDesc& ro_Desc, size_t v_XOffsetBytes, size_t v_YOffset, size_t v_ZOffset) {
		ro_Desc.m_SrcXOffsetBytes = v_XOffsetBytes;
		ro_Desc.m_SrcYOffset      = v_YOffset;
		ro_Desc.m_SrcZOffset      = v_ZOffset;
	}

	SPEC_CUDA_BK_RUNTIME_API void setMemCpy3DDstOffsets(MemCpy3DDesc& ro_Desc, size_t v_XOffsetBytes, size_t v_YOffset, size_t v_ZOffset) {
		ro_Desc.m_DstXOffsetBytes = v_XOffsetBytes;
		ro_Desc.m_DstYOffset      = v_YOffset;
		ro_Desc.m_DstZOffset      = v_ZOffset;
	}

	SPEC_CUDA_BK_RUNTIME_API void initResourceDesc(ResourceDesc& ro_Desc) {
		ro_Desc.m_ResType         = ResourceType::ARRAY;
		ro_Desc.m_Array           = {};
		ro_Desc.m_MipArray        = {};
		ro_Desc.m_LinearDevice    = {};
		ro_Desc.m_LinearFormat    = ArrayFormat::FP32_ARRAY;
		ro_Desc.m_LinearChannels  = 0;
		ro_Desc.m_LinearSizeBytes = 0;
		ro_Desc.m_Pitch2DDevice   = {};
		ro_Desc.m_Pitch2DFormat   = ArrayFormat::FP32_ARRAY;
		ro_Desc.m_Pitch2DChannels = 0;
		ro_Desc.m_Pitch2DWidth    = 0;
		ro_Desc.m_Pitch2DHeight   = 0;
		ro_Desc.m_Pitch2DPitch    = 0;
	}

	SPEC_CUDA_BK_RUNTIME_API void setResourceArray(ResourceDesc& ro_Desc, CudaArray v_Array) {
		ro_Desc.m_ResType = ResourceType::ARRAY;
		ro_Desc.m_Array   = v_Array;
	}

	SPEC_CUDA_BK_RUNTIME_API void setResourceMipmappedArray(ResourceDesc& ro_Desc, MipmappedArray v_Array) {
		ro_Desc.m_ResType  = ResourceType::MIPMAPPED_ARRAY;
		ro_Desc.m_MipArray = v_Array;
	}

	SPEC_CUDA_BK_RUNTIME_API void setResourceLinear(ResourceDesc& ro_Desc, GpuAddress v_Device, ArrayFormat v_Format, uint32_t v_Channels, size_t v_SizeBytes) {
		ro_Desc.m_ResType         = ResourceType::LINEAR;
		ro_Desc.m_LinearDevice    = v_Device;
		ro_Desc.m_LinearFormat    = v_Format;
		ro_Desc.m_LinearChannels  = v_Channels;
		ro_Desc.m_LinearSizeBytes = v_SizeBytes;
	}

	SPEC_CUDA_BK_RUNTIME_API void setResourcePitch2D(ResourceDesc& ro_Desc, GpuAddress v_Device, ArrayFormat v_Format, uint32_t v_Channels, size_t v_Width, size_t v_Height, size_t v_Pitch) {
		ro_Desc.m_ResType         = ResourceType::PITCH_2D;
		ro_Desc.m_Pitch2DDevice   = v_Device;
		ro_Desc.m_Pitch2DFormat   = v_Format;
		ro_Desc.m_Pitch2DChannels = v_Channels;
		ro_Desc.m_Pitch2DWidth    = v_Width;
		ro_Desc.m_Pitch2DHeight   = v_Height;
		ro_Desc.m_Pitch2DPitch    = v_Pitch;
	}

	SPEC_CUDA_BK_RUNTIME_API void initTextureDesc(TextureDesc& ro_Desc) {
		ro_Desc.m_AddressModeU        = TexAddressMode::CLAMP;
		ro_Desc.m_AddressModeV        = TexAddressMode::CLAMP;
		ro_Desc.m_AddressModeW        = TexAddressMode::CLAMP;
		ro_Desc.m_FilterMode          = TexFilterMode::LINEAR_FILTER;
		ro_Desc.m_Flags               = 0;
		ro_Desc.m_MaxAnisotropy       = 1;
		ro_Desc.m_MipmapFilterMode    = TexFilterMode::POINT;
		ro_Desc.m_MipmapLevelBias     = 0.0f;
		ro_Desc.m_MinMipmapLevelClamp = 0.0f;
		ro_Desc.m_MaxMipmapLevelClamp = 0.0f;
		ro_Desc.m_BorderColor[0]      = 0.0f;
		ro_Desc.m_BorderColor[1]      = 0.0f;
		ro_Desc.m_BorderColor[2]      = 0.0f;
		ro_Desc.m_BorderColor[3]      = 0.0f;
	}

	SPEC_CUDA_BK_RUNTIME_API void setTexAddressMode(TextureDesc& ro_Desc, TexAddressMode v_U, TexAddressMode v_V, TexAddressMode v_W) {
		ro_Desc.m_AddressModeU = v_U;
		ro_Desc.m_AddressModeV = v_V;
		ro_Desc.m_AddressModeW = v_W;
	}

	SPEC_CUDA_BK_RUNTIME_API void setTexFilterMode(TextureDesc& ro_Desc, TexFilterMode v_Mode) {
		ro_Desc.m_FilterMode = v_Mode;
	}

	SPEC_CUDA_BK_RUNTIME_API void setTexFlags(TextureDesc& ro_Desc, uint32_t v_Flags) {
		ro_Desc.m_Flags = v_Flags;
	}

	SPEC_CUDA_BK_RUNTIME_API void setTexMipmapParams(TextureDesc& ro_Desc, TexFilterMode v_FilterMode, float v_Bias, float v_MinClamp, float v_MaxClamp) {
		ro_Desc.m_MipmapFilterMode    = v_FilterMode;
		ro_Desc.m_MipmapLevelBias     = v_Bias;
		ro_Desc.m_MinMipmapLevelClamp = v_MinClamp;
		ro_Desc.m_MaxMipmapLevelClamp = v_MaxClamp;
	}

	SPEC_CUDA_BK_RUNTIME_API void setTexMaxAnisotropy(TextureDesc& ro_Desc, uint32_t v_MaxAnisotropy) {
		ro_Desc.m_MaxAnisotropy = v_MaxAnisotropy;
	}

	SPEC_CUDA_BK_RUNTIME_API void setTexBorderColor(TextureDesc& ro_Desc, float v_R, float v_G, float v_B, float v_A) {
		ro_Desc.m_BorderColor[0] = v_R;
		ro_Desc.m_BorderColor[1] = v_G;
		ro_Desc.m_BorderColor[2] = v_B;
		ro_Desc.m_BorderColor[3] = v_A;
	}

	SPEC_CUDA_BK_RUNTIME_API void initResourceViewDesc(ResourceViewDesc& ro_Desc) {
		ro_Desc.m_Format           = ResourceViewFormat::NONE;
		ro_Desc.m_Width            = 0;
		ro_Desc.m_Height           = 0;
		ro_Desc.m_Depth            = 0;
		ro_Desc.m_FirstMipmapLevel = 0;
		ro_Desc.m_LastMipmapLevel  = 0;
		ro_Desc.m_FirstLayer       = 0;
		ro_Desc.m_LastLayer        = 0;
	}

	SPEC_CUDA_BK_RUNTIME_API void setResourceViewFormat(ResourceViewDesc& ro_Desc, ResourceViewFormat v_Format) {
		ro_Desc.m_Format = v_Format;
	}

	SPEC_CUDA_BK_RUNTIME_API void setResourceViewDimensions(ResourceViewDesc& ro_Desc, size_t v_Width, size_t v_Height, size_t v_Depth) {
		ro_Desc.m_Width  = v_Width;
		ro_Desc.m_Height = v_Height;
		ro_Desc.m_Depth  = v_Depth;
	}

	SPEC_CUDA_BK_RUNTIME_API void setResourceViewMipmapRange(ResourceViewDesc& ro_Desc, uint32_t v_First, uint32_t v_Last) {
		ro_Desc.m_FirstMipmapLevel = v_First;
		ro_Desc.m_LastMipmapLevel  = v_Last;
	}

	SPEC_CUDA_BK_RUNTIME_API void setResourceViewLayerRange(ResourceViewDesc& ro_Desc, uint32_t v_First, uint32_t v_Last) {
		ro_Desc.m_FirstLayer = v_First;
		ro_Desc.m_LastLayer  = v_Last;
	}

	SPEC_CUDA_BK_NODISCARD bool validateArray3dDesc(const Array3dDesc& ro_Desc) {
		if (ro_Desc.m_Width == 0)  return false;                                          // array must have non-zero width
		if (ro_Desc.m_Height == 0) return false;                                          // array must have non-zero height; set Depth=0 for 2D
		const uint32_t ch = ro_Desc.m_Channels;
		if (ch != 1 && ch != 2 && ch != 4) return false;                                 // CUDA only supports 1, 2, or 4 channels
		return true;
	}

	SPEC_CUDA_BK_NODISCARD bool validateMemCpy3DDesc(const MemCpy3DDesc& ro_Desc) {
		if (ro_Desc.m_WidthInBytes == 0) return false;                                    // copy width must be non-zero
		if (ro_Desc.m_Height == 0)       return false;                                    // copy height must be non-zero
		if (ro_Desc.m_Depth == 0)        return false;                                    // copy depth must be non-zero

		if (ro_Desc.m_SrcType == CopyMemoryType::HOST   && !ro_Desc.m_SrcHost.isValid())   return false;  // host source pointer must be valid
		if (ro_Desc.m_SrcType == CopyMemoryType::DEVICE && !ro_Desc.m_SrcDevice.isValid()) return false;  // device source address must be valid
		if (ro_Desc.m_SrcType == CopyMemoryType::ARRAY  && !ro_Desc.m_SrcArray.isValid())  return false;  // source array handle must be valid

		if (ro_Desc.m_DstType == CopyMemoryType::HOST   && !ro_Desc.m_DstHost.isValid())   return false;  // host destination pointer must be valid
		if (ro_Desc.m_DstType == CopyMemoryType::DEVICE && !ro_Desc.m_DstDevice.isValid()) return false;  // device destination address must be valid
		if (ro_Desc.m_DstType == CopyMemoryType::ARRAY  && !ro_Desc.m_DstArray.isValid())  return false;  // destination array handle must be valid

		if (ro_Desc.m_SrcType != CopyMemoryType::ARRAY && ro_Desc.m_Height > 1 && ro_Desc.m_SrcPitch == 0) return false;  // host/device source needs pitch for multi-row copies
		if (ro_Desc.m_DstType != CopyMemoryType::ARRAY && ro_Desc.m_Height > 1 && ro_Desc.m_DstPitch == 0) return false;  // host/device destination needs pitch for multi-row copies

		return true;
	}

	SPEC_CUDA_BK_NODISCARD bool validateResourceDesc(const ResourceDesc& ro_Desc) {
		switch (ro_Desc.m_ResType) {
		case ResourceType::ARRAY:
			if (!ro_Desc.m_Array.isValid())    return false;                              // array handle must be valid
			break;
		case ResourceType::MIPMAPPED_ARRAY:
			if (!ro_Desc.m_MipArray.isValid()) return false;                              // mipmapped array handle must be valid
			break;
		case ResourceType::LINEAR: {
			if (!ro_Desc.m_LinearDevice.isValid())  return false;                         // linear device pointer must be valid
			if (ro_Desc.m_LinearSizeBytes == 0)     return false;                         // linear resource must have non-zero size
			const uint32_t ch = ro_Desc.m_LinearChannels;
			if (ch != 1 && ch != 2 && ch != 4)     return false;                         // CUDA only supports 1, 2, or 4 channels
			break;
		}
		case ResourceType::PITCH_2D: {
			if (!ro_Desc.m_Pitch2DDevice.isValid()) return false;                         // pitch2D device pointer must be valid
			if (ro_Desc.m_Pitch2DWidth == 0)        return false;                         // pitch2D resource must have non-zero width
			if (ro_Desc.m_Pitch2DHeight == 0)       return false;                         // pitch2D resource must have non-zero height
			if (ro_Desc.m_Pitch2DPitch == 0)        return false;                         // row pitch must be non-zero for pitch2D layout
			const uint32_t ch = ro_Desc.m_Pitch2DChannels;
			if (ch != 1 && ch != 2 && ch != 4)     return false;                         // CUDA only supports 1, 2, or 4 channels
			break;
		}
		}
		return true;
	}

	SPEC_CUDA_BK_NODISCARD bool validateTextureDesc(const TextureDesc& ro_Desc) {
		if (ro_Desc.m_MaxAnisotropy < 1)                                          return false;  // anisotropy must be at least 1
		if (ro_Desc.m_MinMipmapLevelClamp > ro_Desc.m_MaxMipmapLevelClamp)       return false;  // mipmap clamp range must be ordered min <= max
		return true;
	}

	SPEC_CUDA_BK_NODISCARD bool validateResourceViewDesc(const ResourceViewDesc& ro_Desc) {
		if (ro_Desc.m_FirstMipmapLevel > ro_Desc.m_LastMipmapLevel) return false;  // mipmap level range must be ordered first <= last
		if (ro_Desc.m_FirstLayer > ro_Desc.m_LastLayer)             return false;  // layer range must be ordered first <= last
		return true;
	}

	uint64_t CudaHelpers::computeAccessFlags(std::initializer_list<AccessFlagBits> flags) {
		uint64_t result = 0;

		for (const auto& f : flags) {
			result |= static_cast<uint64_t>(f);
		}
		return result;
	}

	SPEC_CUDA_BK_RUNTIME_API void initJitOptions(JitOptions& ro_Options) {
		ro_Options.m_OptLevel              = JitOptimizationLevel::DEFAULT_MAX;
		ro_Options.m_Target                = JitTarget::TARGET_AUTO;
		ro_Options.m_CacheMode             = JitCacheMode::NONE;
		ro_Options.m_GenerateDebugInfo     = false;
		ro_Options.m_GenerateLineInfo      = false;
		ro_Options.m_InfoLogBuffer         = nullptr;
		ro_Options.m_InfoLogBufferSize     = 0;
		ro_Options.m_ErrorLogBuffer        = nullptr;
		ro_Options.m_ErrorLogBufferSize    = 0;
		ro_Options.m_MaxRegistersPerThread = 0;
	}

	SPEC_CUDA_BK_RUNTIME_API void setJitOptimization(JitOptions& ro_Options, JitOptimizationLevel v_Level, bool v_DebugInfo, bool v_LineInfo) {
		ro_Options.m_OptLevel          = v_Level;
		ro_Options.m_GenerateDebugInfo = v_DebugInfo;
		ro_Options.m_GenerateLineInfo  = v_LineInfo;
	}

	SPEC_CUDA_BK_RUNTIME_API void setJitHardwareOptions(JitOptions& ro_Options, JitTarget v_Target, JitCacheMode v_Cache, uint32_t v_MaxRegisters) {
		ro_Options.m_Target                = v_Target;
		ro_Options.m_CacheMode             = v_Cache;
		ro_Options.m_MaxRegistersPerThread = v_MaxRegisters;
	}

	SPEC_CUDA_BK_RUNTIME_API void setJitLogBuffers(JitOptions& ro_Options, char* p_InfoLog, uint32_t v_InfoSize, char* p_ErrorLog, uint32_t v_ErrorSize) {
		ro_Options.m_InfoLogBuffer      = p_InfoLog;
		ro_Options.m_InfoLogBufferSize  = v_InfoSize;
		ro_Options.m_ErrorLogBuffer     = p_ErrorLog;
		ro_Options.m_ErrorLogBufferSize = v_ErrorSize;
	}

}