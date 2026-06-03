#include "SpectraVulkanBackend.h"
#include "VulkanPipeline.h"
#include "VulkanInternalHelpers.h"
#include "VulkanState.h"

namespace Spectra::Vulkan {

	// -------------------------------------------------------------------------
	// VulkanShader
	// -------------------------------------------------------------------------

	Utils::ShaderModuleHandle VulkanShader::createShaderModule(
		const uint32_t* p_Spirv, size_t v_SizeBytes,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		auto info       = Internal::vkInit<VkShaderModuleCreateInfo>();
		info.codeSize   = v_SizeBytes;
		info.pCode      = p_Spirv;

		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;

		Utils::ShaderModuleHandle handle{};
		vkCreateShaderModule(g_GlobalInstance.m_LogicalDevice.m_Device, &info, pA,
		                     reinterpret_cast<VkShaderModule*>(&handle.m_Handle));
		return handle;
	}

	void VulkanShader::destroyShaderModule(
		const Utils::ShaderModuleHandle& r_Module,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;
		vkDestroyShaderModule(g_GlobalInstance.m_LogicalDevice.m_Device,
		                      static_cast<VkShaderModule>(r_Module.m_Handle), pA);
	}

	// -------------------------------------------------------------------------
	// VulkanPipeline — layout and cache
	// -------------------------------------------------------------------------

	Utils::PipelineLayoutHandle VulkanPipeline::createLayout(
		uint32_t v_SetCount, const Utils::DescriptorSetLayoutHandle* p_SetLayouts,
		uint32_t v_PushConstantCount, const Utils::PushConstantRange* p_PushConstants,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		VkDescriptorSetLayout setLayouts[8];
		SPEC_VK_BK_ASSERT(v_SetCount <= 8);
		for (uint32_t i = 0; i < v_SetCount; ++i)
			setLayouts[i] = static_cast<VkDescriptorSetLayout>(p_SetLayouts[i].m_Handle);

		VkPushConstantRange pushRanges[8];
		SPEC_VK_BK_ASSERT(v_PushConstantCount <= 8);
		for (uint32_t i = 0; i < v_PushConstantCount; ++i) {
			pushRanges[i].stageFlags = static_cast<VkShaderStageFlags>(p_PushConstants[i].m_StageFlags);
			pushRanges[i].offset     = p_PushConstants[i].m_Offset;
			pushRanges[i].size       = p_PushConstants[i].m_Size;
		}

		auto info                    = Internal::vkInit<VkPipelineLayoutCreateInfo>();
		info.setLayoutCount          = v_SetCount;
		info.pSetLayouts             = setLayouts;
		info.pushConstantRangeCount  = v_PushConstantCount;
		info.pPushConstantRanges     = pushRanges;

		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;

		Utils::PipelineLayoutHandle handle{};
		vkCreatePipelineLayout(g_GlobalInstance.m_LogicalDevice.m_Device, &info, pA,
		                       reinterpret_cast<VkPipelineLayout*>(&handle.m_Handle));
		return handle;
	}

	void VulkanPipeline::destroyLayout(
		const Utils::PipelineLayoutHandle& r_Layout,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;
		vkDestroyPipelineLayout(g_GlobalInstance.m_LogicalDevice.m_Device,
		                        static_cast<VkPipelineLayout>(r_Layout.m_Handle), pA);
	}

	Utils::PipelineCacheHandle VulkanPipeline::createCache(const Utils::AllocationCallbacksDesc& r_Alloc) {
		return createCacheFromData(nullptr, 0, r_Alloc);
	}

	Utils::PipelineCacheHandle VulkanPipeline::createCacheFromData(
		const void* p_Data, size_t v_Size,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		auto info             = Internal::vkInit<VkPipelineCacheCreateInfo>();
		info.initialDataSize  = v_Size;
		info.pInitialData     = p_Data;

		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;

		Utils::PipelineCacheHandle handle{};
		vkCreatePipelineCache(g_GlobalInstance.m_LogicalDevice.m_Device, &info, pA,
		                      reinterpret_cast<VkPipelineCache*>(&handle.m_Handle));
		return handle;
	}

	void VulkanPipeline::destroyCache(
		const Utils::PipelineCacheHandle& r_Cache,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;
		vkDestroyPipelineCache(g_GlobalInstance.m_LogicalDevice.m_Device,
		                       static_cast<VkPipelineCache>(r_Cache.m_Handle), pA);
	}

	void VulkanPipeline::destroyPipeline(
		const Utils::PipelineHandle& r_Pipeline,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;
		vkDestroyPipeline(g_GlobalInstance.m_LogicalDevice.m_Device,
		                  static_cast<VkPipeline>(r_Pipeline.m_Handle), pA);
	}

	// -------------------------------------------------------------------------
	// VulkanPipeline — graphics
	// -------------------------------------------------------------------------

	Utils::PipelineHandle VulkanPipeline::createGraphics(
		const Utils::GraphicsPipelineDesc& r_Desc,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		// Shader stages
		VkPipelineShaderStageCreateInfo stages[8];
		SPEC_VK_BK_ASSERT(r_Desc.m_StageCount <= 8);
		for (uint32_t i = 0; i < r_Desc.m_StageCount; ++i) {
			stages[i]        = Internal::vkInit<VkPipelineShaderStageCreateInfo>();
			stages[i].stage  = static_cast<VkShaderStageFlagBits>(r_Desc.m_Stages[i].m_Stage);
			stages[i].module = static_cast<VkShaderModule>(r_Desc.m_Stages[i].m_Module.m_Handle);
			stages[i].pName  = r_Desc.m_Stages[i].m_EntryPoint;
		}

		// Vertex input
		VkVertexInputBindingDescription   vkBindings[16];
		VkVertexInputAttributeDescription vkAttribs [32];
		SPEC_VK_BK_ASSERT(r_Desc.m_VertexBindingCount   <= 16);
		SPEC_VK_BK_ASSERT(r_Desc.m_VertexAttributeCount <= 32);
		for (uint32_t i = 0; i < r_Desc.m_VertexBindingCount; ++i) {
			vkBindings[i].binding   = r_Desc.m_VertexBindings[i].m_Binding;
			vkBindings[i].stride    = r_Desc.m_VertexBindings[i].m_Stride;
			vkBindings[i].inputRate = static_cast<VkVertexInputRate>(r_Desc.m_VertexBindings[i].m_InputRate);
		}
		for (uint32_t i = 0; i < r_Desc.m_VertexAttributeCount; ++i) {
			vkAttribs[i].location = r_Desc.m_VertexAttributes[i].m_Location;
			vkAttribs[i].binding  = r_Desc.m_VertexAttributes[i].m_Binding;
			vkAttribs[i].format   = static_cast<VkFormat>(r_Desc.m_VertexAttributes[i].m_Format);
			vkAttribs[i].offset   = r_Desc.m_VertexAttributes[i].m_Offset;
		}
		auto vertexInput                       = Internal::vkInit<VkPipelineVertexInputStateCreateInfo>();
		vertexInput.vertexBindingDescriptionCount   = r_Desc.m_VertexBindingCount;
		vertexInput.pVertexBindingDescriptions      = vkBindings;
		vertexInput.vertexAttributeDescriptionCount = r_Desc.m_VertexAttributeCount;
		vertexInput.pVertexAttributeDescriptions    = vkAttribs;

		// Input assembly
		auto ia          = Internal::vkInit<VkPipelineInputAssemblyStateCreateInfo>();
		ia.topology      = static_cast<VkPrimitiveTopology>(r_Desc.m_Topology);

		// Viewport (both dynamic)
		auto vp               = Internal::vkInit<VkPipelineViewportStateCreateInfo>();
		vp.viewportCount      = 1;
		vp.scissorCount       = 1;

		// Rasterization
		auto rs                   = Internal::vkInit<VkPipelineRasterizationStateCreateInfo>();
		rs.polygonMode            = static_cast<VkPolygonMode>(r_Desc.m_Rasterization.m_PolygonMode);
		rs.cullMode               = static_cast<VkCullModeFlags>(r_Desc.m_Rasterization.m_CullMode);
		rs.frontFace              = static_cast<VkFrontFace>(r_Desc.m_Rasterization.m_FrontFace);
		rs.depthBiasEnable        = r_Desc.m_Rasterization.m_DepthBiasEnable ? VK_TRUE : VK_FALSE;
		rs.depthBiasConstantFactor = r_Desc.m_Rasterization.m_DepthBiasConstant;
		rs.depthBiasSlopeFactor    = r_Desc.m_Rasterization.m_DepthBiasSlope;
		rs.lineWidth              = 1.0f;

		// Multisample (always 1 sample in base API)
		auto ms                          = Internal::vkInit<VkPipelineMultisampleStateCreateInfo>();
		ms.rasterizationSamples          = VK_SAMPLE_COUNT_1_BIT;

		// Depth stencil
		auto ds                  = Internal::vkInit<VkPipelineDepthStencilStateCreateInfo>();
		ds.depthTestEnable       = r_Desc.m_DepthStencil.m_DepthTestEnable  ? VK_TRUE : VK_FALSE;
		ds.depthWriteEnable      = r_Desc.m_DepthStencil.m_DepthWriteEnable ? VK_TRUE : VK_FALSE;
		ds.depthCompareOp        = static_cast<VkCompareOp>(r_Desc.m_DepthStencil.m_DepthCompareOp);
		ds.stencilTestEnable     = r_Desc.m_DepthStencil.m_StencilTestEnable ? VK_TRUE : VK_FALSE;

		// Color blend attachments
		VkPipelineColorBlendAttachmentState blendAttachments[8];
		SPEC_VK_BK_ASSERT(r_Desc.m_BlendAttachmentCount <= 8);
		for (uint32_t i = 0; i < r_Desc.m_BlendAttachmentCount; ++i) {
			const auto& src = r_Desc.m_BlendAttachments[i];
			auto& dst       = blendAttachments[i];
			dst.blendEnable         = src.m_BlendEnable ? VK_TRUE : VK_FALSE;
			dst.srcColorBlendFactor = static_cast<VkBlendFactor>(src.m_SrcColor);
			dst.dstColorBlendFactor = static_cast<VkBlendFactor>(src.m_DstColor);
			dst.colorBlendOp        = static_cast<VkBlendOp>(src.m_ColorOp);
			dst.srcAlphaBlendFactor = static_cast<VkBlendFactor>(src.m_SrcAlpha);
			dst.dstAlphaBlendFactor = static_cast<VkBlendFactor>(src.m_DstAlpha);
			dst.alphaBlendOp        = static_cast<VkBlendOp>(src.m_AlphaOp);
			dst.colorWriteMask      = static_cast<VkColorComponentFlags>(src.m_WriteMask);
		}
		auto cb                    = Internal::vkInit<VkPipelineColorBlendStateCreateInfo>();
		cb.attachmentCount         = r_Desc.m_BlendAttachmentCount;
		cb.pAttachments            = blendAttachments;

		// Dynamic state
		VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		auto dyn                   = Internal::vkInit<VkPipelineDynamicStateCreateInfo>();
		dyn.dynamicStateCount      = 2;
		dyn.pDynamicStates         = dynStates;

		// Dynamic rendering — no render pass
		VkFormat colorFmts[8];
		SPEC_VK_BK_ASSERT(r_Desc.m_ColorFormatCount <= 8);
		for (uint32_t i = 0; i < r_Desc.m_ColorFormatCount; ++i)
			colorFmts[i] = static_cast<VkFormat>(r_Desc.m_ColorFormats[i]);
		auto rendering                          = Internal::vkInit<VkPipelineRenderingCreateInfo>();
		rendering.colorAttachmentCount          = r_Desc.m_ColorFormatCount;
		rendering.pColorAttachmentFormats       = colorFmts;
		rendering.depthAttachmentFormat         = static_cast<VkFormat>(r_Desc.m_DepthFormat);

		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;

		auto ci                    = Internal::vkInit<VkGraphicsPipelineCreateInfo>();
		ci.pNext                   = &rendering;
		ci.stageCount              = r_Desc.m_StageCount;
		ci.pStages                 = stages;
		ci.pVertexInputState       = &vertexInput;
		ci.pInputAssemblyState     = &ia;
		ci.pViewportState          = &vp;
		ci.pRasterizationState     = &rs;
		ci.pMultisampleState       = &ms;
		ci.pDepthStencilState      = &ds;
		ci.pColorBlendState        = &cb;
		ci.pDynamicState           = &dyn;
		ci.layout                  = static_cast<VkPipelineLayout>(r_Desc.m_Layout.m_Handle);
		ci.renderPass              = VK_NULL_HANDLE;

		Utils::PipelineHandle handle{};
		vkCreateGraphicsPipelines(g_GlobalInstance.m_LogicalDevice.m_Device,
		                          static_cast<VkPipelineCache>(r_Desc.m_Cache.m_Handle),
		                          1, &ci, pA,
		                          reinterpret_cast<VkPipeline*>(&handle.m_Handle));
		return handle;
	}

	// -------------------------------------------------------------------------
	// VulkanPipeline — compute
	// -------------------------------------------------------------------------

	Utils::PipelineHandle VulkanPipeline::createCompute(
		const Utils::ComputePipelineDesc& r_Desc,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		auto stage        = Internal::vkInit<VkPipelineShaderStageCreateInfo>();
		stage.stage       = static_cast<VkShaderStageFlagBits>(r_Desc.m_Stage.m_Stage);
		stage.module      = static_cast<VkShaderModule>(r_Desc.m_Stage.m_Module.m_Handle);
		stage.pName       = r_Desc.m_Stage.m_EntryPoint;

		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;

		auto ci    = Internal::vkInit<VkComputePipelineCreateInfo>();
		ci.stage   = stage;
		ci.layout  = static_cast<VkPipelineLayout>(r_Desc.m_Layout.m_Handle);

		Utils::PipelineHandle handle{};
		vkCreateComputePipelines(g_GlobalInstance.m_LogicalDevice.m_Device,
		                         static_cast<VkPipelineCache>(r_Desc.m_Cache.m_Handle),
		                         1, &ci, pA,
		                         reinterpret_cast<VkPipeline*>(&handle.m_Handle));
		return handle;
	}

	// -------------------------------------------------------------------------
	// VulkanPipeline — ray tracing (dynamically loaded)
	// -------------------------------------------------------------------------

	Utils::PipelineHandle VulkanPipeline::createRayTracing(
		const Utils::RayTracingPipelineDesc& r_Desc,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
		static auto pfnCreate = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(
			vkGetDeviceProcAddr(dev, "vkCreateRayTracingPipelinesKHR"));
		SPEC_VK_BK_ASSERT(pfnCreate != nullptr);

		VkPipelineShaderStageCreateInfo stages[16];
		SPEC_VK_BK_ASSERT(r_Desc.m_StageCount <= 16);
		for (uint32_t i = 0; i < r_Desc.m_StageCount; ++i) {
			stages[i]       = Internal::vkInit<VkPipelineShaderStageCreateInfo>();
			stages[i].stage = static_cast<VkShaderStageFlagBits>(r_Desc.m_Stages[i].m_Stage);
			stages[i].module = static_cast<VkShaderModule>(r_Desc.m_Stages[i].m_Module.m_Handle);
			stages[i].pName = r_Desc.m_Stages[i].m_EntryPoint;
		}

		VkRayTracingShaderGroupCreateInfoKHR groups[32];
		SPEC_VK_BK_ASSERT(r_Desc.m_GroupCount <= 32);
		for (uint32_t i = 0; i < r_Desc.m_GroupCount; ++i) {
			groups[i]                       = Internal::vkInit<VkRayTracingShaderGroupCreateInfoKHR>();
			groups[i].type                  = static_cast<VkRayTracingShaderGroupTypeKHR>(r_Desc.m_Groups[i].m_Type);
			groups[i].generalShader         = r_Desc.m_Groups[i].m_GeneralShader;
			groups[i].closestHitShader      = r_Desc.m_Groups[i].m_ClosestHitShader;
			groups[i].anyHitShader          = r_Desc.m_Groups[i].m_AnyHitShader;
			groups[i].intersectionShader    = r_Desc.m_Groups[i].m_IntersectionShader;
		}

		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;

		auto ci                    = Internal::vkInit<VkRayTracingPipelineCreateInfoKHR>();
		ci.stageCount              = r_Desc.m_StageCount;
		ci.pStages                 = stages;
		ci.groupCount              = r_Desc.m_GroupCount;
		ci.pGroups                 = groups;
		ci.maxPipelineRayRecursionDepth = r_Desc.m_MaxRecursionDepth;
		ci.layout                  = static_cast<VkPipelineLayout>(r_Desc.m_Layout.m_Handle);

		Utils::PipelineHandle handle{};
		pfnCreate(dev,
		          VK_NULL_HANDLE,
		          static_cast<VkPipelineCache>(r_Desc.m_Cache.m_Handle),
		          1, &ci, pA,
		          reinterpret_cast<VkPipeline*>(&handle.m_Handle));
		return handle;
	}

	void VulkanPipeline::getRayTracingShaderGroupHandles(
		const Utils::PipelineHandle& r_Pipeline,
		uint32_t v_FirstGroup, uint32_t v_GroupCount,
		uint32_t v_DataSize, void* p_Data)
	{
		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
		static auto pfn = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(
			vkGetDeviceProcAddr(dev, "vkGetRayTracingShaderGroupHandlesKHR"));
		SPEC_VK_BK_ASSERT(pfn != nullptr);
		pfn(dev, static_cast<VkPipeline>(r_Pipeline.m_Handle),
		    v_FirstGroup, v_GroupCount, v_DataSize, p_Data);
	}
}
