#include "SpectraVulkanBackend.h"
#include "VulkanAccelStructure.h"
#include "VulkanInternalHelpers.h"
#include "VulkanState.h"

namespace Spectra::Vulkan {

	static void fillGeomArray(const Utils::ASBuildDesc& r_Desc,
	                          VkAccelerationStructureGeometryKHR* p_Out) {
		if (r_Desc.m_Instances != nullptr) {
			for (uint32_t i = 0; i < r_Desc.m_GeometryCount; ++i) {
				p_Out[i]              = Internal::vkInit<VkAccelerationStructureGeometryKHR>();
				p_Out[i].geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
				p_Out[i].flags        = r_Desc.m_GeometryFlags
				                          ? static_cast<VkGeometryFlagsKHR>(r_Desc.m_GeometryFlags[i])
				                          : VK_GEOMETRY_OPAQUE_BIT_KHR;

				auto& inst            = p_Out[i].geometry.instances;
				inst                  = Internal::vkInit<VkAccelerationStructureGeometryInstancesDataKHR>();
				inst.arrayOfPointers  = r_Desc.m_Instances[i].m_ArrayOfPointers ? VK_TRUE : VK_FALSE;
				inst.data.deviceAddress = r_Desc.m_Instances[i].m_InstancesDeviceAddress;
			}
		} else {
			for (uint32_t i = 0; i < r_Desc.m_GeometryCount; ++i) {
				p_Out[i]              = Internal::vkInit<VkAccelerationStructureGeometryKHR>();
				p_Out[i].geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
				p_Out[i].flags        = r_Desc.m_GeometryFlags
				                          ? static_cast<VkGeometryFlagsKHR>(r_Desc.m_GeometryFlags[i])
				                          : VK_GEOMETRY_OPAQUE_BIT_KHR;

				auto& tri                   = p_Out[i].geometry.triangles;
				tri                         = Internal::vkInit<VkAccelerationStructureGeometryTrianglesDataKHR>();
				tri.vertexFormat            = static_cast<VkFormat>(r_Desc.m_Geometries[i].m_VertexFormat);
				tri.vertexData.deviceAddress = r_Desc.m_Geometries[i].m_VertexDeviceAddress;
				tri.vertexStride            = r_Desc.m_Geometries[i].m_VertexStride;
				tri.maxVertex               = r_Desc.m_Geometries[i].m_MaxVertex;
				tri.indexType               = static_cast<VkIndexType>(r_Desc.m_Geometries[i].m_IndexType);
				tri.indexData.deviceAddress = r_Desc.m_Geometries[i].m_IndexDeviceAddress;
				tri.transformData.deviceAddress = 0;
			}
		}
	}

	// -------------------------------------------------------------------------
	// VulkanAccelStructure
	// -------------------------------------------------------------------------

	Utils::ASBuildSizes VulkanAccelStructure::getBuildSizes(const Utils::ASBuildDesc& r_Desc) {
		SPEC_VK_BK_ASSERT(r_Desc.m_GeometryCount <= MAX_GEOMETRIES);
		SPEC_VK_BK_ASSERT(r_Desc.m_PrimitiveCounts != nullptr);

		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
		static auto pfn = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
			vkGetDeviceProcAddr(dev, "vkGetAccelerationStructureBuildSizesKHR"));
		SPEC_VK_BK_ASSERT(pfn != nullptr);

		VkAccelerationStructureGeometryKHR geoms[MAX_GEOMETRIES];
		fillGeomArray(r_Desc, geoms);

		auto buildInfo          = Internal::vkInit<VkAccelerationStructureBuildGeometryInfoKHR>();
		buildInfo.type          = static_cast<VkAccelerationStructureTypeKHR>(r_Desc.m_Type);
		buildInfo.flags         = static_cast<VkBuildAccelerationStructureFlagsKHR>(r_Desc.m_Flags);
		buildInfo.geometryCount = r_Desc.m_GeometryCount;
		buildInfo.pGeometries   = geoms;

		VkAccelerationStructureBuildSizesInfoKHR sizes{};
		sizes.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		pfn(dev, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		    &buildInfo, r_Desc.m_PrimitiveCounts, &sizes);

		Utils::ASBuildSizes result{};
		result.m_AccelStructureSize = sizes.accelerationStructureSize;
		result.m_ScratchSize        = sizes.buildScratchSize;
		return result;
	}

	Utils::AccelerationStructureHandle VulkanAccelStructure::createAccelStructure(
		Utils::AccelerationStructureType       v_Type,
		Utils::AccelerationStructureBuildFlags v_Flags,
		uint64_t                               v_Size)
	{
		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
		static auto pfn = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
			vkGetDeviceProcAddr(dev, "vkCreateAccelerationStructureKHR"));
		SPEC_VK_BK_ASSERT(pfn != nullptr);

		VkBufferCreateInfo bufInfo  = Internal::vkInit<VkBufferCreateInfo>();
		bufInfo.size                = v_Size;
		bufInfo.usage               = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
		                            | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		bufInfo.sharingMode         = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo vmaInfo{};
		vmaInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

		VkBuffer      buf{};
		VmaAllocation alloc{};
		vmaCreateBuffer(g_GlobalInstance.m_Allocator.m_Allocator, &bufInfo, &vmaInfo,
		                &buf, &alloc, nullptr);

		auto createInfo        = Internal::vkInit<VkAccelerationStructureCreateInfoKHR>();
		createInfo.buffer      = buf;
		createInfo.size        = v_Size;
		createInfo.type        = static_cast<VkAccelerationStructureTypeKHR>(v_Type);
		(void)v_Flags;

		VkAccelerationStructureKHR as{};
		pfn(dev, &createInfo, nullptr, &as);

		Utils::AccelerationStructureHandle handle{};
		handle.m_Handle        = static_cast<void*>(as);
		handle.m_BackingBuffer = static_cast<void*>(buf);
		handle.m_BackingAlloc  = static_cast<void*>(alloc);
		return handle;
	}

	uint64_t VulkanAccelStructure::getDeviceAddress(const Utils::AccelerationStructureHandle& r_AS) {
		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
		static auto pfn = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
			vkGetDeviceProcAddr(dev, "vkGetAccelerationStructureDeviceAddressKHR"));
		SPEC_VK_BK_ASSERT(pfn != nullptr);

		auto info = Internal::vkInit<VkAccelerationStructureDeviceAddressInfoKHR>();
		info.accelerationStructure = static_cast<VkAccelerationStructureKHR>(r_AS.m_Handle);
		return pfn(dev, &info);
	}

	void VulkanAccelStructure::cmdBuild(const Utils::CommandBufferHandle& r_Cmd,
	                                    const Utils::ASBuildDesc&         r_Desc) {
		SPEC_VK_BK_ASSERT(r_Desc.m_GeometryCount <= MAX_GEOMETRIES);
		SPEC_VK_BK_ASSERT(r_Desc.m_PrimitiveCounts != nullptr);
		SPEC_VK_BK_ASSERT(r_Desc.m_Dst.isValid());

		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
		static auto pfn = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
			vkGetDeviceProcAddr(dev, "vkCmdBuildAccelerationStructuresKHR"));
		SPEC_VK_BK_ASSERT(pfn != nullptr);

		VkAccelerationStructureGeometryKHR geoms[MAX_GEOMETRIES];
		fillGeomArray(r_Desc, geoms);

		auto buildInfo                      = Internal::vkInit<VkAccelerationStructureBuildGeometryInfoKHR>();
		buildInfo.type                      = static_cast<VkAccelerationStructureTypeKHR>(r_Desc.m_Type);
		buildInfo.flags                     = static_cast<VkBuildAccelerationStructureFlagsKHR>(r_Desc.m_Flags);
		buildInfo.mode                      = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.dstAccelerationStructure  = static_cast<VkAccelerationStructureKHR>(r_Desc.m_Dst.m_Handle);
		buildInfo.geometryCount             = r_Desc.m_GeometryCount;
		buildInfo.pGeometries               = geoms;
		buildInfo.scratchData.deviceAddress = r_Desc.m_ScratchDeviceAddress;

		VkAccelerationStructureBuildRangeInfoKHR ranges[MAX_GEOMETRIES];
		for (uint32_t i = 0; i < r_Desc.m_GeometryCount; ++i) {
			ranges[i].primitiveCount  = r_Desc.m_PrimitiveCounts[i];
			ranges[i].primitiveOffset = 0;
			ranges[i].firstVertex     = 0;
			ranges[i].transformOffset = 0;
		}

		const VkAccelerationStructureBuildRangeInfoKHR* pRanges = ranges;
		pfn(static_cast<VkCommandBuffer>(r_Cmd.m_Handle), 1, &buildInfo, &pRanges);
	}

	void VulkanAccelStructure::cmdWriteCompactedSize(
		const Utils::CommandBufferHandle&        r_Cmd,
		const Utils::AccelerationStructureHandle& r_AS,
		const Utils::QueryPoolHandle&             r_Pool,
		uint32_t                                  v_FirstQuery)
	{
		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
		static auto pfn = reinterpret_cast<PFN_vkCmdWriteAccelerationStructuresPropertiesKHR>(
			vkGetDeviceProcAddr(dev, "vkCmdWriteAccelerationStructuresPropertiesKHR"));
		SPEC_VK_BK_ASSERT(pfn != nullptr);

		VkAccelerationStructureKHR as = static_cast<VkAccelerationStructureKHR>(r_AS.m_Handle);
		pfn(static_cast<VkCommandBuffer>(r_Cmd.m_Handle),
		    1, &as,
		    VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
		    static_cast<VkQueryPool>(r_Pool.m_Handle),
		    v_FirstQuery);
	}

	void VulkanAccelStructure::cmdCompact(
		const Utils::CommandBufferHandle&        r_Cmd,
		const Utils::AccelerationStructureHandle& r_Src,
		const Utils::AccelerationStructureHandle& r_Dst)
	{
		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
		static auto pfn = reinterpret_cast<PFN_vkCmdCopyAccelerationStructureKHR>(
			vkGetDeviceProcAddr(dev, "vkCmdCopyAccelerationStructureKHR"));
		SPEC_VK_BK_ASSERT(pfn != nullptr);

		auto copyInfo      = Internal::vkInit<VkCopyAccelerationStructureInfoKHR>();
		copyInfo.src       = static_cast<VkAccelerationStructureKHR>(r_Src.m_Handle);
		copyInfo.dst       = static_cast<VkAccelerationStructureKHR>(r_Dst.m_Handle);
		copyInfo.mode      = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR;

		pfn(static_cast<VkCommandBuffer>(r_Cmd.m_Handle), &copyInfo);
	}

	void VulkanAccelStructure::destroyAccelStructure(const Utils::AccelerationStructureHandle& r_AS) {
		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
		static auto pfn = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
			vkGetDeviceProcAddr(dev, "vkDestroyAccelerationStructureKHR"));
		SPEC_VK_BK_ASSERT(pfn != nullptr);

		pfn(dev, static_cast<VkAccelerationStructureKHR>(r_AS.m_Handle), nullptr);
		vmaDestroyBuffer(g_GlobalInstance.m_Allocator.m_Allocator,
		                 static_cast<VkBuffer>(r_AS.m_BackingBuffer),
		                 static_cast<VmaAllocation>(r_AS.m_BackingAlloc));
	}
}
