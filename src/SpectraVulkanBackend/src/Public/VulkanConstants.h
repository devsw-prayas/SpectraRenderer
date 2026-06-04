#pragma once

#include "SpectraVulkanBackend.h"
#include "SpecVulkanDiagnostics.h"

namespace Spectra::Vulkan::Utils {

	inline constexpr uint32_t VK_SUPPORTED_EXT_COUNT    = 27;
	inline constexpr uint32_t VK_SUPPORTED_LAYERS_COUNT = 1;
	inline constexpr uint32_t MAX_INSTANCE_EXT          = 128;

	enum class SPEC_VK_BK_RUNTIME_API VulkanExtensions final : uint8_t {
		VK_SURFACE,
		VK_WIN32_SURFACE,
		VK_GET_PHYSICAL_DEVICE_PROPERTIES,
		VK_DEBUG_UTILS,

		VK_SWAPCHAIN,
		VK_SYNCHRONIZATION_2,
		VK_DYNAMIC_RENDERING,
		VK_BUFFER_DEVICE_ADDRESS,
		VK_DESCRIPTOR_INDEXING,
		VK_MAINTENANCE4,
		VK_TIMELINE_SEMAPHORE,
		VK_EXTERNAL_MEMORY,
		VK_EXTERNAL_MEMORY_WIN32,
		VK_EXTERNAL_SEMAPHORE,
		VK_EXTERNAL_SEMAPHORE_WIN32,
		VK_DEDICATED_ALLOCATION,
		VK_BIND_MEMORY2,
		VK_GET_MEMORY_REQUIREMENTS2,

		VK_RAY_TRACING_PIPELINE,
		VK_ACCELERATION_STRUCTURE,
		VK_DEFERRED_HOST_OPERATIONS,
		VK_RAY_QUERY,
		VK_PIPELINE_LIBRARY,

		VK_MEMORY_BUDGET,
		VK_SHADER_FLOAT16_INT8,
		VK_16BIT_STORAGE,
		VK_MESH_SHADER,

		NONE
	};

	enum class SPEC_VK_BK_RUNTIME_API VulkanLayers final : uint8_t {
		VK_VALIDATION,
		NONE
	};

	enum class SPEC_VK_BK_RUNTIME_API ExtensionRequirement final : uint8_t {
		REQUIRED,
		NON_ESSENTIAL
	};

	// Values match VkPipelineStageFlags2 — impl casts directly.
	enum class PipelineStage : uint64_t {
		NONE                         = 0,
		TOP_OF_PIPE                  = 0x0000000000000001ULL,
		DRAW_INDIRECT                = 0x0000000000000002ULL,
		VERTEX_SHADER                = 0x0000000000000008ULL,
		FRAGMENT_SHADER              = 0x0000000000000080ULL,
		EARLY_FRAGMENT_TESTS         = 0x0000000000000100ULL,
		LATE_FRAGMENT_TESTS          = 0x0000000000000200ULL,
		COLOR_ATTACHMENT_OUTPUT      = 0x0000000000000400ULL,
		COMPUTE_SHADER               = 0x0000000000000800ULL,
		ALL_TRANSFER                 = 0x0000000000001000ULL,
		BOTTOM_OF_PIPE               = 0x0000000000002000ULL,
		HOST                         = 0x0000000000004000ULL,
		ALL_GRAPHICS                 = 0x0000000000008000ULL,
		ALL_COMMANDS                 = 0x0000000000010000ULL,
		RAY_TRACING_SHADER           = 0x0000000000200000ULL,
		ACCELERATION_STRUCTURE_BUILD = 0x0000000002000000ULL,
		COPY                         = 0x0000000100000000ULL,
		RESOLVE                      = 0x0000000200000000ULL,
		BLIT                         = 0x0000000400000000ULL,
		CLEAR                        = 0x0000000800000000ULL,
		INDEX_INPUT                  = 0x0000001000000000ULL,
		VERTEX_ATTRIBUTE_INPUT       = 0x0000002000000000ULL,
		PRE_RASTERIZATION_SHADERS    = 0x0000004000000000ULL,
	};

	// Values match VkAccessFlags2 — impl casts directly.
	enum class AccessType : uint64_t {
		NONE                           = 0,
		INDIRECT_COMMAND_READ          = 0x0000000000000001ULL,
		INDEX_READ                     = 0x0000000000000002ULL,
		VERTEX_ATTRIBUTE_READ          = 0x0000000000000004ULL,
		UNIFORM_READ                   = 0x0000000000000008ULL,
		SHADER_READ                    = 0x0000000000000020ULL,
		SHADER_WRITE                   = 0x0000000000000040ULL,
		COLOR_ATTACHMENT_READ          = 0x0000000000000080ULL,
		COLOR_ATTACHMENT_WRITE         = 0x0000000000000100ULL,
		DEPTH_STENCIL_ATTACHMENT_READ  = 0x0000000000000200ULL,
		DEPTH_STENCIL_ATTACHMENT_WRITE = 0x0000000000000400ULL,
		TRANSFER_READ                  = 0x0000000000000800ULL,
		TRANSFER_WRITE                 = 0x0000000000001000ULL,
		HOST_READ                      = 0x0000000000002000ULL,
		HOST_WRITE                     = 0x0000000000004000ULL,
		MEMORY_READ                    = 0x0000000000008000ULL,
		MEMORY_WRITE                   = 0x0000000000010000ULL,
		ACCELERATION_STRUCTURE_READ    = 0x0000000000200000ULL,
		ACCELERATION_STRUCTURE_WRITE   = 0x0000000000400000ULL,
		SHADER_SAMPLED_READ            = 0x0000000100000000ULL,
		SHADER_STORAGE_READ            = 0x0000000200000000ULL,
		SHADER_STORAGE_WRITE           = 0x0000000400000000ULL,
	};

	// Values match VkImageLayout — impl casts directly.
	enum class ImageLayout : uint32_t {
		UNDEFINED                = 0,
		GENERAL                  = 1,
		COLOR_ATTACHMENT         = 2,
		DEPTH_STENCIL_ATTACHMENT = 3,
		DEPTH_STENCIL_READ_ONLY  = 4,
		SHADER_READ_ONLY         = 5,
		TRANSFER_SRC             = 6,
		TRANSFER_DST             = 7,
		PREINITIALIZED           = 8,
		PRESENT_SRC              = 1000001002,
		READ_ONLY                = 1000314000,
		ATTACHMENT               = 1000314001,
	};

	enum class QueueType : uint8_t {
		GRAPHICS,
		COMPUTE,
		TRANSFER,
		NONE
	};

	// Values match VkPipelineBindPoint — impl casts directly.
	enum class PipelineBindPoint : uint32_t {
		GRAPHICS    = 0,
		COMPUTE     = 1,
		RAY_TRACING = 1000165000,
	};

	// Values match VkFormat — impl casts directly. Curated subset.
	enum class Format : uint16_t {
		UNDEFINED           = 0,
		R8_UNORM            = 9,
		R8G8B8A8_UNORM      = 37,
		R8G8B8A8_SRGB       = 43,
		B8G8R8A8_UNORM      = 44,
		B8G8R8A8_SRGB       = 50,
		R16_UINT            = 74,
		R16_SFLOAT          = 76,
		R16G16_SFLOAT       = 83,
		R16G16B16A16_SFLOAT = 97,
		R32_UINT            = 98,
		R32_SFLOAT          = 100,
		R32G32_UINT         = 101,
		R32G32_SFLOAT       = 103,
		R32G32B32_SFLOAT    = 106,
		R32G32B32A32_SFLOAT = 109,
		D16_UNORM           = 124,
		D32_SFLOAT          = 126,
		D24_UNORM_S8_UINT   = 129,
		D32_SFLOAT_S8_UINT  = 130,
	};

	// Values match VkBufferUsageFlagBits — impl casts directly.
	enum class BufferUsage : uint32_t {
		NONE                               = 0,
		TRANSFER_SRC                       = 0x00000001,
		TRANSFER_DST                       = 0x00000002,
		UNIFORM_BUFFER                     = 0x00000010,
		STORAGE_BUFFER                     = 0x00000020,
		INDEX_BUFFER                       = 0x00000040,
		VERTEX_BUFFER                      = 0x00000080,
		INDIRECT_BUFFER                    = 0x00000100,
		SHADER_BINDING_TABLE               = 0x00000400,
		SHADER_DEVICE_ADDRESS              = 0x00020000,
		ACCELERATION_STRUCTURE_BUILD_INPUT = 0x00080000,
		ACCELERATION_STRUCTURE_STORAGE     = 0x00100000,
	};

	// Values match VkImageUsageFlagBits — impl casts directly.
	enum class ImageUsage : uint32_t {
		NONE                     = 0,
		TRANSFER_SRC             = 0x00000001,
		TRANSFER_DST             = 0x00000002,
		SAMPLED                  = 0x00000004,
		STORAGE                  = 0x00000008,
		COLOR_ATTACHMENT         = 0x00000010,
		DEPTH_STENCIL_ATTACHMENT = 0x00000020,
	};

	// Values match VkImageAspectFlagBits — impl casts directly.
	enum class ImageAspect : uint8_t {
		NONE    = 0,
		COLOR   = 0x01,
		DEPTH   = 0x02,
		STENCIL = 0x04,
	};

	// Values match VkImageType — impl casts directly.
	enum class ImageType : uint8_t {
		D1 = 0,
		D2 = 1,
		D3 = 2,
	};

	// Values match VkImageViewType — impl casts directly.
	enum class ImageViewType : uint8_t {
		D1         = 0,
		D2         = 1,
		D3         = 2,
		CUBE       = 3,
		D1_ARRAY   = 4,
		D2_ARRAY   = 5,
		CUBE_ARRAY = 6,
	};

	// Values match VkSampleCountFlagBits — impl casts directly.
	enum class SampleCount : uint8_t {
		x1  = 0x01,
		x2  = 0x02,
		x4  = 0x04,
		x8  = 0x08,
		x16 = 0x10,
		x32 = 0x20,
		x64 = 0x40,
	};

	// Values match VkFilter — impl casts directly.
	enum class FilterMode : uint8_t {
		NEAREST = 0,
		LINEAR  = 1,
	};

	// Values match VkSamplerMipmapMode — impl casts directly.
	enum class MipmapMode : uint8_t {
		NEAREST = 0,
		LINEAR  = 1,
	};

	// Values match VkSamplerAddressMode — impl casts directly.
	enum class AddressMode : uint8_t {
		REPEAT               = 0,
		MIRRORED_REPEAT      = 1,
		CLAMP_TO_EDGE        = 2,
		CLAMP_TO_BORDER      = 3,
		MIRROR_CLAMP_TO_EDGE = 4,
	};

	// Values match VkCompareOp — impl casts directly.
	enum class CompareOp : uint8_t {
		NEVER            = 0,
		LESS             = 1,
		EQUAL            = 2,
		LESS_OR_EQUAL    = 3,
		GREATER          = 4,
		NOT_EQUAL        = 5,
		GREATER_OR_EQUAL = 6,
		ALWAYS           = 7,
	};

	// Values match VkBorderColor — impl casts directly.
	enum class BorderColor : uint8_t {
		FLOAT_TRANSPARENT_BLACK = 0,
		INT_TRANSPARENT_BLACK   = 1,
		FLOAT_OPAQUE_BLACK      = 2,
		INT_OPAQUE_BLACK        = 3,
		FLOAT_OPAQUE_WHITE      = 4,
		INT_OPAQUE_WHITE        = 5,
	};

	// Values match VkDescriptorType — impl casts directly.
	enum class DescriptorType : uint32_t {
		SAMPLER                = 0,
		COMBINED_IMAGE_SAMPLER = 1,
		SAMPLED_IMAGE          = 2,
		STORAGE_IMAGE          = 3,
		UNIFORM_BUFFER         = 6,
		STORAGE_BUFFER         = 7,
		UNIFORM_BUFFER_DYNAMIC = 8,
		STORAGE_BUFFER_DYNAMIC = 9,
		ACCELERATION_STRUCTURE = 1000150000,
	};

	// Values match VkShaderStageFlagBits — impl casts directly.
	enum class ShaderStage : uint32_t {
		NONE         = 0,
		VERTEX       = 0x00000001,
		FRAGMENT     = 0x00000010,
		COMPUTE      = 0x00000020,
		ALL_GRAPHICS = 0x0000001F,
		RAYGEN       = 0x00000100,
		ANY_HIT      = 0x00000200,
		CLOSEST_HIT  = 0x00000400,
		MISS         = 0x00000800,
		INTERSECTION = 0x00001000,
		CALLABLE     = 0x00002000,
	};

	// Values match VkPrimitiveTopology — impl casts directly.
	enum class PrimitiveTopology : uint8_t {
		POINT_LIST     = 0,
		LINE_LIST      = 1,
		LINE_STRIP     = 2,
		TRIANGLE_LIST  = 3,
		TRIANGLE_STRIP = 4,
		TRIANGLE_FAN   = 5,
	};

	// Values match VkPolygonMode — impl casts directly.
	enum class PolygonMode : uint8_t {
		FILL  = 0,
		LINE  = 1,
		POINT = 2,
	};

	// Values match VkCullModeFlagBits — impl casts directly.
	enum class CullMode : uint8_t {
		NONE           = 0,
		FRONT          = 0x01,
		BACK           = 0x02,
		FRONT_AND_BACK = 0x03,
	};

	// Values match VkFrontFace — impl casts directly.
	enum class FrontFace : uint8_t {
		CCW = 0,
		CW  = 1,
	};

	// Values match VkBlendFactor — impl casts directly.
	enum class BlendFactor : uint8_t {
		ZERO                     = 0,
		ONE                      = 1,
		SRC_COLOR                = 2,
		ONE_MINUS_SRC_COLOR      = 3,
		DST_COLOR                = 4,
		ONE_MINUS_DST_COLOR      = 5,
		SRC_ALPHA                = 6,
		ONE_MINUS_SRC_ALPHA      = 7,
		DST_ALPHA                = 8,
		ONE_MINUS_DST_ALPHA      = 9,
		CONSTANT_COLOR           = 10,
		ONE_MINUS_CONSTANT_COLOR = 11,
		CONSTANT_ALPHA           = 12,
		ONE_MINUS_CONSTANT_ALPHA = 13,
		SRC_ALPHA_SATURATE       = 14,
	};

	// Values match VkBlendOp — impl casts directly.
	enum class BlendOp : uint8_t {
		ADD              = 0,
		SUBTRACT         = 1,
		REVERSE_SUBTRACT = 2,
		MIN              = 3,
		MAX              = 4,
	};

	// Values match VkColorComponentFlagBits — impl casts directly.
	enum class ColorComponent : uint8_t {
		NONE = 0,
		R    = 0x01,
		G    = 0x02,
		B    = 0x04,
		A    = 0x08,
		ALL  = 0x0F,
	};

	// Values match VkStencilOp — impl casts directly.
	enum class StencilOp : uint8_t {
		KEEP                = 0,
		ZERO                = 1,
		REPLACE             = 2,
		INCREMENT_AND_CLAMP = 3,
		DECREMENT_AND_CLAMP = 4,
		INVERT              = 5,
		INCREMENT_AND_WRAP  = 6,
		DECREMENT_AND_WRAP  = 7,
	};

	// Values match VkVertexInputRate — impl casts directly.
	enum class VertexInputRate : uint8_t {
		VERTEX   = 0,
		INSTANCE = 1,
	};

	// Values match VkIndexType — impl casts directly.
	enum class IndexType : uint8_t {
		UINT16 = 0,
		UINT32 = 1,
	};

	// Values match VkAttachmentLoadOp — impl casts directly.
	enum class LoadOp : uint8_t {
		LOAD      = 0,
		CLEAR     = 1,
		DONT_CARE = 2,
	};

	// Values match VkAttachmentStoreOp — impl casts directly.
	enum class StoreOp : uint8_t {
		STORE     = 0,
		DONT_CARE = 1,
	};

	// Values match VkPresentModeKHR — impl casts directly.
	enum class PresentMode : uint8_t {
		IMMEDIATE    = 0,
		MAILBOX      = 1,
		FIFO         = 2,
		FIFO_RELAXED = 3,
	};

	// Values match VkColorSpaceKHR — impl casts directly. Curated subset.
	enum class ColorSpace : uint32_t {
		SRGB_NONLINEAR = 0,
		HDR10_ST2084   = 1000104002,
		BT2020_LINEAR  = 1000104005,
	};

	// Swapchain acquire/present status — does not alias VkResult values.
	enum class SwapchainStatus : uint8_t {
		OK          = 0,
		SUBOPTIMAL  = 1,
		OUT_OF_DATE = 2,
		ERROR       = 3,
	};

	// Values match VkAccelerationStructureTypeKHR — impl casts directly.
	enum class AccelerationStructureType : uint8_t {
		TOP_LEVEL    = 0,
		BOTTOM_LEVEL = 1,
	};

	// Values match VkBuildAccelerationStructureFlagBitsKHR — impl casts directly.
	enum class AccelerationStructureBuildFlags : uint8_t {
		NONE              = 0,
		ALLOW_UPDATE      = 0x01,
		ALLOW_COMPACTION  = 0x02,
		PREFER_FAST_TRACE = 0x04,
		PREFER_FAST_BUILD = 0x08,
	};

	// Values match VkGeometryFlagBitsKHR — impl casts directly.
	enum class GeometryFlags : uint8_t {
		NONE                 = 0,
		OPAQUE               = 0x01,
		NO_DUPLICATE_ANY_HIT = 0x02,
	};

	// Values match VkRayTracingShaderGroupTypeKHR — impl casts directly.
	enum class ShaderGroupType : uint8_t {
		GENERAL        = 0,
		TRIANGLES_HIT  = 1,
		PROCEDURAL_HIT = 2,
	};

	// Values match VkQueryType — impl casts directly.
	enum class QueryType : uint32_t {
		PIPELINE_STATISTICS                   = 1,
		TIMESTAMP                             = 2,
		ACCELERATION_STRUCTURE_COMPACTED_SIZE = 1000150000,
	};

	using PFN_VkAlloc         = void*(SPEC_VK_BK_API_PTR*)(void* pUserData, size_t size, size_t alignment, uint32_t scope);
	using PFN_VkRealloc       = void*(SPEC_VK_BK_API_PTR*)(void* pUserData, void* pOriginal, size_t size, size_t alignment, uint32_t scope);
	using PFN_VkFree          = void (SPEC_VK_BK_API_PTR*)(void* pUserData, void* pMemory);
	using PFN_VkInternalAlloc = void (SPEC_VK_BK_API_PTR*)(void* pUserData, size_t size, uint32_t type, uint32_t scope);
	using PFN_VkInternalFree  = void (SPEC_VK_BK_API_PTR*)(void* pUserData, size_t size, uint32_t type, uint32_t scope);

	// Values match VkSystemAllocationScope — impl casts directly.
	enum class AllocationScope : uint32_t {
		COMMAND  = 0,
		OBJECT   = 1,
		CACHE    = 2,
		DEVICE   = 3,
		INSTANCE = 4,
	};

	// Values match VkInternalAllocationType — impl casts directly.
	enum class InternalAllocationType : uint32_t {
		EXECUTABLE = 0,
	};


	// -------------------------------------------------------------------------
	// Bitwise operators — flag enums only
	// -------------------------------------------------------------------------

	SPEC_VK_BK_FORCEINLINE PipelineStage  operator| (PipelineStage  v_A, PipelineStage  v_B) { return static_cast<PipelineStage> (static_cast<uint64_t>(v_A) | static_cast<uint64_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE PipelineStage  operator& (PipelineStage  v_A, PipelineStage  v_B) { return static_cast<PipelineStage> (static_cast<uint64_t>(v_A) & static_cast<uint64_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE PipelineStage  operator~ (PipelineStage  v_A)                     { return static_cast<PipelineStage> (~static_cast<uint64_t>(v_A)); }
	SPEC_VK_BK_FORCEINLINE PipelineStage& operator|=(PipelineStage& v_A, PipelineStage  v_B) { v_A = v_A | v_B; return v_A; }

	SPEC_VK_BK_FORCEINLINE AccessType  operator| (AccessType  v_A, AccessType  v_B) { return static_cast<AccessType> (static_cast<uint64_t>(v_A) | static_cast<uint64_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE AccessType  operator& (AccessType  v_A, AccessType  v_B) { return static_cast<AccessType> (static_cast<uint64_t>(v_A) & static_cast<uint64_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE AccessType  operator~ (AccessType  v_A)                  { return static_cast<AccessType> (~static_cast<uint64_t>(v_A)); }
	SPEC_VK_BK_FORCEINLINE AccessType& operator|=(AccessType& v_A, AccessType  v_B) { v_A = v_A | v_B; return v_A; }

	SPEC_VK_BK_FORCEINLINE BufferUsage  operator| (BufferUsage  v_A, BufferUsage  v_B) { return static_cast<BufferUsage> (static_cast<uint32_t>(v_A) | static_cast<uint32_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE BufferUsage  operator& (BufferUsage  v_A, BufferUsage  v_B) { return static_cast<BufferUsage> (static_cast<uint32_t>(v_A) & static_cast<uint32_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE BufferUsage& operator|=(BufferUsage& v_A, BufferUsage  v_B) { v_A = v_A | v_B; return v_A; }

	SPEC_VK_BK_FORCEINLINE ImageUsage  operator| (ImageUsage  v_A, ImageUsage  v_B) { return static_cast<ImageUsage> (static_cast<uint32_t>(v_A) | static_cast<uint32_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE ImageUsage  operator& (ImageUsage  v_A, ImageUsage  v_B) { return static_cast<ImageUsage> (static_cast<uint32_t>(v_A) & static_cast<uint32_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE ImageUsage& operator|=(ImageUsage& v_A, ImageUsage  v_B) { v_A = v_A | v_B; return v_A; }

	SPEC_VK_BK_FORCEINLINE ImageAspect  operator| (ImageAspect  v_A, ImageAspect  v_B) { return static_cast<ImageAspect> (static_cast<uint8_t>(v_A) | static_cast<uint8_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE ImageAspect  operator& (ImageAspect  v_A, ImageAspect  v_B) { return static_cast<ImageAspect> (static_cast<uint8_t>(v_A) & static_cast<uint8_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE ImageAspect& operator|=(ImageAspect& v_A, ImageAspect  v_B) { v_A = v_A | v_B; return v_A; }

	SPEC_VK_BK_FORCEINLINE ShaderStage  operator| (ShaderStage  v_A, ShaderStage  v_B) { return static_cast<ShaderStage> (static_cast<uint32_t>(v_A) | static_cast<uint32_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE ShaderStage  operator& (ShaderStage  v_A, ShaderStage  v_B) { return static_cast<ShaderStage> (static_cast<uint32_t>(v_A) & static_cast<uint32_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE ShaderStage& operator|=(ShaderStage& v_A, ShaderStage  v_B) { v_A = v_A | v_B; return v_A; }

	SPEC_VK_BK_FORCEINLINE ColorComponent  operator| (ColorComponent  v_A, ColorComponent  v_B) { return static_cast<ColorComponent> (static_cast<uint8_t>(v_A) | static_cast<uint8_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE ColorComponent  operator& (ColorComponent  v_A, ColorComponent  v_B) { return static_cast<ColorComponent> (static_cast<uint8_t>(v_A) & static_cast<uint8_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE ColorComponent& operator|=(ColorComponent& v_A, ColorComponent  v_B) { v_A = v_A | v_B; return v_A; }

	SPEC_VK_BK_FORCEINLINE AccelerationStructureBuildFlags  operator| (AccelerationStructureBuildFlags  v_A, AccelerationStructureBuildFlags  v_B) { return static_cast<AccelerationStructureBuildFlags>(static_cast<uint8_t>(v_A) | static_cast<uint8_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE AccelerationStructureBuildFlags  operator& (AccelerationStructureBuildFlags  v_A, AccelerationStructureBuildFlags  v_B) { return static_cast<AccelerationStructureBuildFlags>(static_cast<uint8_t>(v_A) & static_cast<uint8_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE AccelerationStructureBuildFlags& operator|=(AccelerationStructureBuildFlags& v_A, AccelerationStructureBuildFlags  v_B) { v_A = v_A | v_B; return v_A; }

	SPEC_VK_BK_FORCEINLINE GeometryFlags  operator| (GeometryFlags  v_A, GeometryFlags  v_B) { return static_cast<GeometryFlags>(static_cast<uint8_t>(v_A) | static_cast<uint8_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE GeometryFlags  operator& (GeometryFlags  v_A, GeometryFlags  v_B) { return static_cast<GeometryFlags>(static_cast<uint8_t>(v_A) & static_cast<uint8_t>(v_B)); }
	SPEC_VK_BK_FORCEINLINE GeometryFlags& operator|=(GeometryFlags& v_A, GeometryFlags  v_B) { v_A = v_A | v_B; return v_A; }
}
