#include "SpectraVulkanBackend.h"
#include "VulkanCommands.h"
#include "VulkanInternalHelpers.h"
#include "VulkanState.h"

namespace Spectra::Vulkan {

	static VkCommandPool poolForQueue(Utils::QueueType v_Type) {
		switch (v_Type) {
		case Utils::QueueType::COMPUTE:  return g_GlobalInstance.m_CommandPools.m_ComputePool;
		case Utils::QueueType::TRANSFER: return g_GlobalInstance.m_CommandPools.m_TransferPool;
		default:                         return g_GlobalInstance.m_CommandPools.m_GraphicsPool;
		}
	}

	// -------------------------------------------------------------------------
	// Lifecycle
	// -------------------------------------------------------------------------

	Utils::CommandBufferHandle VulkanCommandBuffer::allocate(Utils::QueueType v_QueueType) {
		auto info                   = Internal::vkInit<VkCommandBufferAllocateInfo>();
		info.commandPool            = poolForQueue(v_QueueType);
		info.level                  = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		info.commandBufferCount     = 1;

		Utils::CommandBufferHandle handle{};
		vkAllocateCommandBuffers(g_GlobalInstance.m_LogicalDevice.m_Device, &info,
		                         reinterpret_cast<VkCommandBuffer*>(&handle.m_Handle));
		return handle;
	}

	void VulkanCommandBuffer::free(const Utils::CommandBufferHandle& r_Cmd, Utils::QueueType v_QueueType) {
		VkCommandBuffer cmd = static_cast<VkCommandBuffer>(r_Cmd.m_Handle);
		vkFreeCommandBuffers(g_GlobalInstance.m_LogicalDevice.m_Device,
		                     poolForQueue(v_QueueType), 1, &cmd);
	}

	void VulkanCommandBuffer::begin(const Utils::CommandBufferHandle& r_Cmd) {
		auto info  = Internal::vkInit<VkCommandBufferBeginInfo>();
		info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(static_cast<VkCommandBuffer>(r_Cmd.m_Handle), &info);
	}

	void VulkanCommandBuffer::beginReusable(const Utils::CommandBufferHandle& r_Cmd) {
		auto info = Internal::vkInit<VkCommandBufferBeginInfo>();
		vkBeginCommandBuffer(static_cast<VkCommandBuffer>(r_Cmd.m_Handle), &info);
	}

	void VulkanCommandBuffer::end(const Utils::CommandBufferHandle& r_Cmd) {
		vkEndCommandBuffer(static_cast<VkCommandBuffer>(r_Cmd.m_Handle));
	}

	void VulkanCommandBuffer::reset(const Utils::CommandBufferHandle& r_Cmd) {
		vkResetCommandBuffer(static_cast<VkCommandBuffer>(r_Cmd.m_Handle), 0);
	}

	// -------------------------------------------------------------------------
	// Dynamic rendering
	// -------------------------------------------------------------------------

	void VulkanCommandBuffer::beginRendering(
		const Utils::CommandBufferHandle& r_Cmd, const Utils::RenderingDesc& r_Desc)
	{
		VkRenderingAttachmentInfo colorAttachments[8];
		SPEC_VK_BK_ASSERT(r_Desc.m_ColorAttachmentCount <= 8);
		for (uint32_t i = 0; i < r_Desc.m_ColorAttachmentCount; ++i) {
			const auto& src = r_Desc.m_ColorAttachments[i];
			auto& dst       = colorAttachments[i];
			dst = Internal::vkInit<VkRenderingAttachmentInfo>();
			dst.imageView   = static_cast<VkImageView>(src.m_ImageView.m_Handle);
			dst.imageLayout = static_cast<VkImageLayout>(src.m_Layout);
			dst.loadOp      = static_cast<VkAttachmentLoadOp>(src.m_LoadOp);
			dst.storeOp     = static_cast<VkAttachmentStoreOp>(src.m_StoreOp);
			static_assert(sizeof(dst.clearValue) == sizeof(src.m_ClearValue));
			memcpy(&dst.clearValue, &src.m_ClearValue, sizeof(dst.clearValue));
		}

		VkRenderingAttachmentInfo depthAttachment{};
		if (r_Desc.m_DepthAttachment != nullptr) {
			const auto& src = *r_Desc.m_DepthAttachment;
			depthAttachment = Internal::vkInit<VkRenderingAttachmentInfo>();
			depthAttachment.imageView   = static_cast<VkImageView>(src.m_ImageView.m_Handle);
			depthAttachment.imageLayout = static_cast<VkImageLayout>(src.m_Layout);
			depthAttachment.loadOp      = static_cast<VkAttachmentLoadOp>(src.m_LoadOp);
			depthAttachment.storeOp     = static_cast<VkAttachmentStoreOp>(src.m_StoreOp);
			memcpy(&depthAttachment.clearValue, &src.m_ClearValue, sizeof(depthAttachment.clearValue));
		}

		auto ri                        = Internal::vkInit<VkRenderingInfo>();
		ri.renderArea.offset           = { r_Desc.m_OffsetX, r_Desc.m_OffsetY };
		ri.renderArea.extent           = { r_Desc.m_Width, r_Desc.m_Height };
		ri.layerCount                  = 1;
		ri.colorAttachmentCount        = r_Desc.m_ColorAttachmentCount;
		ri.pColorAttachments           = colorAttachments;
		ri.pDepthAttachment            = r_Desc.m_DepthAttachment ? &depthAttachment : nullptr;

		vkCmdBeginRendering(static_cast<VkCommandBuffer>(r_Cmd.m_Handle), &ri);
	}

	void VulkanCommandBuffer::endRendering(const Utils::CommandBufferHandle& r_Cmd) {
		vkCmdEndRendering(static_cast<VkCommandBuffer>(r_Cmd.m_Handle));
	}

	// -------------------------------------------------------------------------
	// Pipeline state
	// -------------------------------------------------------------------------

	void VulkanCommandBuffer::bindPipeline(
		const Utils::CommandBufferHandle& r_Cmd,
		const Utils::PipelineHandle& r_Pipeline,
		Utils::PipelineBindPoint v_BindPoint)
	{
		vkCmdBindPipeline(static_cast<VkCommandBuffer>(r_Cmd.m_Handle),
		                  static_cast<VkPipelineBindPoint>(v_BindPoint),
		                  static_cast<VkPipeline>(r_Pipeline.m_Handle));
	}

	void VulkanCommandBuffer::bindDescriptorSets(
		const Utils::CommandBufferHandle& r_Cmd,
		const Utils::PipelineLayoutHandle& r_Layout,
		Utils::PipelineBindPoint v_BindPoint,
		uint32_t v_FirstSet,
		uint32_t v_SetCount, const Utils::DescriptorSetHandle* p_Sets)
	{
		VkDescriptorSet sets[16];
		SPEC_VK_BK_ASSERT(v_SetCount <= 16);
		for (uint32_t i = 0; i < v_SetCount; ++i)
			sets[i] = static_cast<VkDescriptorSet>(p_Sets[i].m_Handle);

		vkCmdBindDescriptorSets(static_cast<VkCommandBuffer>(r_Cmd.m_Handle),
		                        static_cast<VkPipelineBindPoint>(v_BindPoint),
		                        static_cast<VkPipelineLayout>(r_Layout.m_Handle),
		                        v_FirstSet, v_SetCount, sets, 0, nullptr);
	}

	void VulkanCommandBuffer::pushConstants(
		const Utils::CommandBufferHandle& r_Cmd,
		const Utils::PipelineLayoutHandle& r_Layout,
		Utils::ShaderStage v_Stages,
		uint32_t v_Offset, uint32_t v_Size, const void* p_Data)
	{
		vkCmdPushConstants(static_cast<VkCommandBuffer>(r_Cmd.m_Handle),
		                   static_cast<VkPipelineLayout>(r_Layout.m_Handle),
		                   static_cast<VkShaderStageFlags>(v_Stages),
		                   v_Offset, v_Size, p_Data);
	}

	// -------------------------------------------------------------------------
	// Vertex / index
	// -------------------------------------------------------------------------

	void VulkanCommandBuffer::bindVertexBuffers(
		const Utils::CommandBufferHandle& r_Cmd,
		uint32_t v_First, uint32_t v_Count,
		const Utils::BufferHandle* p_Buffers, const uint64_t* p_Offsets)
	{
		SPEC_VK_BK_ASSERT(v_Count <= MAX_VERTEX_BUFFERS_BOUND);
		VkBuffer bufs[MAX_VERTEX_BUFFERS_BOUND];
		for (uint32_t i = 0; i < v_Count; ++i)
			bufs[i] = static_cast<VkBuffer>(p_Buffers[i].m_Handle);
		vkCmdBindVertexBuffers(static_cast<VkCommandBuffer>(r_Cmd.m_Handle),
		                       v_First, v_Count, bufs, p_Offsets);
	}

	void VulkanCommandBuffer::bindIndexBuffer(
		const Utils::CommandBufferHandle& r_Cmd,
		const Utils::BufferHandle& r_Buffer,
		uint64_t v_Offset, Utils::IndexType v_Type)
	{
		vkCmdBindIndexBuffer(static_cast<VkCommandBuffer>(r_Cmd.m_Handle),
		                     static_cast<VkBuffer>(r_Buffer.m_Handle),
		                     v_Offset,
		                     static_cast<VkIndexType>(v_Type));
	}

	// -------------------------------------------------------------------------
	// Dynamic state
	// -------------------------------------------------------------------------

	void VulkanCommandBuffer::setViewport(
		const Utils::CommandBufferHandle& r_Cmd,
		float v_X, float v_Y, float v_Width, float v_Height,
		float v_MinDepth, float v_MaxDepth)
	{
		VkViewport vp{ v_X, v_Y, v_Width, v_Height, v_MinDepth, v_MaxDepth };
		vkCmdSetViewport(static_cast<VkCommandBuffer>(r_Cmd.m_Handle), 0, 1, &vp);
	}

	void VulkanCommandBuffer::setScissor(
		const Utils::CommandBufferHandle& r_Cmd,
		int32_t v_X, int32_t v_Y, uint32_t v_Width, uint32_t v_Height)
	{
		VkRect2D sc{ { v_X, v_Y }, { v_Width, v_Height } };
		vkCmdSetScissor(static_cast<VkCommandBuffer>(r_Cmd.m_Handle), 0, 1, &sc);
	}

	// -------------------------------------------------------------------------
	// Draw / dispatch
	// -------------------------------------------------------------------------

	void VulkanCommandBuffer::draw(
		const Utils::CommandBufferHandle& r_Cmd,
		uint32_t v_VertexCount, uint32_t v_InstanceCount,
		uint32_t v_FirstVertex, uint32_t v_FirstInstance)
	{
		vkCmdDraw(static_cast<VkCommandBuffer>(r_Cmd.m_Handle),
		          v_VertexCount, v_InstanceCount, v_FirstVertex, v_FirstInstance);
	}

	void VulkanCommandBuffer::drawIndexed(
		const Utils::CommandBufferHandle& r_Cmd,
		uint32_t v_IndexCount, uint32_t v_InstanceCount,
		uint32_t v_FirstIndex, int32_t v_VertexOffset, uint32_t v_FirstInstance)
	{
		vkCmdDrawIndexed(static_cast<VkCommandBuffer>(r_Cmd.m_Handle),
		                 v_IndexCount, v_InstanceCount, v_FirstIndex,
		                 v_VertexOffset, v_FirstInstance);
	}

	void VulkanCommandBuffer::dispatch(
		const Utils::CommandBufferHandle& r_Cmd,
		uint32_t v_X, uint32_t v_Y, uint32_t v_Z)
	{
		vkCmdDispatch(static_cast<VkCommandBuffer>(r_Cmd.m_Handle), v_X, v_Y, v_Z);
	}

	void VulkanCommandBuffer::dispatchRays(
		const Utils::CommandBufferHandle& r_Cmd,
		const Utils::ShaderBindingTableRegion& r_Raygen,
		const Utils::ShaderBindingTableRegion& r_Miss,
		const Utils::ShaderBindingTableRegion& r_Hit,
		const Utils::ShaderBindingTableRegion& r_Callable,
		uint32_t v_Width, uint32_t v_Height, uint32_t v_Depth)
	{
		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
		static auto pfn = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(
			vkGetDeviceProcAddr(dev, "vkCmdTraceRaysKHR"));
		SPEC_VK_BK_ASSERT(pfn != nullptr);

		// ShaderBindingTableRegion has identical layout to VkStridedDeviceAddressRegionKHR
		pfn(static_cast<VkCommandBuffer>(r_Cmd.m_Handle),
		    reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(&r_Raygen),
		    reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(&r_Miss),
		    reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(&r_Hit),
		    reinterpret_cast<const VkStridedDeviceAddressRegionKHR*>(&r_Callable),
		    v_Width, v_Height, v_Depth);
	}

	// -------------------------------------------------------------------------
	// Transfer
	// -------------------------------------------------------------------------

	void VulkanCommandBuffer::copyBuffer(
		const Utils::CommandBufferHandle& r_Cmd,
		const Utils::BufferHandle& r_Src, const Utils::BufferHandle& r_Dst,
		uint64_t v_SrcOffset, uint64_t v_DstOffset, uint64_t v_Size)
	{
		VkBufferCopy region{ v_SrcOffset, v_DstOffset, v_Size };
		vkCmdCopyBuffer(static_cast<VkCommandBuffer>(r_Cmd.m_Handle),
		                static_cast<VkBuffer>(r_Src.m_Handle),
		                static_cast<VkBuffer>(r_Dst.m_Handle),
		                1, &region);
	}

	void VulkanCommandBuffer::copyBufferToImage(
		const Utils::CommandBufferHandle& r_Cmd,
		const Utils::BufferHandle& r_Src, uint64_t v_SrcOffset,
		const Utils::ImageHandle& r_Dst, Utils::ImageLayout v_DstLayout,
		uint32_t v_Width, uint32_t v_Height,
		Utils::ImageAspect v_Aspect, uint32_t v_MipLevel, uint32_t v_ArrayLayer)
	{
		VkBufferImageCopy region{};
		region.bufferOffset                    = v_SrcOffset;
		region.imageSubresource.aspectMask     = static_cast<VkImageAspectFlags>(v_Aspect);
		region.imageSubresource.mipLevel       = v_MipLevel;
		region.imageSubresource.baseArrayLayer = v_ArrayLayer;
		region.imageSubresource.layerCount     = 1;
		region.imageExtent                     = { v_Width, v_Height, 1 };

		vkCmdCopyBufferToImage(static_cast<VkCommandBuffer>(r_Cmd.m_Handle),
		                       static_cast<VkBuffer>(r_Src.m_Handle),
		                       static_cast<VkImage>(r_Dst.m_Handle),
		                       static_cast<VkImageLayout>(v_DstLayout),
		                       1, &region);
	}

	void VulkanCommandBuffer::blitImage(
		const Utils::CommandBufferHandle& r_Cmd,
		const Utils::ImageHandle& r_Src, Utils::ImageLayout v_SrcLayout,
		const Utils::ImageHandle& r_Dst, Utils::ImageLayout v_DstLayout,
		uint32_t v_SrcWidth, uint32_t v_SrcHeight, uint32_t v_SrcMip,
		uint32_t v_DstWidth, uint32_t v_DstHeight, uint32_t v_DstMip,
		Utils::ImageAspect v_Aspect, Utils::FilterMode v_Filter)
	{
		VkImageBlit blit{};
		blit.srcSubresource.aspectMask = static_cast<VkImageAspectFlags>(v_Aspect);
		blit.srcSubresource.mipLevel   = v_SrcMip;
		blit.srcSubresource.layerCount = 1;
		blit.srcOffsets[1]             = { static_cast<int32_t>(v_SrcWidth), static_cast<int32_t>(v_SrcHeight), 1 };
		blit.dstSubresource.aspectMask = static_cast<VkImageAspectFlags>(v_Aspect);
		blit.dstSubresource.mipLevel   = v_DstMip;
		blit.dstSubresource.layerCount = 1;
		blit.dstOffsets[1]             = { static_cast<int32_t>(v_DstWidth), static_cast<int32_t>(v_DstHeight), 1 };

		vkCmdBlitImage(static_cast<VkCommandBuffer>(r_Cmd.m_Handle),
		               static_cast<VkImage>(r_Src.m_Handle), static_cast<VkImageLayout>(v_SrcLayout),
		               static_cast<VkImage>(r_Dst.m_Handle), static_cast<VkImageLayout>(v_DstLayout),
		               1, &blit,
		               static_cast<VkFilter>(v_Filter));
	}
}
