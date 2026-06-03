#pragma once

#include "VulkanUtils.h"

namespace Spectra::Vulkan {

	class SPEC_VK_BK_RUNTIME_API VulkanShader final {
	public:
		// p_Spirv must be 4-byte aligned. v_SizeBytes is the byte size of the SPIR-V blob.
		static Utils::ShaderModuleHandle createShaderModule(
			const uint32_t* p_Spirv, size_t v_SizeBytes,
			const Utils::AllocationCallbacksDesc& r_Alloc);
		static void destroyShaderModule(
			const Utils::ShaderModuleHandle& r_Module,
			const Utils::AllocationCallbacksDesc& r_Alloc);

		VulkanShader() = delete;
		~VulkanShader() = delete;
		VulkanShader(const VulkanShader&) = delete;
		VulkanShader& operator=(const VulkanShader&) = delete;
		VulkanShader(VulkanShader&&) noexcept = delete;
		VulkanShader& operator=(VulkanShader&&) noexcept = delete;
	};

	class SPEC_VK_BK_RUNTIME_API VulkanPipeline final {
	public:
		static Utils::PipelineLayoutHandle createLayout(
			uint32_t v_SetCount, const Utils::DescriptorSetLayoutHandle* p_SetLayouts,
			uint32_t v_PushConstantCount, const Utils::PushConstantRange* p_PushConstants,
			const Utils::AllocationCallbacksDesc& r_Alloc);
		static void destroyLayout(
			const Utils::PipelineLayoutHandle& r_Layout,
			const Utils::AllocationCallbacksDesc& r_Alloc);

		// Empty pipeline cache (initial state). Serialize with vkGetPipelineCacheData externally.
		static Utils::PipelineCacheHandle createCache(const Utils::AllocationCallbacksDesc& r_Alloc);
		// Populate from previously saved binary data.
		static Utils::PipelineCacheHandle createCacheFromData(
			const void* p_Data, size_t v_Size,
			const Utils::AllocationCallbacksDesc& r_Alloc);
		static void destroyCache(
			const Utils::PipelineCacheHandle& r_Cache,
			const Utils::AllocationCallbacksDesc& r_Alloc);

		// All graphics pipelines use dynamic viewport+scissor. No renderpass — dynamic rendering.
		static Utils::PipelineHandle createGraphics(
			const Utils::GraphicsPipelineDesc& r_Desc,
			const Utils::AllocationCallbacksDesc& r_Alloc);
		static Utils::PipelineHandle createCompute(
			const Utils::ComputePipelineDesc& r_Desc,
			const Utils::AllocationCallbacksDesc& r_Alloc);
		static Utils::PipelineHandle createRayTracing(
			const Utils::RayTracingPipelineDesc& r_Desc,
			const Utils::AllocationCallbacksDesc& r_Alloc);
		static void destroyPipeline(
			const Utils::PipelineHandle& r_Pipeline,
			const Utils::AllocationCallbacksDesc& r_Alloc);

		// Copies v_GroupCount * handleSize bytes into p_Data (handleSize from physical device RT props).
		static void getRayTracingShaderGroupHandles(
			const Utils::PipelineHandle& r_Pipeline,
			uint32_t v_FirstGroup, uint32_t v_GroupCount,
			uint32_t v_DataSize, void* p_Data);

		VulkanPipeline() = delete;
		~VulkanPipeline() = delete;
		VulkanPipeline(const VulkanPipeline&) = delete;
		VulkanPipeline& operator=(const VulkanPipeline&) = delete;
		VulkanPipeline(VulkanPipeline&&) noexcept = delete;
		VulkanPipeline& operator=(VulkanPipeline&&) noexcept = delete;
	};
}
