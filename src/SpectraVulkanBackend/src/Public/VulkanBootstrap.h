#pragma once
#include "SpectraVulkanBackend.h"
#include "VulkanUtils.h"

namespace Spectra::Vulkan {
	struct VulkanInstance;
	using InstanceHandle = VulkanInstance*;
	struct DeviceProperties;
	using PropertyHandle = DeviceProperties*;
	struct LogicalDevice;
	using LogicalDeviceHandle = LogicalDevice*;

	class SPEC_VK_BK_RUNTIME_API VulkanBootstrap final {
	public:
		static InstanceHandle initializeVulkan(const Utils::InitDesc& ro_Desc);
		static void selectPhysicalDevice();
		static InstanceHandle getVulkanInstance();
		static void createLogicalDevice();
		static void createAllocator();
		static void createCommandPools();
		static void shutdownVulkan(InstanceHandle p_Handle);
	};
}
