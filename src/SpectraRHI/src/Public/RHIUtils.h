#pragma once
#include "SpectraRHI.h"
#include "SpecRHICompiler.h"

namespace Spectra::RHI::Utils {
	ENUM_CLASS_8(RHIBackendType) {
		Vulkan = 0,
		CUDA   = 1
	};

	ENUM_CLASS_8(RHIResult) {
		Success,
		DeviceLost,
		OutOfMemory,
		InvalidUsage,
		Unsupported
	};

	ENUM_CLASS_8(RHIQueueType) {
		None,
		Graphics,
		Compute,
		Transfer,
		CUDA_Stream
	};

	ENUM_CLASS_8(RHIMemoryLocation) {
		Device,
		Upload,
		Readback
	};

	ENUM_CLASS_32(RHIMemoryFlags) {
		None               = 0,
		Dedicated          = 1 << 0,
		PersistentlyMapped = 1 << 1,
		ExternalShared     = 1 << 2,
		DeviceAddress      = 1 << 3
	};

	ENUM_CLASS_8(RHIResourceOwner) {
		None     = 0,
		Vulkan   = 1,
		CUDA     = 2,
		External = 3
	};

	ENUM_CLASS_8(RHIResourceOrigin) {
		Internal = 0,
		Imported = 1
	};

	ENUM_CLASS_8(RHIStage) {
		None,
		VertexInput,
		Graphics,
		Compute,
		RayTracing,
		Transfer,
		All
	};

	ENUM_CLASS_8(RHIAccess) {
		None,
		Read,
		Write,
		RenderTarget,
		DepthWrite,
		ASBuild
	};

	ENUM_CLASS_32(RHIFormat) {
		Undefined = 0,

		// 8-bit
		R8_UNORM, R8_SNORM, R8_UINT, R8_SINT,
		RG8_UNORM, RG8_SNORM, RG8_UINT, RG8_SINT,
		RGBA8_UNORM, RGBA8_SNORM, RGBA8_UINT, RGBA8_SINT, RGBA8_SRGB,
		BGRA8_UNORM, BGRA8_SRGB,

		// 16-bit
		R16_UNORM, R16_SNORM, R16_UINT, R16_SINT, R16_SFLOAT,
		RG16_UNORM, RG16_SNORM, RG16_UINT, RG16_SINT, RG16_SFLOAT,
		RGBA16_UNORM, RGBA16_SNORM, RGBA16_UINT, RGBA16_SINT, RGBA16_SFLOAT,

		// 32-bit
		R32_UINT, R32_SINT, R32_SFLOAT,
		RG32_UINT, RG32_SINT, RG32_SFLOAT,
		RGB32_UINT, RGB32_SINT, RGB32_SFLOAT,
		RGBA32_UINT, RGBA32_SINT, RGBA32_SFLOAT,

		// Packed
		RGB10A2_UNORM,
		B10GR11_UFLOAT,
		E5B9G9R9_UFLOAT,

		// Depth / stencil
		D16_UNORM,
		D32_SFLOAT,
		D24_UNORM_S8_UINT,
		D32_SFLOAT_S8_UINT,

		// Compressed (Vulkan backend only)
		BC1_RGB_UNORM, BC1_RGB_SRGB,
		BC3_UNORM, BC3_SRGB,
		BC4_UNORM, BC4_SNORM,
		BC5_UNORM, BC5_SNORM,
		BC6H_UFLOAT, BC6H_SFLOAT,
		BC7_UNORM, BC7_SRGB,
	};

	ENUM_CLASS_8(RHIQueryType) {
		Timestamp,
		PipelineStatistics
	};

	ENUM_CLASS_8(RHIDebugSeverity) {
		Info    = 0,
		Warning = 1,
		Error   = 2,
		Fatal   = 3,
	};

	ENUM_CLASS_32(RHIShaderStage) {
		None       = 0,
		Vertex     = 1 << 0,
		Fragment   = 1 << 1,
		Compute    = 1 << 2,
		RayGen     = 1 << 3,
		Miss       = 1 << 4,
		ClosestHit = 1 << 5,
		AnyHit     = 1 << 6,
		All        = 0xFFFFFFFF
	};

	ENUM_CLASS_8(RHIBindingType) {
		UniformBuffer  = 0,
		StorageBuffer  = 1,
		Texture        = 2,
		StorageTexture = 3,
		Sampler        = 4,
		AccelStructure = 5,
	};

	ENUM_CLASS_8(RHIFillMode) {
		Solid,
		Wireframe
	};

	ENUM_CLASS_8(RHICullMode) {
		None,
		Front,
		Back
	};

	ENUM_CLASS_8(RHICompareOp) {
		Never,
		Less,
		Equal,
		LessEqual,
		Greater,
		NotEqual,
		GreaterEqual,
		Always
	};

	ENUM_CLASS_8(RHIBlendFactor) {
		Zero,
		One,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha
	};

	ENUM_CLASS_8(RHIBlendOp) {
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};

	ENUM_CLASS_8(RHIVertexInputRate) {
		PerVertex,
		PerInstance
	};

	ENUM_CLASS_8(RHIPrimitiveTopology) {
		TriangleList,
		TriangleStrip,
		LineList,
		LineStrip,
		PointList
	};

	ENUM_CLASS_8(RHIASType) { BottomLevel, TopLevel };

	ENUM_CLASS_32(RHIASBuildFlags) {
		None            = 0,
		PreferFastTrace = 1 << 0,
		PreferFastBuild = 1 << 1,
		AllowCompaction = 1 << 2,
		AllowUpdate     = 1 << 3,
	};
}
