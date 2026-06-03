#include "SpectraVulkanBackend.h"
#include "VulkanQueue.h"
#include "VulkanInternalHelpers.h"
#include "VulkanState.h"

namespace Spectra::Vulkan {

	static VkQueue queueForType(Utils::QueueType v_Type) {
		switch (v_Type) {
		case Utils::QueueType::COMPUTE:  return g_GlobalInstance.m_LogicalDevice.m_ComputeQueue;
		case Utils::QueueType::TRANSFER: return g_GlobalInstance.m_LogicalDevice.m_TransferQueue;
		default:                         return g_GlobalInstance.m_LogicalDevice.m_GraphicsQueue;
		}
	}

	void VulkanQueue::submit(
		Utils::QueueType v_QueueType,
		const Utils::SubmitDesc& r_Desc,
		const Utils::FenceHandle& r_Fence)
	{
		SPEC_VK_BK_ASSERT(r_Desc.m_WaitCount          <= MAX_SUBMIT_SEMAPHORES);
		SPEC_VK_BK_ASSERT(r_Desc.m_SignalCount         <= MAX_SUBMIT_SEMAPHORES);
		SPEC_VK_BK_ASSERT(r_Desc.m_CommandBufferCount  <= MAX_SUBMIT_CMDS);

		VkSemaphoreSubmitInfo       waitInfos  [MAX_SUBMIT_SEMAPHORES];
		VkSemaphoreSubmitInfo       signalInfos[MAX_SUBMIT_SEMAPHORES];
		VkCommandBufferSubmitInfo   cmdInfos   [MAX_SUBMIT_CMDS];

		for (uint32_t i = 0; i < r_Desc.m_WaitCount; ++i) {
			waitInfos[i]           = Internal::vkInit<VkSemaphoreSubmitInfo>();
			waitInfos[i].semaphore = static_cast<VkSemaphore>(r_Desc.m_WaitSemaphores[i].m_Handle);
			waitInfos[i].value     = r_Desc.m_WaitValues  ? r_Desc.m_WaitValues [i] : 0;
			waitInfos[i].stageMask = r_Desc.m_WaitStages  ? static_cast<VkPipelineStageFlags2>(r_Desc.m_WaitStages[i])
			                                              : VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		}

		for (uint32_t i = 0; i < r_Desc.m_SignalCount; ++i) {
			signalInfos[i]           = Internal::vkInit<VkSemaphoreSubmitInfo>();
			signalInfos[i].semaphore = static_cast<VkSemaphore>(r_Desc.m_SignalSemaphores[i].m_Handle);
			signalInfos[i].value     = r_Desc.m_SignalValues  ? r_Desc.m_SignalValues[i] : 0;
			signalInfos[i].stageMask = r_Desc.m_SignalStages  ? static_cast<VkPipelineStageFlags2>(r_Desc.m_SignalStages[i])
			                                                  : VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		}

		for (uint32_t i = 0; i < r_Desc.m_CommandBufferCount; ++i) {
			cmdInfos[i]               = Internal::vkInit<VkCommandBufferSubmitInfo>();
			cmdInfos[i].commandBuffer = static_cast<VkCommandBuffer>(r_Desc.m_CommandBuffers[i].m_Handle);
		}

		auto submit2                       = Internal::vkInit<VkSubmitInfo2>();
		submit2.waitSemaphoreInfoCount     = r_Desc.m_WaitCount;
		submit2.pWaitSemaphoreInfos        = r_Desc.m_WaitCount   ? waitInfos   : nullptr;
		submit2.commandBufferInfoCount     = r_Desc.m_CommandBufferCount;
		submit2.pCommandBufferInfos        = r_Desc.m_CommandBufferCount ? cmdInfos : nullptr;
		submit2.signalSemaphoreInfoCount   = r_Desc.m_SignalCount;
		submit2.pSignalSemaphoreInfos      = r_Desc.m_SignalCount  ? signalInfos : nullptr;

		vkQueueSubmit2(queueForType(v_QueueType), 1, &submit2,
		               static_cast<VkFence>(r_Fence.m_Handle));
	}

	void VulkanQueue::waitIdle(Utils::QueueType v_QueueType) {
		vkQueueWaitIdle(queueForType(v_QueueType));
	}

	void VulkanQueue::deviceWaitIdle() {
		vkDeviceWaitIdle(g_GlobalInstance.m_LogicalDevice.m_Device);
	}
}
