#pragma once
#include "SpectraVulkanBackend.h"
#include "VulkanUtils.h"

namespace Spectra::Vulkan::Sync {
	class SPEC_VK_BK_RUNTIME_API VulkanFence final {
	public:
		static Utils::FenceHandle createFence(const Utils::FenceDesc& r_Desc, const Utils::AllocationCallbacksDesc& r_Alloc);
		static void               waitForFences(uint32_t v_Count, const Utils::FenceHandle* p_Fences, bool v_WaitAll, uint64_t v_Timeout);
		static void               resetFences(uint32_t v_Count, const Utils::FenceHandle* p_Fences);
		static bool               getFenceStatus(const Utils::FenceHandle& r_Fence);
		static void               destroyFence(const Utils::FenceHandle& r_Fence, const Utils::AllocationCallbacksDesc& r_Alloc);
	};

	class SPEC_VK_BK_RUNTIME_API VulkanTimelineSemaphore final {
	public:
		static Utils::SemaphoreHandle create(const Utils::TimelineSemaphoreDesc& r_Desc, const Utils::AllocationCallbacksDesc& r_Alloc);
		static void                   signal(const Utils::SemaphoreHandle& r_Sem, uint64_t v_Value);
		static void                   wait(const Utils::SemaphoreHandle& r_Sem, uint64_t v_Value, uint64_t v_Timeout);
		static uint64_t               getCounter(const Utils::SemaphoreHandle& r_Sem);
		static void*                  getWin32Handle(const Utils::SemaphoreHandle& r_Sem);
		static void                   destroy(const Utils::SemaphoreHandle& r_Sem, const Utils::AllocationCallbacksDesc& r_Alloc);
	};

	class SPEC_VK_BK_RUNTIME_API VulkanBarrier final {
	public:
		static void cmdMemoryBarrier(const Utils::CommandBufferHandle& r_Cmd, const Utils::MemoryBarrierDesc& r_Barrier);
		static void cmdBufferBarrier(const Utils::CommandBufferHandle& r_Cmd, const Utils::BufferBarrierDesc& r_Barrier);
		static void cmdImageBarrier (const Utils::CommandBufferHandle& r_Cmd, const Utils::ImageBarrierDesc& r_Barrier);
		static void cmdPipelineBarrier(const Utils::CommandBufferHandle& r_Cmd,
		                               uint32_t v_MemCount,  const Utils::MemoryBarrierDesc* p_Mem,
		                               uint32_t v_BufCount,  const Utils::BufferBarrierDesc* p_Buf,
		                               uint32_t v_ImgCount,  const Utils::ImageBarrierDesc*  p_Img);
	};
}
