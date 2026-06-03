#include "SpectraVulkanBackend.h"
#include "VulkanSync.h"

#include "VulkanInternalHelpers.h"
#include "VulkanState.h"

namespace Spectra::Vulkan::Sync {
	Utils::FenceHandle VulkanFence::createFence(const Utils::FenceDesc& r_Desc, const Utils::AllocationCallbacksDesc& r_Alloc) {
		VkFenceCreateInfo info = Internal::vkInit<VkFenceCreateInfo>();
		info.flags = r_Desc.m_PreSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;

		Utils::FenceHandle handle{};
		vkCreateFence(g_GlobalInstance.m_LogicalDevice.m_Device, &info, pA, reinterpret_cast<VkFence*>(&handle.m_Handle));
		return handle;
	}

	void VulkanFence::waitForFences(uint32_t v_Count, const Utils::FenceHandle* p_Fences, bool v_WaitAll, uint64_t v_Timeout) {
		vkWaitForFences(g_GlobalInstance.m_LogicalDevice.m_Device, v_Count, reinterpret_cast<const VkFence*>(p_Fences), v_WaitAll ? VK_TRUE : VK_FALSE, v_Timeout);
	}

	void VulkanFence::resetFences(uint32_t v_Count, const Utils::FenceHandle* p_Fences) {
		vkResetFences(g_GlobalInstance.m_LogicalDevice.m_Device, v_Count, reinterpret_cast<const VkFence*>(p_Fences));
	}

	bool VulkanFence::getFenceStatus(const Utils::FenceHandle& r_Fence) {
		return vkGetFenceStatus(g_GlobalInstance.m_LogicalDevice.m_Device, static_cast<VkFence>(r_Fence.m_Handle)) == VK_SUCCESS;
	}

	void VulkanFence::destroyFence(const Utils::FenceHandle& r_Fence, const Utils::AllocationCallbacksDesc& r_Alloc) {
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;
		vkDestroyFence(g_GlobalInstance.m_LogicalDevice.m_Device, static_cast<VkFence>(r_Fence.m_Handle), pA);
	}

	Utils::SemaphoreHandle VulkanTimelineSemaphore::create(const Utils::TimelineSemaphoreDesc& r_Desc, const Utils::AllocationCallbacksDesc& r_Alloc) {
		auto typeInfo = Internal::vkInit<VkSemaphoreTypeCreateInfo>();
		typeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		typeInfo.initialValue  = r_Desc.m_InitialValue;

		auto exportInfo = Internal::vkInit<VkExportSemaphoreCreateInfo>();
		exportInfo.handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;

		auto createInfo = Internal::vkInit<VkSemaphoreCreateInfo>();
		createInfo.pNext = &typeInfo;
		if (r_Desc.m_ExportForCuda)
			typeInfo.pNext = &exportInfo;

		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;

		Utils::SemaphoreHandle handle{};
		vkCreateSemaphore(g_GlobalInstance.m_LogicalDevice.m_Device, &createInfo, pA, reinterpret_cast<VkSemaphore*>(&handle.m_Handle));
		return handle;
	}

	void VulkanTimelineSemaphore::signal(const Utils::SemaphoreHandle& r_Sem, uint64_t v_Value) {
		auto info      = Internal::vkInit<VkSemaphoreSignalInfo>();
		info.semaphore = static_cast<VkSemaphore>(r_Sem.m_Handle);
		info.value     = v_Value;
		vkSignalSemaphore(g_GlobalInstance.m_LogicalDevice.m_Device, &info);
	}

	void VulkanTimelineSemaphore::wait(const Utils::SemaphoreHandle& r_Sem, uint64_t v_Value, uint64_t v_Timeout) {
		VkSemaphore vkSem = static_cast<VkSemaphore>(r_Sem.m_Handle);
		auto info         = Internal::vkInit<VkSemaphoreWaitInfo>();
		info.semaphoreCount = 1;
		info.pSemaphores    = &vkSem;
		info.pValues        = &v_Value;
		vkWaitSemaphores(g_GlobalInstance.m_LogicalDevice.m_Device, &info, v_Timeout);
	}

	uint64_t VulkanTimelineSemaphore::getCounter(const Utils::SemaphoreHandle& r_Sem) {
		uint64_t value = 0;
		vkGetSemaphoreCounterValue(g_GlobalInstance.m_LogicalDevice.m_Device, static_cast<VkSemaphore>(r_Sem.m_Handle), &value);
		return value;
	}

	void* VulkanTimelineSemaphore::getWin32Handle(const Utils::SemaphoreHandle& r_Sem) {
		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
		auto fn = reinterpret_cast<PFN_vkGetSemaphoreWin32HandleKHR>(
			vkGetDeviceProcAddr(dev, "vkGetSemaphoreWin32HandleKHR"));
		SPEC_VK_BK_ASSERT(fn != nullptr);

		auto info      = Internal::vkInit<VkSemaphoreGetWin32HandleInfoKHR>();
		info.semaphore = static_cast<VkSemaphore>(r_Sem.m_Handle);
		info.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;

		HANDLE win32Handle = nullptr;
		fn(dev, &info, &win32Handle);
		return win32Handle;
	}

	void VulkanTimelineSemaphore::destroy(const Utils::SemaphoreHandle& r_Sem, const Utils::AllocationCallbacksDesc& r_Alloc) {
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;
		vkDestroySemaphore(g_GlobalInstance.m_LogicalDevice.m_Device, static_cast<VkSemaphore>(r_Sem.m_Handle), pA);
	}

	void VulkanBarrier::cmdPipelineBarrier(
		const Utils::CommandBufferHandle& r_Cmd,
		uint32_t v_MemCount,  const Utils::MemoryBarrierDesc* p_Mem,
		uint32_t v_BufCount,  const Utils::BufferBarrierDesc* p_Buf,
		uint32_t v_ImgCount,  const Utils::ImageBarrierDesc*  p_Img)
	{
		constexpr uint32_t MAX_BARRIERS = 32;
		SPEC_VK_BK_ASSERT(v_MemCount <= MAX_BARRIERS);
		SPEC_VK_BK_ASSERT(v_BufCount <= MAX_BARRIERS);
		SPEC_VK_BK_ASSERT(v_ImgCount <= MAX_BARRIERS);

		VkMemoryBarrier2       memBarriers[MAX_BARRIERS];
		VkBufferMemoryBarrier2 bufBarriers[MAX_BARRIERS];
		VkImageMemoryBarrier2  imgBarriers[MAX_BARRIERS];

		for (uint32_t i = 0; i < v_MemCount; ++i) {
			memBarriers[i]              = Internal::vkInit<VkMemoryBarrier2>();
			memBarriers[i].srcStageMask  = static_cast<VkPipelineStageFlags2>(p_Mem[i].m_SrcStage);
			memBarriers[i].dstStageMask  = static_cast<VkPipelineStageFlags2>(p_Mem[i].m_DstStage);
			memBarriers[i].srcAccessMask = static_cast<VkAccessFlags2>(p_Mem[i].m_SrcAccess);
			memBarriers[i].dstAccessMask = static_cast<VkAccessFlags2>(p_Mem[i].m_DstAccess);
		}

		for (uint32_t i = 0; i < v_BufCount; ++i) {
			bufBarriers[i]                      = Internal::vkInit<VkBufferMemoryBarrier2>();
			bufBarriers[i].srcStageMask          = static_cast<VkPipelineStageFlags2>(p_Buf[i].m_SrcStage);
			bufBarriers[i].dstStageMask          = static_cast<VkPipelineStageFlags2>(p_Buf[i].m_DstStage);
			bufBarriers[i].srcAccessMask         = static_cast<VkAccessFlags2>(p_Buf[i].m_SrcAccess);
			bufBarriers[i].dstAccessMask         = static_cast<VkAccessFlags2>(p_Buf[i].m_DstAccess);
			bufBarriers[i].srcQueueFamilyIndex   = p_Buf[i].m_SrcQueueFamily;
			bufBarriers[i].dstQueueFamilyIndex   = p_Buf[i].m_DstQueueFamily;
			bufBarriers[i].buffer                = static_cast<VkBuffer>(p_Buf[i].m_Buffer.m_Handle);
			bufBarriers[i].offset                = p_Buf[i].m_Offset;
			bufBarriers[i].size                  = p_Buf[i].m_Size;
		}

		for (uint32_t i = 0; i < v_ImgCount; ++i) {
			imgBarriers[i]                                    = Internal::vkInit<VkImageMemoryBarrier2>();
			imgBarriers[i].srcStageMask                        = static_cast<VkPipelineStageFlags2>(p_Img[i].m_SrcStage);
			imgBarriers[i].dstStageMask                        = static_cast<VkPipelineStageFlags2>(p_Img[i].m_DstStage);
			imgBarriers[i].srcAccessMask                       = static_cast<VkAccessFlags2>(p_Img[i].m_SrcAccess);
			imgBarriers[i].dstAccessMask                       = static_cast<VkAccessFlags2>(p_Img[i].m_DstAccess);
			imgBarriers[i].oldLayout                           = static_cast<VkImageLayout>(p_Img[i].m_OldLayout);
			imgBarriers[i].newLayout                           = static_cast<VkImageLayout>(p_Img[i].m_NewLayout);
			imgBarriers[i].srcQueueFamilyIndex                 = p_Img[i].m_SrcQueueFamily;
			imgBarriers[i].dstQueueFamilyIndex                 = p_Img[i].m_DstQueueFamily;
			imgBarriers[i].image                               = static_cast<VkImage>(p_Img[i].m_Image.m_Handle);
			imgBarriers[i].subresourceRange.aspectMask         = static_cast<VkImageAspectFlags>(p_Img[i].m_Aspect);
			imgBarriers[i].subresourceRange.baseMipLevel       = p_Img[i].m_BaseMip;
			imgBarriers[i].subresourceRange.levelCount         = p_Img[i].m_MipCount;
			imgBarriers[i].subresourceRange.baseArrayLayer     = p_Img[i].m_BaseLayer;
			imgBarriers[i].subresourceRange.layerCount         = p_Img[i].m_LayerCount;
		}

		auto dep = Internal::vkInit<VkDependencyInfo>();
		dep.memoryBarrierCount        = v_MemCount;
		dep.pMemoryBarriers           = v_MemCount ? memBarriers : nullptr;
		dep.bufferMemoryBarrierCount  = v_BufCount;
		dep.pBufferMemoryBarriers     = v_BufCount ? bufBarriers : nullptr;
		dep.imageMemoryBarrierCount   = v_ImgCount;
		dep.pImageMemoryBarriers      = v_ImgCount ? imgBarriers : nullptr;

		vkCmdPipelineBarrier2(static_cast<VkCommandBuffer>(r_Cmd.m_Handle), &dep);
	}

	void VulkanBarrier::cmdMemoryBarrier(const Utils::CommandBufferHandle& r_Cmd, const Utils::MemoryBarrierDesc& r_Barrier) {
		cmdPipelineBarrier(r_Cmd, 1, &r_Barrier, 0, nullptr, 0, nullptr);
	}

	void VulkanBarrier::cmdBufferBarrier(const Utils::CommandBufferHandle& r_Cmd, const Utils::BufferBarrierDesc& r_Barrier) {
		cmdPipelineBarrier(r_Cmd, 0, nullptr, 1, &r_Barrier, 0, nullptr);
	}

	void VulkanBarrier::cmdImageBarrier(const Utils::CommandBufferHandle& r_Cmd, const Utils::ImageBarrierDesc& r_Barrier) {
		cmdPipelineBarrier(r_Cmd, 0, nullptr, 0, nullptr, 1, &r_Barrier);
	}
}