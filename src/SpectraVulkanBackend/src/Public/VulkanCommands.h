#pragma once

#include "VulkanUtils.h"

namespace Spectra::Vulkan {

	class SPEC_VK_BK_RUNTIME_API VulkanCommandBuffer final {
	public:
		static constexpr uint32_t MAX_VERTEX_BUFFERS_BOUND = 16;

		// Lifecycle
		static Utils::CommandBufferHandle allocate(Utils::QueueType v_QueueType);
		static void                       free    (const Utils::CommandBufferHandle& r_Cmd, Utils::QueueType v_QueueType);
		static void                       begin           (const Utils::CommandBufferHandle& r_Cmd); // ONE_TIME_SUBMIT
		static void                       beginReusable   (const Utils::CommandBufferHandle& r_Cmd); // reset+rerecord each frame
		static void                       end             (const Utils::CommandBufferHandle& r_Cmd);
		static void                       reset           (const Utils::CommandBufferHandle& r_Cmd);

		// Dynamic rendering
		static void beginRendering(const Utils::CommandBufferHandle& r_Cmd, const Utils::RenderingDesc& r_Desc);
		static void endRendering  (const Utils::CommandBufferHandle& r_Cmd);

		// Pipeline state
		static void bindPipeline(const Utils::CommandBufferHandle& r_Cmd,
		                         const Utils::PipelineHandle& r_Pipeline,
		                         Utils::PipelineBindPoint v_BindPoint);
		static void bindDescriptorSets(const Utils::CommandBufferHandle& r_Cmd,
		                               const Utils::PipelineLayoutHandle& r_Layout,
		                               Utils::PipelineBindPoint v_BindPoint,
		                               uint32_t v_FirstSet,
		                               uint32_t v_SetCount, const Utils::DescriptorSetHandle* p_Sets);
		static void pushConstants(const Utils::CommandBufferHandle& r_Cmd,
		                          const Utils::PipelineLayoutHandle& r_Layout,
		                          Utils::ShaderStage v_Stages,
		                          uint32_t v_Offset, uint32_t v_Size, const void* p_Data);

		// Vertex / index
		// v_Count must be <= MAX_VERTEX_BUFFERS_BOUND
		static void bindVertexBuffers(const Utils::CommandBufferHandle& r_Cmd,
		                              uint32_t v_First, uint32_t v_Count,
		                              const Utils::BufferHandle* p_Buffers,
		                              const uint64_t* p_Offsets);
		static void bindIndexBuffer(const Utils::CommandBufferHandle& r_Cmd,
		                            const Utils::BufferHandle& r_Buffer,
		                            uint64_t v_Offset, Utils::IndexType v_Type);

		// Dynamic state
		static void setViewport(const Utils::CommandBufferHandle& r_Cmd,
		                        float v_X, float v_Y, float v_Width, float v_Height,
		                        float v_MinDepth, float v_MaxDepth);
		static void setScissor (const Utils::CommandBufferHandle& r_Cmd,
		                        int32_t v_X, int32_t v_Y, uint32_t v_Width, uint32_t v_Height);

		// Draw
		static void draw       (const Utils::CommandBufferHandle& r_Cmd,
		                        uint32_t v_VertexCount, uint32_t v_InstanceCount,
		                        uint32_t v_FirstVertex, uint32_t v_FirstInstance);
		static void drawIndexed(const Utils::CommandBufferHandle& r_Cmd,
		                        uint32_t v_IndexCount, uint32_t v_InstanceCount,
		                        uint32_t v_FirstIndex, int32_t v_VertexOffset,
		                        uint32_t v_FirstInstance);

		// Compute
		static void dispatch(const Utils::CommandBufferHandle& r_Cmd,
		                     uint32_t v_X, uint32_t v_Y, uint32_t v_Z);

		// Ray tracing — SBT regions carry device addresses + strides + sizes
		static void dispatchRays(const Utils::CommandBufferHandle& r_Cmd,
		                         const Utils::ShaderBindingTableRegion& r_Raygen,
		                         const Utils::ShaderBindingTableRegion& r_Miss,
		                         const Utils::ShaderBindingTableRegion& r_Hit,
		                         const Utils::ShaderBindingTableRegion& r_Callable,
		                         uint32_t v_Width, uint32_t v_Height, uint32_t v_Depth);

		// Transfer
		static void copyBuffer(const Utils::CommandBufferHandle& r_Cmd,
		                       const Utils::BufferHandle& r_Src, const Utils::BufferHandle& r_Dst,
		                       uint64_t v_SrcOffset, uint64_t v_DstOffset, uint64_t v_Size);
		static void copyBufferToImage(const Utils::CommandBufferHandle& r_Cmd,
		                              const Utils::BufferHandle& r_Src, uint64_t v_SrcOffset,
		                              const Utils::ImageHandle& r_Dst, Utils::ImageLayout v_DstLayout,
		                              uint32_t v_Width, uint32_t v_Height,
		                              Utils::ImageAspect v_Aspect,
		                              uint32_t v_MipLevel, uint32_t v_ArrayLayer);
		static void blitImage(const Utils::CommandBufferHandle& r_Cmd,
		                      const Utils::ImageHandle& r_Src, Utils::ImageLayout v_SrcLayout,
		                      const Utils::ImageHandle& r_Dst, Utils::ImageLayout v_DstLayout,
		                      uint32_t v_SrcWidth, uint32_t v_SrcHeight, uint32_t v_SrcMip,
		                      uint32_t v_DstWidth, uint32_t v_DstHeight, uint32_t v_DstMip,
		                      Utils::ImageAspect v_Aspect, Utils::FilterMode v_Filter);

		VulkanCommandBuffer() = delete;
		~VulkanCommandBuffer() = delete;
		VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
		VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;
		VulkanCommandBuffer(VulkanCommandBuffer&&) noexcept = delete;
		VulkanCommandBuffer& operator=(VulkanCommandBuffer&&) noexcept = delete;
	};
}
