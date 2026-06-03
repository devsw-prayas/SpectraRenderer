#include "SpectraVulkanBackend.h"
#include "VulkanDescriptors.h"
#include "VulkanInternalHelpers.h"
#include "VulkanState.h"

namespace Spectra::Vulkan {

	Utils::DescriptorSetLayoutHandle VulkanDescriptors::createDescriptorSetLayout(
		uint32_t v_BindingCount, const Utils::DescriptorSetLayoutBinding* p_Bindings,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		VkDescriptorSetLayoutBinding vkBindings[32];
		SPEC_VK_BK_ASSERT(v_BindingCount <= 32);
		for (uint32_t i = 0; i < v_BindingCount; ++i) {
			vkBindings[i].binding         = p_Bindings[i].m_Binding;
			vkBindings[i].descriptorType  = static_cast<VkDescriptorType>(p_Bindings[i].m_Type);
			vkBindings[i].descriptorCount = p_Bindings[i].m_Count;
			vkBindings[i].stageFlags      = static_cast<VkShaderStageFlags>(p_Bindings[i].m_StageFlags);
			vkBindings[i].pImmutableSamplers = nullptr;
		}

		auto info               = Internal::vkInit<VkDescriptorSetLayoutCreateInfo>();
		info.bindingCount       = v_BindingCount;
		info.pBindings          = vkBindings;

		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;

		Utils::DescriptorSetLayoutHandle handle{};
		vkCreateDescriptorSetLayout(g_GlobalInstance.m_LogicalDevice.m_Device, &info, pA,
		                            reinterpret_cast<VkDescriptorSetLayout*>(&handle.m_Handle));
		return handle;
	}

	void VulkanDescriptors::destroyDescriptorSetLayout(
		const Utils::DescriptorSetLayoutHandle& r_Layout,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;
		vkDestroyDescriptorSetLayout(g_GlobalInstance.m_LogicalDevice.m_Device,
		                             static_cast<VkDescriptorSetLayout>(r_Layout.m_Handle), pA);
	}

	Utils::DescriptorPoolHandle VulkanDescriptors::createDescriptorPool(
		uint32_t v_MaxSets, uint32_t v_PoolSizeCount,
		const Utils::DescriptorPoolSize* p_PoolSizes,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		VkDescriptorPoolSize vkSizes[32];
		SPEC_VK_BK_ASSERT(v_PoolSizeCount <= 32);
		for (uint32_t i = 0; i < v_PoolSizeCount; ++i) {
			vkSizes[i].type            = static_cast<VkDescriptorType>(p_PoolSizes[i].m_Type);
			vkSizes[i].descriptorCount = p_PoolSizes[i].m_Count;
		}

		auto info            = Internal::vkInit<VkDescriptorPoolCreateInfo>();
		info.flags           = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		info.maxSets         = v_MaxSets;
		info.poolSizeCount   = v_PoolSizeCount;
		info.pPoolSizes      = vkSizes;

		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;

		Utils::DescriptorPoolHandle handle{};
		vkCreateDescriptorPool(g_GlobalInstance.m_LogicalDevice.m_Device, &info, pA,
		                       reinterpret_cast<VkDescriptorPool*>(&handle.m_Handle));
		return handle;
	}

	void VulkanDescriptors::destroyDescriptorPool(
		const Utils::DescriptorPoolHandle& r_Pool,
		const Utils::AllocationCallbacksDesc& r_Alloc)
	{
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;
		vkDestroyDescriptorPool(g_GlobalInstance.m_LogicalDevice.m_Device,
		                        static_cast<VkDescriptorPool>(r_Pool.m_Handle), pA);
	}

	void VulkanDescriptors::resetDescriptorPool(const Utils::DescriptorPoolHandle& r_Pool) {
		vkResetDescriptorPool(g_GlobalInstance.m_LogicalDevice.m_Device,
		                      static_cast<VkDescriptorPool>(r_Pool.m_Handle), 0);
	}

	void VulkanDescriptors::allocateDescriptorSets(
		const Utils::DescriptorPoolHandle& r_Pool,
		uint32_t v_Count, const Utils::DescriptorSetLayoutHandle* p_Layouts,
		Utils::DescriptorSetHandle* p_Sets)
	{
		SPEC_VK_BK_ASSERT(v_Count <= MAX_SETS_PER_ALLOC);

		VkDescriptorSetLayout layouts[MAX_SETS_PER_ALLOC];
		for (uint32_t i = 0; i < v_Count; ++i)
			layouts[i] = static_cast<VkDescriptorSetLayout>(p_Layouts[i].m_Handle);

		auto info             = Internal::vkInit<VkDescriptorSetAllocateInfo>();
		info.descriptorPool   = static_cast<VkDescriptorPool>(r_Pool.m_Handle);
		info.descriptorSetCount = v_Count;
		info.pSetLayouts      = layouts;

		VkDescriptorSet sets[MAX_SETS_PER_ALLOC];
		vkAllocateDescriptorSets(g_GlobalInstance.m_LogicalDevice.m_Device, &info, sets);
		for (uint32_t i = 0; i < v_Count; ++i)
			p_Sets[i].m_Handle = static_cast<void*>(sets[i]);
	}

	void VulkanDescriptors::updateDescriptorSets(
		uint32_t v_WriteCount, const Utils::WriteDescriptorSet* p_Writes)
	{
		SPEC_VK_BK_ASSERT(v_WriteCount <= MAX_WRITES);

		VkWriteDescriptorSet                    vkWrites  [MAX_WRITES];
		VkDescriptorBufferInfo                  bufPool   [MAX_INFOS_PER_CALL];
		VkDescriptorImageInfo                   imgPool   [MAX_INFOS_PER_CALL];
		VkAccelerationStructureKHR              asHandles [MAX_INFOS_PER_CALL];
		VkWriteDescriptorSetAccelerationStructureKHR asExts[MAX_WRITES];

		uint32_t bufOffset = 0, imgOffset = 0, asOffset = 0;

		for (uint32_t i = 0; i < v_WriteCount; ++i) {
			const Utils::WriteDescriptorSet& w = p_Writes[i];
			VkWriteDescriptorSet& vw = vkWrites[i];
			vw = Internal::vkInit<VkWriteDescriptorSet>();
			vw.dstSet          = static_cast<VkDescriptorSet>(w.m_DstSet.m_Handle);
			vw.dstBinding      = w.m_Binding;
			vw.dstArrayElement = w.m_ArrayElement;
			vw.descriptorCount = w.m_Count;
			vw.descriptorType  = static_cast<VkDescriptorType>(w.m_Type);

			if (w.m_pBufferInfo != nullptr) {
				SPEC_VK_BK_ASSERT(bufOffset + w.m_Count <= MAX_INFOS_PER_CALL);
				vw.pBufferInfo = &bufPool[bufOffset];
				for (uint32_t j = 0; j < w.m_Count; ++j) {
					bufPool[bufOffset + j].buffer = static_cast<VkBuffer>(w.m_pBufferInfo[j].m_Buffer.m_Handle);
					bufPool[bufOffset + j].offset = w.m_pBufferInfo[j].m_Offset;
					bufPool[bufOffset + j].range  = w.m_pBufferInfo[j].m_Range;
				}
				bufOffset += w.m_Count;
			} else if (w.m_pImageInfo != nullptr) {
				SPEC_VK_BK_ASSERT(imgOffset + w.m_Count <= MAX_INFOS_PER_CALL);
				vw.pImageInfo = &imgPool[imgOffset];
				for (uint32_t j = 0; j < w.m_Count; ++j) {
					imgPool[imgOffset + j].sampler     = static_cast<VkSampler>    (w.m_pImageInfo[j].m_Sampler.m_Handle);
					imgPool[imgOffset + j].imageView   = static_cast<VkImageView>  (w.m_pImageInfo[j].m_ImageView.m_Handle);
					imgPool[imgOffset + j].imageLayout = static_cast<VkImageLayout>(w.m_pImageInfo[j].m_Layout);
				}
				imgOffset += w.m_Count;
			} else if (w.m_pAccelerationStructures != nullptr) {
				SPEC_VK_BK_ASSERT(asOffset + w.m_Count <= MAX_INFOS_PER_CALL);
				for (uint32_t j = 0; j < w.m_Count; ++j)
					asHandles[asOffset + j] = static_cast<VkAccelerationStructureKHR>(w.m_pAccelerationStructures[j].m_Handle);
				asExts[i] = Internal::vkInit<VkWriteDescriptorSetAccelerationStructureKHR>();
				asExts[i].accelerationStructureCount = w.m_Count;
				asExts[i].pAccelerationStructures    = &asHandles[asOffset];
				vw.pNext = &asExts[i];
				asOffset += w.m_Count;
			}
		}

		vkUpdateDescriptorSets(g_GlobalInstance.m_LogicalDevice.m_Device,
		                       v_WriteCount, vkWrites, 0, nullptr);
	}
}
