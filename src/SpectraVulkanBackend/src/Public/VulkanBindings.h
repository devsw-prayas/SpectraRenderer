#pragma once

#include "VulkanHandles.h"

namespace Spectra::Vulkan::Utils {
	// ClearValue

	struct SPEC_VK_BK_ALIGNAS(4) ClearValue final {
		union {
			float m_Color[4];
			struct {
				float   m_Depth;
				uint8_t m_Stencil;
			} m_DepthStencil;
		};
	};

	// Registry helpers (used internally by VulkanInternalHelpers)

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) VkExtension final {
		const char* m_Extension;
		VulkanExtensions     m_ExtensionName;
		ExtensionRequirement m_Requirement;

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
		VulkanLayers m_LayerName;

		VkLayer() = default;
		VkLayer(const char* v_Layer, VulkanLayers v_Name)
			: m_Layer(v_Layer), m_LayerName(v_Name) {
		}
		VkLayer(const VkLayer&) = default;
		VkLayer& operator=(const VkLayer&) = default;
		VkLayer(VkLayer&&) noexcept = default;
		VkLayer& operator=(VkLayer&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) InitDesc final {
		const char* m_ApplicationName = nullptr;
		bool        m_EnableValidation = false;

		InitDesc() = default;
		InitDesc(const char* v_Name, bool v_Enabled)
			: m_ApplicationName(v_Name), m_EnableValidation(v_Enabled) {
		}
		InitDesc(const InitDesc&) = default;
		InitDesc& operator=(const InitDesc&) = default;
		InitDesc(InitDesc&&) noexcept = default;
		InitDesc& operator=(InitDesc&&) noexcept = default;
	};

	// Resource descs

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) BufferDesc final {
		uint64_t    m_Size = 0;
		BufferUsage m_Usage = BufferUsage::NONE;
		bool        m_Dedicated = false;
		bool        m_HostVisible = false;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(4) ImageDesc final {
		ImageType   m_Type = ImageType::D2;
		Format      m_Format = Format::UNDEFINED;
		uint32_t    m_Width = 0;
		uint32_t    m_Height = 0;
		uint32_t    m_Depth = 1;
		uint32_t    m_MipLevels = 1;
		uint32_t    m_ArrayLayers = 1;
		SampleCount m_Samples = SampleCount::x1;
		ImageUsage  m_Usage = ImageUsage::NONE;
		bool        m_Dedicated = false;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(16) ImageViewDesc final {
		ImageHandle   m_Image = {};
		ImageViewType m_ViewType = ImageViewType::D2;
		Format        m_Format = Format::UNDEFINED;
		ImageAspect   m_AspectMask = ImageAspect::COLOR;
		uint32_t      m_BaseMip = 0;
		uint32_t      m_MipCount = 1;
		uint32_t      m_BaseLayer = 0;
		uint32_t      m_LayerCount = 1;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(4) SamplerDesc final {
		FilterMode  m_MagFilter = FilterMode::LINEAR;
		FilterMode  m_MinFilter = FilterMode::LINEAR;
		MipmapMode  m_MipmapMode = MipmapMode::LINEAR;
		AddressMode m_AddressModeU = AddressMode::CLAMP_TO_EDGE;
		AddressMode m_AddressModeV = AddressMode::CLAMP_TO_EDGE;
		AddressMode m_AddressModeW = AddressMode::CLAMP_TO_EDGE;
		bool        m_AnisotropyEnable = false;
		float       m_MaxAnisotropy = 1.0f;
		bool        m_CompareEnable = false;
		CompareOp   m_CompareOp = CompareOp::ALWAYS;
		float       m_MinLod = 0.0f;
		float       m_MaxLod = 1000.0f;
		BorderColor m_BorderColor = BorderColor::FLOAT_TRANSPARENT_BLACK;
	};

	// Swapchain desc

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) SwapchainDesc final {
		SurfaceHandle   m_Surface       = {};
		uint32_t        m_MinImageCount = 2;
		Format          m_Format        = Format::B8G8R8A8_SRGB;
		ColorSpace      m_ColorSpace    = ColorSpace::SRGB_NONLINEAR;
		uint32_t        m_Width         = 0;
		uint32_t        m_Height        = 0;
		ImageUsage      m_Usage         = ImageUsage::COLOR_ATTACHMENT;
		PresentMode     m_PresentMode   = PresentMode::FIFO;
		SwapchainHandle m_OldSwapchain  = {};
	};

	// Descriptor descs

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(4) DescriptorSetLayoutBinding final {
		uint32_t       m_Binding = 0;
		DescriptorType m_Type = DescriptorType::UNIFORM_BUFFER;
		uint32_t       m_Count = 1;
		ShaderStage    m_StageFlags = ShaderStage::NONE;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(4) DescriptorPoolSize final {
		DescriptorType m_Type = DescriptorType::UNIFORM_BUFFER;
		uint32_t       m_Count = 0;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(16) DescriptorBufferInfo final {
		BufferHandle m_Buffer = {};
		uint64_t     m_Offset = 0;
		uint64_t     m_Range = ~0ULL;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) DescriptorImageInfo final {
		SamplerHandle   m_Sampler = {};
		ImageViewHandle m_ImageView = {};
		ImageLayout     m_Layout = ImageLayout::SHADER_READ_ONLY;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) WriteDescriptorSet final {
		DescriptorSetHandle               m_DstSet = {};
		uint32_t                          m_Binding = 0;
		uint32_t                          m_ArrayElement = 0;
		uint32_t                          m_Count = 1;
		DescriptorType                    m_Type = DescriptorType::UNIFORM_BUFFER;
		const DescriptorBufferInfo* m_pBufferInfo = nullptr;
		const DescriptorImageInfo* m_pImageInfo = nullptr;
		const AccelerationStructureHandle* m_pAccelerationStructures = nullptr;
	};

	// Pipeline building blocks

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(4) PushConstantRange final {
		ShaderStage m_StageFlags = ShaderStage::NONE;
		uint32_t    m_Offset = 0;
		uint32_t    m_Size = 0;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(4) VertexInputBinding final {
		uint32_t       m_Binding = 0;
		uint32_t       m_Stride = 0;
		VertexInputRate m_InputRate = VertexInputRate::VERTEX;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(4) VertexInputAttribute final {
		uint32_t m_Location = 0;
		uint32_t m_Binding = 0;
		Format   m_Format = Format::UNDEFINED;
		uint32_t m_Offset = 0;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(4) ColorBlendAttachment final {
		bool           m_BlendEnable = false;
		BlendFactor    m_SrcColor = BlendFactor::ONE;
		BlendFactor    m_DstColor = BlendFactor::ZERO;
		BlendOp        m_ColorOp = BlendOp::ADD;
		BlendFactor    m_SrcAlpha = BlendFactor::ONE;
		BlendFactor    m_DstAlpha = BlendFactor::ZERO;
		BlendOp        m_AlphaOp = BlendOp::ADD;
		ColorComponent m_WriteMask = ColorComponent::ALL;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(4) DepthStencilDesc final {
		bool      m_DepthTestEnable = false;
		bool      m_DepthWriteEnable = false;
		CompareOp m_DepthCompareOp = CompareOp::LESS;
		bool      m_StencilTestEnable = false;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(4) RasterizationDesc final {
		PolygonMode m_PolygonMode = PolygonMode::FILL;
		CullMode    m_CullMode = CullMode::BACK;
		FrontFace   m_FrontFace = FrontFace::CCW;
		bool        m_DepthBiasEnable = false;
		float       m_DepthBiasConstant = 0.0f;
		float       m_DepthBiasSlope = 0.0f;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) ShaderStageDesc final {
		ShaderStage       m_Stage = ShaderStage::NONE;
		ShaderModuleHandle m_Module = {};
		const char* m_EntryPoint = "main";
	};

	// Pipeline descs

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) GraphicsPipelineDesc final {
		const ShaderStageDesc* m_Stages = nullptr;
		uint32_t                    m_StageCount = 0;
		const VertexInputBinding* m_VertexBindings = nullptr;
		uint32_t                    m_VertexBindingCount = 0;
		const VertexInputAttribute* m_VertexAttributes = nullptr;
		uint32_t                    m_VertexAttributeCount = 0;
		PrimitiveTopology           m_Topology = PrimitiveTopology::TRIANGLE_LIST;
		RasterizationDesc           m_Rasterization = {};
		DepthStencilDesc            m_DepthStencil = {};
		const ColorBlendAttachment* m_BlendAttachments = nullptr;
		uint32_t                    m_BlendAttachmentCount = 0;
		const Format* m_ColorFormats = nullptr;
		uint32_t                    m_ColorFormatCount = 0;
		Format                      m_DepthFormat = Format::UNDEFINED;
		PipelineLayoutHandle        m_Layout = {};
		PipelineCacheHandle         m_Cache = {};
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) ComputePipelineDesc final {
		ShaderStageDesc      m_Stage = {};
		PipelineLayoutHandle m_Layout = {};
		PipelineCacheHandle  m_Cache = {};
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(4) RayTracingShaderGroup final {
		ShaderGroupType m_Type = ShaderGroupType::GENERAL;
		uint32_t        m_GeneralShader = ~0u;
		uint32_t        m_ClosestHitShader = ~0u;
		uint32_t        m_AnyHitShader = ~0u;
		uint32_t        m_IntersectionShader = ~0u;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) RayTracingPipelineDesc final {
		const ShaderStageDesc* m_Stages = nullptr;
		uint32_t                    m_StageCount = 0;
		const RayTracingShaderGroup* m_Groups = nullptr;
		uint32_t                    m_GroupCount = 0;
		uint32_t                    m_MaxRecursionDepth = 1;
		PipelineLayoutHandle        m_Layout = {};
		PipelineCacheHandle         m_Cache = {};
	};

	// Dynamic rendering descs

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) RenderingAttachmentDesc final {
		ImageViewHandle m_ImageView = {};
		ImageLayout     m_Layout = ImageLayout::COLOR_ATTACHMENT;
		LoadOp          m_LoadOp = LoadOp::CLEAR;
		StoreOp         m_StoreOp = StoreOp::STORE;
		ClearValue      m_ClearValue = {};
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) RenderingDesc final {
		int32_t                        m_OffsetX = 0;
		int32_t                        m_OffsetY = 0;
		uint32_t                       m_Width = 0;
		uint32_t                       m_Height = 0;
		const RenderingAttachmentDesc* m_ColorAttachments = nullptr;
		uint32_t                       m_ColorAttachmentCount = 0;
		const RenderingAttachmentDesc* m_DepthAttachment = nullptr;
	};

	// Acceleration structure descs

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) ASGeometryTriangles final {
		Format    m_VertexFormat        = Format::R32G32B32_SFLOAT;
		uint64_t  m_VertexDeviceAddress = 0;
		uint32_t  m_VertexStride        = 0;
		uint32_t  m_MaxVertex           = 0;
		IndexType m_IndexType           = IndexType::UINT32;
		uint64_t  m_IndexDeviceAddress  = 0;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) ASGeometryInstances final {
		uint64_t m_InstancesDeviceAddress = 0;
		bool     m_ArrayOfPointers        = false;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) ASBuildSizes final {
		uint64_t m_AccelStructureSize = 0;
		uint64_t m_ScratchSize        = 0;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(16) ASBuildDesc final {
		AccelerationStructureType           m_Type              = AccelerationStructureType::BOTTOM_LEVEL;
		AccelerationStructureBuildFlags     m_Flags             = AccelerationStructureBuildFlags::NONE;
		const ASGeometryTriangles*          m_Geometries        = nullptr;  // BLAS: one per geometry
		const ASGeometryInstances*          m_Instances         = nullptr;  // TLAS: one per geometry (usually 1); overrides m_Geometries
		uint32_t                            m_GeometryCount     = 0;
		const uint32_t*                     m_PrimitiveCounts   = nullptr;  // required: one per geometry
		const GeometryFlags*                m_GeometryFlags     = nullptr;  // optional: per-geometry; nullptr = all OPAQUE
		AccelerationStructureHandle         m_Dst               = {};
		uint64_t                            m_ScratchDeviceAddress = 0;
	};

	// Shader binding table region — maps directly to VkStridedDeviceAddressRegionKHR.
	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) ShaderBindingTableRegion final {
		uint64_t m_DeviceAddress = 0;
		uint64_t m_Stride        = 0;
		uint64_t m_Size          = 0;
	};

	// Submit descriptor for VkQueueSubmit2.
	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) SubmitDesc final {
		const SemaphoreHandle*     m_WaitSemaphores      = nullptr;
		const uint64_t*            m_WaitValues          = nullptr;
		const PipelineStage*       m_WaitStages          = nullptr;
		uint32_t                   m_WaitCount           = 0;
		const CommandBufferHandle* m_CommandBuffers      = nullptr;
		uint32_t                   m_CommandBufferCount  = 0;
		const SemaphoreHandle*     m_SignalSemaphores    = nullptr;
		const uint64_t*            m_SignalValues        = nullptr;
		const PipelineStage*       m_SignalStages        = nullptr;
		uint32_t                   m_SignalCount         = 0;
	};

	// Query pool desc

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(4) QueryPoolDesc final {
		QueryType m_Type = QueryType::TIMESTAMP;
		uint32_t  m_Count = 0;
	};

	// Allocation callbacks

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) AllocationCallbacksDesc final {
		void*               m_pUserData        = nullptr;
		PFN_VkAlloc         m_pfnAllocation    = nullptr;
		PFN_VkRealloc       m_pfnReallocation  = nullptr;
		PFN_VkFree          m_pfnFree          = nullptr;
		PFN_VkInternalAlloc m_pfnInternalAlloc = nullptr;
		PFN_VkInternalFree  m_pfnInternalFree  = nullptr;
	};

	// Sync descs

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(4) FenceDesc final {
		bool m_PreSignaled = false;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) TimelineSemaphoreDesc final {
		uint64_t m_InitialValue  = 0;
		bool     m_ExportForCuda = false;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) MemoryBarrierDesc final {
		PipelineStage m_SrcStage  = PipelineStage::NONE;
		PipelineStage m_DstStage  = PipelineStage::NONE;
		AccessType    m_SrcAccess = AccessType::NONE;
		AccessType    m_DstAccess = AccessType::NONE;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(16) BufferBarrierDesc final {
		PipelineStage m_SrcStage       = PipelineStage::NONE;
		PipelineStage m_DstStage       = PipelineStage::NONE;
		AccessType    m_SrcAccess      = AccessType::NONE;
		AccessType    m_DstAccess      = AccessType::NONE;
		BufferHandle  m_Buffer         = {};
		uint64_t      m_Offset         = 0;
		uint64_t      m_Size           = ~0ULL;
		uint32_t      m_SrcQueueFamily = ~0u;
		uint32_t      m_DstQueueFamily = ~0u;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(16) ImageBarrierDesc final {
		PipelineStage m_SrcStage       = PipelineStage::NONE;
		PipelineStage m_DstStage       = PipelineStage::NONE;
		AccessType    m_SrcAccess      = AccessType::NONE;
		AccessType    m_DstAccess      = AccessType::NONE;
		ImageHandle   m_Image          = {};
		ImageLayout   m_OldLayout      = ImageLayout::UNDEFINED;
		ImageLayout   m_NewLayout      = ImageLayout::UNDEFINED;
		ImageAspect   m_Aspect         = ImageAspect::COLOR;
		uint32_t      m_BaseMip        = 0;
		uint32_t      m_MipCount       = 1;
		uint32_t      m_BaseLayer      = 0;
		uint32_t      m_LayerCount     = 1;
		uint32_t      m_SrcQueueFamily = ~0u;
		uint32_t      m_DstQueueFamily = ~0u;
	};
}
