#include "SpectraVulkanBackend.h"
#include "VulkanQuery.h"
#include "VulkanInternalHelpers.h"
#include "VulkanState.h"

namespace Spectra::Vulkan {

	Utils::QueryPoolHandle VulkanQuery::createQueryPool(
		const Utils::QueryPoolDesc&           r_Desc,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;

		auto info        = Internal::vkInit<VkQueryPoolCreateInfo>();
		info.queryType   = static_cast<VkQueryType>(r_Desc.m_Type);
		info.queryCount  = r_Desc.m_Count;

		Utils::QueryPoolHandle handle{};
		vkCreateQueryPool(g_GlobalInstance.m_LogicalDevice.m_Device, &info, pA,
		                  reinterpret_cast<VkQueryPool*>(&handle.m_Handle));
		return handle;
	}

	void VulkanQuery::destroyQueryPool(
		const Utils::QueryPoolHandle&         r_Pool,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;
		vkDestroyQueryPool(g_GlobalInstance.m_LogicalDevice.m_Device,
		                   static_cast<VkQueryPool>(r_Pool.m_Handle), pA);
	}

	void VulkanQuery::cmdResetPool(const Utils::CommandBufferHandle& r_Cmd,
	                               const Utils::QueryPoolHandle&     r_Pool,
	                               uint32_t v_FirstQuery, uint32_t v_QueryCount)
	{
		vkCmdResetQueryPool(static_cast<VkCommandBuffer>(r_Cmd.m_Handle),
		                    static_cast<VkQueryPool>(r_Pool.m_Handle),
		                    v_FirstQuery, v_QueryCount);
	}

	void VulkanQuery::cmdWriteTimestamp(const Utils::CommandBufferHandle& r_Cmd,
	                                    const Utils::QueryPoolHandle&     r_Pool,
	                                    uint32_t                          v_QueryIndex,
	                                    Utils::PipelineStage              v_Stage)
	{
		vkCmdWriteTimestamp2(static_cast<VkCommandBuffer>(r_Cmd.m_Handle),
		                     static_cast<VkPipelineStageFlags2>(v_Stage),
		                     static_cast<VkQueryPool>(r_Pool.m_Handle),
		                     v_QueryIndex);
	}

	void VulkanQuery::getResults(const Utils::QueryPoolHandle& r_Pool,
	                             uint32_t v_FirstQuery, uint32_t v_QueryCount,
	                             uint64_t* p_Results, bool v_Wait)
	{
		VkQueryResultFlags flags = VK_QUERY_RESULT_64_BIT;
		if (v_Wait) flags |= VK_QUERY_RESULT_WAIT_BIT;

		vkGetQueryPoolResults(g_GlobalInstance.m_LogicalDevice.m_Device,
		                      static_cast<VkQueryPool>(r_Pool.m_Handle),
		                      v_FirstQuery, v_QueryCount,
		                      v_QueryCount * sizeof(uint64_t), p_Results,
		                      sizeof(uint64_t), flags);
	}
}
