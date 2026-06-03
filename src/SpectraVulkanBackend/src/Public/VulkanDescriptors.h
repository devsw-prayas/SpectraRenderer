#pragma once

#include "VulkanUtils.h"

namespace Spectra::Vulkan {

	class SPEC_VK_BK_RUNTIME_API VulkanDescriptors final {
	public:
		static constexpr uint32_t MAX_SETS_PER_ALLOC  = 16;
		static constexpr uint32_t MAX_WRITES          = 64;
		static constexpr uint32_t MAX_INFOS_PER_CALL  = 256;

		static Utils::DescriptorSetLayoutHandle createDescriptorSetLayout(
			uint32_t v_BindingCount, const Utils::DescriptorSetLayoutBinding* p_Bindings,
			const Utils::AllocationCallbacksDesc& r_Alloc);
		static void destroyDescriptorSetLayout(
			const Utils::DescriptorSetLayoutHandle& r_Layout,
			const Utils::AllocationCallbacksDesc& r_Alloc);

		static Utils::DescriptorPoolHandle createDescriptorPool(
			uint32_t v_MaxSets,
			uint32_t v_PoolSizeCount, const Utils::DescriptorPoolSize* p_PoolSizes,
			const Utils::AllocationCallbacksDesc& r_Alloc);
		static void destroyDescriptorPool(
			const Utils::DescriptorPoolHandle& r_Pool,
			const Utils::AllocationCallbacksDesc& r_Alloc);
		static void resetDescriptorPool(const Utils::DescriptorPoolHandle& r_Pool);

		// v_Count must be <= MAX_SETS_PER_ALLOC.
		static void allocateDescriptorSets(
			const Utils::DescriptorPoolHandle& r_Pool,
			uint32_t v_Count, const Utils::DescriptorSetLayoutHandle* p_Layouts,
			Utils::DescriptorSetHandle* p_Sets);

		// v_WriteCount must be <= MAX_WRITES.
		// For each write, set exactly one of m_pBufferInfo / m_pImageInfo / m_pAccelerationStructures.
		// Total buffer + image descriptors across all writes must be <= MAX_INFOS_PER_CALL.
		static void updateDescriptorSets(
			uint32_t v_WriteCount, const Utils::WriteDescriptorSet* p_Writes);

		VulkanDescriptors() = delete;
		~VulkanDescriptors() = delete;
		VulkanDescriptors(const VulkanDescriptors&) = delete;
		VulkanDescriptors& operator=(const VulkanDescriptors&) = delete;
		VulkanDescriptors(VulkanDescriptors&&) noexcept = delete;
		VulkanDescriptors& operator=(VulkanDescriptors&&) noexcept = delete;
	};
}
