#pragma once
#include "SpectraVulkanBackend.h"
#include "VulkanUtils.h"

namespace Spectra::Vulkan {
	struct VulkanInstance;
	using InstanceHandle = VulkanInstance*;
	class SPEC_VK_BK_RUNTIME_API VulkanBootstrap final {
	public:
		static InstanceHandle initializeVulkan(const Utils::InitDesc& ro_Desc);
		static InstanceHandle getVulkanInstance();
		static void shutdownVulkan(InstanceHandle p_Handle);
	};
}
