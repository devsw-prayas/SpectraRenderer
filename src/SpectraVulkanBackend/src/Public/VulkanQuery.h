#pragma once

#include "VulkanUtils.h"

namespace Spectra::Vulkan {

	class SPEC_VK_BK_RUNTIME_API VulkanQuery final {
	public:
		// Creates a query pool of the given type and count.
		// For ACCELERATION_STRUCTURE_COMPACTED_SIZE queries use QueryType::ACCELERATION_STRUCTURE_COMPACTED_SIZE.
		static Utils::QueryPoolHandle createQueryPool(
			const Utils::QueryPoolDesc&           r_Desc,
			const Utils::AllocationCallbacksDesc& r_Alloc);

		static void destroyQueryPool(
			const Utils::QueryPoolHandle&         r_Pool,
			const Utils::AllocationCallbacksDesc& r_Alloc);

		// Records vkCmdResetQueryPool.
		static void cmdResetPool(const Utils::CommandBufferHandle& r_Cmd,
		                         const Utils::QueryPoolHandle&     r_Pool,
		                         uint32_t                          v_FirstQuery,
		                         uint32_t                          v_QueryCount);

		// Records vkCmdWriteTimestamp2 (synchronisation2 path).
		static void cmdWriteTimestamp(const Utils::CommandBufferHandle& r_Cmd,
		                              const Utils::QueryPoolHandle&     r_Pool,
		                              uint32_t                          v_QueryIndex,
		                              Utils::PipelineStage              v_Stage);

		// Reads v_QueryCount 64-bit results starting at v_FirstQuery into p_Results.
		// If v_Wait is true, blocks until results are available.
		static void getResults(const Utils::QueryPoolHandle& r_Pool,
		                       uint32_t                      v_FirstQuery,
		                       uint32_t                      v_QueryCount,
		                       uint64_t*                     p_Results,
		                       bool                          v_Wait);

		VulkanQuery() = delete;
		~VulkanQuery() = delete;
		VulkanQuery(const VulkanQuery&) = delete;
		VulkanQuery& operator=(const VulkanQuery&) = delete;
		VulkanQuery(VulkanQuery&&) noexcept = delete;
		VulkanQuery& operator=(VulkanQuery&&) noexcept = delete;
	};
}
