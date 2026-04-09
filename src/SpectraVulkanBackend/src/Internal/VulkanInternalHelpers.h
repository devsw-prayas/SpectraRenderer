#pragma once
#include "VulkanUtils.h"
#include <vector>
#define ALLOW_SYSCALL
#include "SpecVulkanSyscalls.h"

namespace Spectra::Vulkan::Internal {
#define VK_LAYERS_KHRONOS_VALIDATION "VK_LAYER_KHRONOS_validation"

	class VulkanRegistry final {
	public:
		static std::vector<Utils::VkExtension> s_Extensions;
		static std::vector<Utils::VkLayer> s_Layers;
		static constexpr int INSTANCE_EXTENSIONS = 4;
		static constexpr int DEVICE_EXTENSIONS = 16;
		static constexpr int RT_EXTENSIONS = 4;
		static constexpr int OPTIONAL_EXTENSIONS = 3;

		static void initRegistry();

		static const Utils::VkExtension* find(Utils::VulkanExtensions name) {
			for (const auto& ext : s_Extensions)
				if (ext.m_ExtensionName == name) return &ext;
			return nullptr;
		}

		VulkanRegistry() = delete;
		~VulkanRegistry() = delete;
		VulkanRegistry(const VulkanRegistry&) = delete;
		VulkanRegistry& operator=(const VulkanRegistry&) = delete;
		VulkanRegistry(VulkanRegistry&&) noexcept = delete;
		VulkanRegistry& operator=(VulkanRegistry&&) noexcept = delete;
	};

	// Struct Initializers

	template<typename T>
	SPEC_VK_BK_FORCEINLINE T vkInit() {
		SPEC_VK_BK_STATIC_ASSERT(sizeof(T) == 0, "vkInit not supported for thie type");
		return {};
	}

	template<>
	SPEC_VK_BK_FORCEINLINE VkApplicationInfo vkInit<VkApplicationInfo>() {
		VkApplicationInfo info{};
		info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<>
	FORCEINLINE VkInstanceCreateInfo vkInit<VkInstanceCreateInfo>() {
		VkInstanceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<>
	SPEC_VK_BK_FORCEINLINE VkDebugUtilsMessengerCreateInfoEXT vkInit<VkDebugUtilsMessengerCreateInfoEXT>() {
		VkDebugUtilsMessengerCreateInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.pNext = nullptr;
		return info;
	}

	template<>
	SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceAccelerationStructurePropertiesKHR vkInit < VkPhysicalDeviceAccelerationStructurePropertiesKHR>() {
		VkPhysicalDeviceAccelerationStructurePropertiesKHR prop{};
		prop.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
		prop.pNext = nullptr;
		return prop;
	}

	template<>
	SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceRayTracingPipelinePropertiesKHR vkInit<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>() {
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR props{};
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		props.pNext = nullptr;
		return props;
	}

	template<>
	SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceProperties2 vkInit<VkPhysicalDeviceProperties2>() {
		VkPhysicalDeviceProperties2 props{};
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		props.pNext = nullptr;
		return props;
	}

	template<>
	SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceVulkan12Features vkInit<VkPhysicalDeviceVulkan12Features>() {
		VkPhysicalDeviceVulkan12Features f{};
		f.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		f.pNext = nullptr;
		return f;
	}

	template<>
	SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceVulkan13Features vkInit<VkPhysicalDeviceVulkan13Features>() {
		VkPhysicalDeviceVulkan13Features f{};
		f.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		f.pNext = nullptr;
		return f;
	}

	template<>
	SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceAccelerationStructureFeaturesKHR vkInit<VkPhysicalDeviceAccelerationStructureFeaturesKHR>() {
		VkPhysicalDeviceAccelerationStructureFeaturesKHR f{};
		f.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		f.pNext = nullptr;
		return f;
	}

	template<>
	SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceRayTracingPipelineFeaturesKHR vkInit<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>() {
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR f{};
		f.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		f.pNext = nullptr;
		return f;
	}

	template<>
	SPEC_VK_BK_FORCEINLINE VkQueueFamilyProperties2 vkInit<VkQueueFamilyProperties2>() {
		VkQueueFamilyProperties2 props{};
		props.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
		props.pNext = nullptr;
		return props;
	}

	template<>
	SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceMemoryProperties2 vkInit<VkPhysicalDeviceMemoryProperties2>() {
		VkPhysicalDeviceMemoryProperties2 props{};
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
		props.pNext = nullptr;
		return props;
	}

	template<>
	SPEC_VK_BK_FORCEINLINE VkDeviceQueueCreateInfo vkInit<VkDeviceQueueCreateInfo>() {
		VkDeviceQueueCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<>
	SPEC_VK_BK_FORCEINLINE VkDeviceCreateInfo vkInit<VkDeviceCreateInfo>() {
		VkDeviceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<>
	SPEC_VK_BK_FORCEINLINE VkDebugUtilsObjectNameInfoEXT vkInit<VkDebugUtilsObjectNameInfoEXT>() {
		VkDebugUtilsObjectNameInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		info.pNext = nullptr;
		return info;
	}

	template<>
	SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceFeatures2 vkInit<VkPhysicalDeviceFeatures2>() {
		VkPhysicalDeviceFeatures2 props{};
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		props.pNext = nullptr;
		return props;
	}
}
