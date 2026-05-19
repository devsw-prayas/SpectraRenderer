#pragma once

#include "SpectraVulkanBackend.h"
#include "SpecVulkanDiagnostics.h"

namespace Spectra::Vulkan::Utils {
	inline constexpr uint32_t VK_SUPPORTED_EXT_COUNT = 27;
	inline constexpr uint32_t VK_SUPPORTED_LAYERS_COUNT = 1;
	inline constexpr uint32_t MAX_INSTANCE_EXT = 128;

	enum class SPEC_VK_BK_RUNTIME_API VulkanExtensions final : uint8_t {
		// Instance Extensions
		VK_SURFACE,
		VK_WIN32_SURFACE,
		VK_GET_PHYSICAL_DEVICE_PROPERTIES,
		VK_DEBUG_UTILS,

		//Device Extensions
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

		// RT Extensions
		VK_RAY_TRACING_PIPELINE,
		VK_ACCELERATION_STRUCTURE,
		VK_DEFERRED_HOST_OPERATIONS,
		VK_RAY_QUERY,
		VK_PIPELINE_LIBRARY,

		// Optional Extensions
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

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) VkExtension final {
		const char* m_Extension;
		VulkanExtensions       m_ExtensionName;
		ExtensionRequirement   m_Requirement;

		VkExtension() = default;

		VkExtension(const char* v_Ext, VulkanExtensions v_Name, ExtensionRequirement v_Req)
			: m_Extension(v_Ext), m_ExtensionName(v_Name), m_Requirement(v_Req) {
		}

		VkExtension(const VkExtension&) = default;
		VkExtension& operator=(const VkExtension&) = default;
		VkExtension(VkExtension&&) noexcept = default;
		VkExtension& operator=(VkExtension&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) VkLayer final {
		const char* m_Layer;
		VulkanLayers       m_LayerName;

		VkLayer() = default;

		VkLayer(const char* v_Ext, VulkanLayers v_Name)
			: m_Layer(v_Ext), m_LayerName(v_Name) {
		}

		VkLayer(const VkLayer&) = default;
		VkLayer& operator=(const VkLayer&) = default;
		VkLayer(VkLayer&&) noexcept = default;
		VkLayer& operator=(VkLayer&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(16) InitDesc final {
		const char* m_ApplicationName = nullptr;
		bool m_EnableValidation = false;

		InitDesc(const char* v_Name, bool v_Enabled)
			: m_ApplicationName(v_Name), m_EnableValidation(v_Enabled) {
		}
		~InitDesc() = default;

		InitDesc(const InitDesc&) = default;
		InitDesc& operator=(const InitDesc&) = default;

		InitDesc(InitDesc&&) noexcept = default;
		InitDesc& operator=(InitDesc&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) PhysicalDevice final {
		void* m_DeviceHandle;
		void* m_PropertiesHandle;

		bool isValid() const {
			return m_DeviceHandle != nullptr && m_PropertiesHandle != nullptr;
		}

		PhysicalDevice() : m_DeviceHandle(nullptr), m_PropertiesHandle(nullptr) {}

		PhysicalDevice(const PhysicalDevice&) = default;
		PhysicalDevice& operator=(const PhysicalDevice&) = default;

		PhysicalDevice(PhysicalDevice&&) noexcept = default;
		PhysicalDevice& operator=(PhysicalDevice&&) noexcept = default;
	};

	// Sync2 pipeline stage flags. Values match VkPipelineStageFlags2 — impl casts directly.
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

	// Sync2 memory access flags. Values match VkAccessFlags2 — impl casts directly.
	enum class AccessType : uint64_t {
		NONE                            = 0,
		INDIRECT_COMMAND_READ           = 0x0000000000000001ULL,
		INDEX_READ                      = 0x0000000000000002ULL,
		VERTEX_ATTRIBUTE_READ           = 0x0000000000000004ULL,
		UNIFORM_READ                    = 0x0000000000000008ULL,
		SHADER_READ                     = 0x0000000000000020ULL,
		SHADER_WRITE                    = 0x0000000000000040ULL,
		COLOR_ATTACHMENT_READ           = 0x0000000000000080ULL,
		COLOR_ATTACHMENT_WRITE          = 0x0000000000000100ULL,
		DEPTH_STENCIL_ATTACHMENT_READ   = 0x0000000000000200ULL,
		DEPTH_STENCIL_ATTACHMENT_WRITE  = 0x0000000000000400ULL,
		TRANSFER_READ                   = 0x0000000000000800ULL,
		TRANSFER_WRITE                  = 0x0000000000001000ULL,
		HOST_READ                       = 0x0000000000002000ULL,
		HOST_WRITE                      = 0x0000000000004000ULL,
		MEMORY_READ                     = 0x0000000000008000ULL,
		MEMORY_WRITE                    = 0x0000000000010000ULL,
		ACCELERATION_STRUCTURE_READ     = 0x0000000000200000ULL,
		ACCELERATION_STRUCTURE_WRITE    = 0x0000000000400000ULL,
		SHADER_SAMPLED_READ             = 0x0000000100000000ULL,
		SHADER_STORAGE_READ             = 0x0000000200000000ULL,
		SHADER_STORAGE_WRITE            = 0x0000000400000000ULL,
	};

	// Image layout. Values match VkImageLayout — impl casts directly.
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

	// Values match VkFormat — impl casts directly. Curated subset only.
	enum class Format : uint16_t {
		UNDEFINED                = 0,

		// 8-bit
		R8_UNORM                 = 9,

		// 8-bit packed RGBA
		R8G8B8A8_UNORM           = 37,
		R8G8B8A8_SRGB            = 43,
		B8G8R8A8_UNORM           = 44,
		B8G8R8A8_SRGB            = 50,

		// 16-bit float
		R16_SFLOAT               = 76,
		R16_UINT                 = 74,
		R16G16_SFLOAT            = 83,
		R16G16B16A16_SFLOAT      = 97,

		// 32-bit float / uint
		R32_UINT                 = 98,
		R32_SFLOAT               = 100,
		R32G32_UINT              = 101,
		R32G32_SFLOAT            = 103,
		R32G32B32_SFLOAT         = 106,
		R32G32B32A32_SFLOAT      = 109,

		// Depth / depth-stencil
		D16_UNORM                = 124,
		D32_SFLOAT               = 126,
		D24_UNORM_S8_UINT        = 129,
		D32_SFLOAT_S8_UINT       = 130,
	};

	// Values match VkBufferUsageFlagBits — impl casts directly.
	enum class BufferUsage : uint32_t {
		NONE                              = 0,
		TRANSFER_SRC                      = 0x00000001,
		TRANSFER_DST                      = 0x00000002,
		UNIFORM_BUFFER                    = 0x00000010,
		STORAGE_BUFFER                    = 0x00000020,
		INDEX_BUFFER                      = 0x00000040,
		VERTEX_BUFFER                     = 0x00000080,
		INDIRECT_BUFFER                   = 0x00000100,
		SHADER_BINDING_TABLE              = 0x00000400,
		SHADER_DEVICE_ADDRESS             = 0x00020000,
		ACCELERATION_STRUCTURE_BUILD_INPUT= 0x00080000,
		ACCELERATION_STRUCTURE_STORAGE    = 0x00100000,
	};

	// Values match VkImageUsageFlagBits — impl casts directly.
	enum class ImageUsage : uint32_t {
		NONE                      = 0,
		TRANSFER_SRC              = 0x00000001,
		TRANSFER_DST              = 0x00000002,
		SAMPLED                   = 0x00000004,
		STORAGE                   = 0x00000008,
		COLOR_ATTACHMENT          = 0x00000010,
		DEPTH_STENCIL_ATTACHMENT  = 0x00000020,
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
		SAMPLER                    = 0,
		COMBINED_IMAGE_SAMPLER     = 1,
		SAMPLED_IMAGE              = 2,
		STORAGE_IMAGE              = 3,
		UNIFORM_BUFFER             = 6,
		STORAGE_BUFFER             = 7,
		UNIFORM_BUFFER_DYNAMIC     = 8,
		STORAGE_BUFFER_DYNAMIC     = 9,
		ACCELERATION_STRUCTURE     = 1000150000,
	};

	// Values match VkShaderStageFlagBits — impl casts directly.
	enum class ShaderStage : uint32_t {
		NONE          = 0,
		VERTEX        = 0x00000001,
		FRAGMENT      = 0x00000010,
		COMPUTE       = 0x00000020,
		ALL_GRAPHICS  = 0x0000001F,
		RAYGEN        = 0x00000100,
		ANY_HIT       = 0x00000200,
		CLOSEST_HIT   = 0x00000400,
		MISS          = 0x00000800,
		INTERSECTION  = 0x00001000,
		CALLABLE      = 0x00002000,
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

	// Values match VkAccelerationStructureTypeKHR — impl casts directly.
	enum class AccelerationStructureType : uint8_t {
		TOP_LEVEL    = 0,
		BOTTOM_LEVEL = 1,
	};

	// Values match VkBuildAccelerationStructureFlagBitsKHR — impl casts directly.
	enum class AccelerationStructureBuildFlags : uint8_t {
		NONE               = 0,
		ALLOW_UPDATE       = 0x01,
		ALLOW_COMPACTION   = 0x02,
		PREFER_FAST_TRACE  = 0x04,
		PREFER_FAST_BUILD  = 0x08,
	};

	// Values match VkGeometryFlagBitsKHR — impl casts directly.
	enum class GeometryFlags : uint8_t {
		NONE                         = 0,
		OPAQUE                       = 0x01,
		NO_DUPLICATE_ANY_HIT         = 0x02,
	};

	// Values match VkQueryType — impl casts directly.
	enum class QueryType : uint8_t {
		TIMESTAMP           = 2,
		PIPELINE_STATISTICS = 1,
	};

	// Bitwise operators for flag enums (BufferUsage, ImageUsage, ShaderStage, ColorComponent, AccelerationStructureBuildFlags, GeometryFlags).
	SPEC_VK_BK_FORCEINLINE BufferUsage operator|(BufferUsage v_A, BufferUsage v_B) {
		return static_cast<BufferUsage>(static_cast<uint32_t>(v_A) | static_cast<uint32_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE BufferUsage operator&(BufferUsage v_A, BufferUsage v_B) {
		return static_cast<BufferUsage>(static_cast<uint32_t>(v_A) & static_cast<uint32_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE BufferUsage& operator|=(BufferUsage& v_A, BufferUsage v_B) {
		v_A = v_A | v_B; return v_A;
	}

	SPEC_VK_BK_FORCEINLINE ImageUsage operator|(ImageUsage v_A, ImageUsage v_B) {
		return static_cast<ImageUsage>(static_cast<uint32_t>(v_A) | static_cast<uint32_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE ImageUsage operator&(ImageUsage v_A, ImageUsage v_B) {
		return static_cast<ImageUsage>(static_cast<uint32_t>(v_A) & static_cast<uint32_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE ImageUsage& operator|=(ImageUsage& v_A, ImageUsage v_B) {
		v_A = v_A | v_B; return v_A;
	}

	SPEC_VK_BK_FORCEINLINE ShaderStage operator|(ShaderStage v_A, ShaderStage v_B) {
		return static_cast<ShaderStage>(static_cast<uint32_t>(v_A) | static_cast<uint32_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE ShaderStage operator&(ShaderStage v_A, ShaderStage v_B) {
		return static_cast<ShaderStage>(static_cast<uint32_t>(v_A) & static_cast<uint32_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE ShaderStage& operator|=(ShaderStage& v_A, ShaderStage v_B) {
		v_A = v_A | v_B; return v_A;
	}

	SPEC_VK_BK_FORCEINLINE ColorComponent operator|(ColorComponent v_A, ColorComponent v_B) {
		return static_cast<ColorComponent>(static_cast<uint8_t>(v_A) | static_cast<uint8_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE ColorComponent operator&(ColorComponent v_A, ColorComponent v_B) {
		return static_cast<ColorComponent>(static_cast<uint8_t>(v_A) & static_cast<uint8_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE ColorComponent& operator|=(ColorComponent& v_A, ColorComponent v_B) {
		v_A = v_A | v_B; return v_A;
	}

	SPEC_VK_BK_FORCEINLINE AccelerationStructureBuildFlags operator|(AccelerationStructureBuildFlags v_A, AccelerationStructureBuildFlags v_B) {
		return static_cast<AccelerationStructureBuildFlags>(static_cast<uint8_t>(v_A) | static_cast<uint8_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE AccelerationStructureBuildFlags operator&(AccelerationStructureBuildFlags v_A, AccelerationStructureBuildFlags v_B) {
		return static_cast<AccelerationStructureBuildFlags>(static_cast<uint8_t>(v_A) & static_cast<uint8_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE AccelerationStructureBuildFlags& operator|=(AccelerationStructureBuildFlags& v_A, AccelerationStructureBuildFlags v_B) {
		v_A = v_A | v_B; return v_A;
	}

	SPEC_VK_BK_FORCEINLINE GeometryFlags operator|(GeometryFlags v_A, GeometryFlags v_B) {
		return static_cast<GeometryFlags>(static_cast<uint8_t>(v_A) | static_cast<uint8_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE GeometryFlags operator&(GeometryFlags v_A, GeometryFlags v_B) {
		return static_cast<GeometryFlags>(static_cast<uint8_t>(v_A) & static_cast<uint8_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE GeometryFlags& operator|=(GeometryFlags& v_A, GeometryFlags v_B) {
		v_A = v_A | v_B; return v_A;
	}

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) FenceHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		FenceHandle() : m_Handle(nullptr) {}
		FenceHandle(const FenceHandle&) = default;
		FenceHandle& operator=(const FenceHandle&) = default;
		FenceHandle(FenceHandle&&) noexcept = default;
		FenceHandle& operator=(FenceHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) SemaphoreHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		SemaphoreHandle() : m_Handle(nullptr) {}
		SemaphoreHandle(const SemaphoreHandle&) = default;
		SemaphoreHandle& operator=(const SemaphoreHandle&) = default;
		SemaphoreHandle(SemaphoreHandle&&) noexcept = default;
		SemaphoreHandle& operator=(SemaphoreHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) CommandBufferHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		CommandBufferHandle() : m_Handle(nullptr) {}
		CommandBufferHandle(const CommandBufferHandle&) = default;
		CommandBufferHandle& operator=(const CommandBufferHandle&) = default;
		CommandBufferHandle(CommandBufferHandle&&) noexcept = default;
		CommandBufferHandle& operator=(CommandBufferHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(16) BufferHandle final {
		void* m_Handle;
		void* m_Allocation;

		bool isValid() const { return m_Handle != nullptr && m_Allocation != nullptr; }

		BufferHandle() : m_Handle(nullptr), m_Allocation(nullptr) {}
		BufferHandle(const BufferHandle&) = default;
		BufferHandle& operator=(const BufferHandle&) = default;
		BufferHandle(BufferHandle&&) noexcept = default;
		BufferHandle& operator=(BufferHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(16) ImageHandle final {
		void* m_Handle;
		void* m_Allocation;

		bool isValid() const { return m_Handle != nullptr && m_Allocation != nullptr; }

		ImageHandle() : m_Handle(nullptr), m_Allocation(nullptr) {}
		ImageHandle(const ImageHandle&) = default;
		ImageHandle& operator=(const ImageHandle&) = default;
		ImageHandle(ImageHandle&&) noexcept = default;
		ImageHandle& operator=(ImageHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) ImageViewHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		ImageViewHandle() : m_Handle(nullptr) {}
		ImageViewHandle(const ImageViewHandle&) = default;
		ImageViewHandle& operator=(const ImageViewHandle&) = default;
		ImageViewHandle(ImageViewHandle&&) noexcept = default;
		ImageViewHandle& operator=(ImageViewHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) SamplerHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		SamplerHandle() : m_Handle(nullptr) {}
		SamplerHandle(const SamplerHandle&) = default;
		SamplerHandle& operator=(const SamplerHandle&) = default;
		SamplerHandle(SamplerHandle&&) noexcept = default;
		SamplerHandle& operator=(SamplerHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) SurfaceHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		SurfaceHandle() : m_Handle(nullptr) {}
		SurfaceHandle(const SurfaceHandle&) = default;
		SurfaceHandle& operator=(const SurfaceHandle&) = default;
		SurfaceHandle(SurfaceHandle&&) noexcept = default;
		SurfaceHandle& operator=(SurfaceHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) SwapchainHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		SwapchainHandle() : m_Handle(nullptr) {}
		SwapchainHandle(const SwapchainHandle&) = default;
		SwapchainHandle& operator=(const SwapchainHandle&) = default;
		SwapchainHandle(SwapchainHandle&&) noexcept = default;
		SwapchainHandle& operator=(SwapchainHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) DescriptorSetLayoutHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		DescriptorSetLayoutHandle() : m_Handle(nullptr) {}
		DescriptorSetLayoutHandle(const DescriptorSetLayoutHandle&) = default;
		DescriptorSetLayoutHandle& operator=(const DescriptorSetLayoutHandle&) = default;
		DescriptorSetLayoutHandle(DescriptorSetLayoutHandle&&) noexcept = default;
		DescriptorSetLayoutHandle& operator=(DescriptorSetLayoutHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) DescriptorPoolHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		DescriptorPoolHandle() : m_Handle(nullptr) {}
		DescriptorPoolHandle(const DescriptorPoolHandle&) = default;
		DescriptorPoolHandle& operator=(const DescriptorPoolHandle&) = default;
		DescriptorPoolHandle(DescriptorPoolHandle&&) noexcept = default;
		DescriptorPoolHandle& operator=(DescriptorPoolHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) DescriptorSetHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		DescriptorSetHandle() : m_Handle(nullptr) {}
		DescriptorSetHandle(const DescriptorSetHandle&) = default;
		DescriptorSetHandle& operator=(const DescriptorSetHandle&) = default;
		DescriptorSetHandle(DescriptorSetHandle&&) noexcept = default;
		DescriptorSetHandle& operator=(DescriptorSetHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) PipelineLayoutHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		PipelineLayoutHandle() : m_Handle(nullptr) {}
		PipelineLayoutHandle(const PipelineLayoutHandle&) = default;
		PipelineLayoutHandle& operator=(const PipelineLayoutHandle&) = default;
		PipelineLayoutHandle(PipelineLayoutHandle&&) noexcept = default;
		PipelineLayoutHandle& operator=(PipelineLayoutHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) PipelineHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		PipelineHandle() : m_Handle(nullptr) {}
		PipelineHandle(const PipelineHandle&) = default;
		PipelineHandle& operator=(const PipelineHandle&) = default;
		PipelineHandle(PipelineHandle&&) noexcept = default;
		PipelineHandle& operator=(PipelineHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) ShaderModuleHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		ShaderModuleHandle() : m_Handle(nullptr) {}
		ShaderModuleHandle(const ShaderModuleHandle&) = default;
		ShaderModuleHandle& operator=(const ShaderModuleHandle&) = default;
		ShaderModuleHandle(ShaderModuleHandle&&) noexcept = default;
		ShaderModuleHandle& operator=(ShaderModuleHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) PipelineCacheHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		PipelineCacheHandle() : m_Handle(nullptr) {}
		PipelineCacheHandle(const PipelineCacheHandle&) = default;
		PipelineCacheHandle& operator=(const PipelineCacheHandle&) = default;
		PipelineCacheHandle(PipelineCacheHandle&&) noexcept = default;
		PipelineCacheHandle& operator=(PipelineCacheHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) AccelerationStructureHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		AccelerationStructureHandle() : m_Handle(nullptr) {}
		AccelerationStructureHandle(const AccelerationStructureHandle&) = default;
		AccelerationStructureHandle& operator=(const AccelerationStructureHandle&) = default;
		AccelerationStructureHandle(AccelerationStructureHandle&&) noexcept = default;
		AccelerationStructureHandle& operator=(AccelerationStructureHandle&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) QueryPoolHandle final {
		void* m_Handle;

		bool isValid() const { return m_Handle != nullptr; }

		QueryPoolHandle() : m_Handle(nullptr) {}
		QueryPoolHandle(const QueryPoolHandle&) = default;
		QueryPoolHandle& operator=(const QueryPoolHandle&) = default;
		QueryPoolHandle(QueryPoolHandle&&) noexcept = default;
		QueryPoolHandle& operator=(QueryPoolHandle&&) noexcept = default;
	};

	// Bitwise operators for flag enums.
	SPEC_VK_BK_FORCEINLINE PipelineStage operator|(PipelineStage v_A, PipelineStage v_B) {
		return static_cast<PipelineStage>(static_cast<uint64_t>(v_A) | static_cast<uint64_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE PipelineStage operator&(PipelineStage v_A, PipelineStage v_B) {
		return static_cast<PipelineStage>(static_cast<uint64_t>(v_A) & static_cast<uint64_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE PipelineStage operator~(PipelineStage v_A) {
		return static_cast<PipelineStage>(~static_cast<uint64_t>(v_A));
	}
	SPEC_VK_BK_FORCEINLINE PipelineStage& operator|=(PipelineStage& v_A, PipelineStage v_B) {
		v_A = v_A | v_B; return v_A;
	}

	SPEC_VK_BK_FORCEINLINE AccessType operator|(AccessType v_A, AccessType v_B) {
		return static_cast<AccessType>(static_cast<uint64_t>(v_A) | static_cast<uint64_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE AccessType operator&(AccessType v_A, AccessType v_B) {
		return static_cast<AccessType>(static_cast<uint64_t>(v_A) & static_cast<uint64_t>(v_B));
	}
	SPEC_VK_BK_FORCEINLINE AccessType operator~(AccessType v_A) {
		return static_cast<AccessType>(~static_cast<uint64_t>(v_A));
	}
	SPEC_VK_BK_FORCEINLINE AccessType& operator|=(AccessType& v_A, AccessType v_B) {
		v_A = v_A | v_B; return v_A;
	}
}
