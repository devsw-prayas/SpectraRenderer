#pragma once

#include "VulkanUtils.h"

namespace Spectra::Vulkan {

	class SPEC_VK_BK_RUNTIME_API VulkanAccelStructure final {
	public:
		static constexpr uint32_t MAX_GEOMETRIES = 32;

		// Returns the backing buffer size and scratch buffer size for the given build.
		// r_Desc.m_PrimitiveCounts must be non-null with one entry per geometry.
		static Utils::ASBuildSizes getBuildSizes(const Utils::ASBuildDesc& r_Desc);

		// Creates the backing VkBuffer (ACCELERATION_STRUCTURE_STORAGE | SHADER_DEVICE_ADDRESS)
		// and the VkAccelerationStructureKHR layered on top of it. v_Size must come from
		// ASBuildSizes::m_AccelStructureSize returned by getBuildSizes.
		static Utils::AccelerationStructureHandle createAccelStructure(
			Utils::AccelerationStructureType       v_Type,
			Utils::AccelerationStructureBuildFlags v_Flags,
			uint64_t                               v_Size);

		// Returns the device address for use in instance data and descriptor writes.
		static uint64_t getDeviceAddress(const Utils::AccelerationStructureHandle& r_AS);

		// Records vkCmdBuildAccelerationStructuresKHR into r_Cmd.
		// r_Desc.m_Dst must be valid; r_Desc.m_PrimitiveCounts must be non-null;
		// r_Desc.m_ScratchDeviceAddress must point to >= ASBuildSizes::m_ScratchSize bytes.
		static void cmdBuild(const Utils::CommandBufferHandle& r_Cmd,
		                     const Utils::ASBuildDesc&         r_Desc);

		// Records vkCmdWriteAccelerationStructuresPropertiesKHR for a compacted-size query.
		// The query pool must be of type ACCELERATION_STRUCTURE_COMPACTED_SIZE.
		static void cmdWriteCompactedSize(const Utils::CommandBufferHandle&        r_Cmd,
		                                  const Utils::AccelerationStructureHandle& r_AS,
		                                  const Utils::QueryPoolHandle&             r_Pool,
		                                  uint32_t                                  v_FirstQuery);

		// Records a COMPACT copy of r_Src into r_Dst.
		// r_Dst must have been created with a size >= the compacted size from the query.
		static void cmdCompact(const Utils::CommandBufferHandle&        r_Cmd,
		                       const Utils::AccelerationStructureHandle& r_Src,
		                       const Utils::AccelerationStructureHandle& r_Dst);

		// Destroys the VkAccelerationStructureKHR and its backing VkBuffer/VmaAllocation.
		static void destroyAccelStructure(const Utils::AccelerationStructureHandle& r_AS);

		VulkanAccelStructure() = delete;
		~VulkanAccelStructure() = delete;
		VulkanAccelStructure(const VulkanAccelStructure&) = delete;
		VulkanAccelStructure& operator=(const VulkanAccelStructure&) = delete;
		VulkanAccelStructure(VulkanAccelStructure&&) noexcept = delete;
		VulkanAccelStructure& operator=(VulkanAccelStructure&&) noexcept = delete;
	};
}
