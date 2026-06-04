#include "SpectraVulkanBackend.h"
#include "VulkanDeferredRelease.h"
#include "VulkanAccelStructure.h"
#include "VulkanState.h"

namespace Spectra::Vulkan::Internal {

	DeferReleaseQueue g_DeferRelease;

	DeferReleaseQueue::DeferReleaseQueue() : m_CurrentSlot(0) {
		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i)
			m_Slots[i].reset();
	}

	void DeferReleaseQueue::deferFence(const Utils::FenceHandle& r_Handle) {
		FrameSlot& slot = m_Slots[m_CurrentSlot];
		SPEC_VK_BK_ASSERT(slot.m_FenceCount < MAX_FENCES);
		slot.m_Fences[slot.m_FenceCount++] = r_Handle;
	}

	void DeferReleaseQueue::deferSemaphore(const Utils::SemaphoreHandle& r_Handle) {
		FrameSlot& slot = m_Slots[m_CurrentSlot];
		SPEC_VK_BK_ASSERT(slot.m_SemaphoreCount < MAX_SEMAPHORES);
		slot.m_Semaphores[slot.m_SemaphoreCount++] = r_Handle;
	}

	void DeferReleaseQueue::deferBuffer(const Utils::BufferHandle& r_Handle) {
		FrameSlot& slot = m_Slots[m_CurrentSlot];
		SPEC_VK_BK_ASSERT(slot.m_BufferCount < MAX_BUFFERS);
		slot.m_Buffers[slot.m_BufferCount++] = r_Handle;
	}

	void DeferReleaseQueue::deferImage(const Utils::ImageHandle& r_Handle) {
		FrameSlot& slot = m_Slots[m_CurrentSlot];
		SPEC_VK_BK_ASSERT(slot.m_ImageCount < MAX_IMAGES);
		slot.m_Images[slot.m_ImageCount++] = r_Handle;
	}

	void DeferReleaseQueue::deferImageView(const Utils::ImageViewHandle& r_Handle) {
		FrameSlot& slot = m_Slots[m_CurrentSlot];
		SPEC_VK_BK_ASSERT(slot.m_ImageViewCount < MAX_IMAGE_VIEWS);
		slot.m_ImageViews[slot.m_ImageViewCount++] = r_Handle;
	}

	void DeferReleaseQueue::deferSampler(const Utils::SamplerHandle& r_Handle) {
		FrameSlot& slot = m_Slots[m_CurrentSlot];
		SPEC_VK_BK_ASSERT(slot.m_SamplerCount < MAX_SAMPLERS);
		slot.m_Samplers[slot.m_SamplerCount++] = r_Handle;
	}

	void DeferReleaseQueue::deferAccelStructure(const Utils::AccelerationStructureHandle& r_Handle) {
		FrameSlot& slot = m_Slots[m_CurrentSlot];
		SPEC_VK_BK_ASSERT(slot.m_AccelStructureCount < MAX_ACCEL_STRUCTURES);
		slot.m_AccelStructures[slot.m_AccelStructureCount++] = r_Handle;
	}

	void DeferReleaseQueue::advance() {
		m_CurrentSlot = (m_CurrentSlot + 1) % FRAMES_IN_FLIGHT;

		FrameSlot&   slot      = m_Slots[m_CurrentSlot];
		VkDevice     dev       = g_GlobalInstance.m_LogicalDevice.m_Device;
		VmaAllocator allocator = g_GlobalInstance.m_Allocator.m_Allocator;

		for (uint32_t i = 0; i < slot.m_FenceCount; ++i)
			vkDestroyFence(dev, static_cast<VkFence>(slot.m_Fences[i].m_Handle), nullptr);

		for (uint32_t i = 0; i < slot.m_SemaphoreCount; ++i)
			vkDestroySemaphore(dev, static_cast<VkSemaphore>(slot.m_Semaphores[i].m_Handle), nullptr);

		for (uint32_t i = 0; i < slot.m_BufferCount; ++i)
			vmaDestroyBuffer(allocator,
			                 static_cast<VkBuffer>(slot.m_Buffers[i].m_Handle),
			                 static_cast<VmaAllocation>(slot.m_Buffers[i].m_Allocation));

		for (uint32_t i = 0; i < slot.m_ImageCount; ++i)
			vmaDestroyImage(allocator,
			                static_cast<VkImage>(slot.m_Images[i].m_Handle),
			                static_cast<VmaAllocation>(slot.m_Images[i].m_Allocation));

		for (uint32_t i = 0; i < slot.m_ImageViewCount; ++i)
			vkDestroyImageView(dev, static_cast<VkImageView>(slot.m_ImageViews[i].m_Handle), nullptr);

		for (uint32_t i = 0; i < slot.m_SamplerCount; ++i)
			vkDestroySampler(dev, static_cast<VkSampler>(slot.m_Samplers[i].m_Handle), nullptr);

		for (uint32_t i = 0; i < slot.m_AccelStructureCount; ++i)
			VulkanAccelStructure::destroyAccelStructure(slot.m_AccelStructures[i]);

		slot.reset();
	}
}
