#pragma once

#include "VulkanUtils.h"

namespace Spectra::Vulkan {

	class SPEC_VK_BK_RUNTIME_API VulkanQueue final {
	public:
		static constexpr uint32_t MAX_SUBMIT_SEMAPHORES = 16;
		static constexpr uint32_t MAX_SUBMIT_CMDS       = 8;

		// Submits one SubmitDesc batch on the queue selected by v_QueueType.
		// r_Fence is signalled when all work in this batch completes; pass an empty FenceHandle{} to skip.
		static void submit(Utils::QueueType v_QueueType,
		                   const Utils::SubmitDesc& r_Desc,
		                   const Utils::FenceHandle& r_Fence);

		// Blocks the CPU until the selected queue drains.
		static void waitIdle(Utils::QueueType v_QueueType);

		// Blocks the CPU until all queues on the device drain.
		static void deviceWaitIdle();

		VulkanQueue() = delete;
		~VulkanQueue() = delete;
		VulkanQueue(const VulkanQueue&) = delete;
		VulkanQueue& operator=(const VulkanQueue&) = delete;
		VulkanQueue(VulkanQueue&&) noexcept = delete;
		VulkanQueue& operator=(VulkanQueue&&) noexcept = delete;
	};
}
