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
		static std::vector<Utils::VkLayer>     s_Layers;

		static constexpr int INSTANCE_EXTENSIONS = 4;
		static constexpr int DEVICE_EXTENSIONS   = 16;
		static constexpr int RT_EXTENSIONS       = 4;
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

	// -------------------------------------------------------------------------
	// vkInit<T> — zero-init + sType + pNext = nullptr for every Vulkan struct.
	// Primary template asserts at compile time if a specialization is missing.
	// -------------------------------------------------------------------------

	template<typename T>
	SPEC_VK_BK_FORCEINLINE T vkInit() {
		SPEC_VK_BK_STATIC_ASSERT(sizeof(T) == 0, "vkInit not supported for this type");
		return {};
	}

	// -------------------------------------------------------------------------
	// Bootstrap / Instance
	// -------------------------------------------------------------------------

	template<> SPEC_VK_BK_FORCEINLINE VkApplicationInfo vkInit<VkApplicationInfo>() {
		VkApplicationInfo info{};
		info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkInstanceCreateInfo vkInit<VkInstanceCreateInfo>() {
		VkInstanceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkDebugUtilsMessengerCreateInfoEXT vkInit<VkDebugUtilsMessengerCreateInfoEXT>() {
		VkDebugUtilsMessengerCreateInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkDebugUtilsObjectNameInfoEXT vkInit<VkDebugUtilsObjectNameInfoEXT>() {
		VkDebugUtilsObjectNameInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkDebugUtilsLabelEXT vkInit<VkDebugUtilsLabelEXT>() {
		VkDebugUtilsLabelEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		info.pNext = nullptr;
		return info;
	}

	// -------------------------------------------------------------------------
	// Physical device selection
	// -------------------------------------------------------------------------

	template<> SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceProperties2 vkInit<VkPhysicalDeviceProperties2>() {
		VkPhysicalDeviceProperties2 props{};
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		props.pNext = nullptr;
		return props;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceFeatures2 vkInit<VkPhysicalDeviceFeatures2>() {
		VkPhysicalDeviceFeatures2 props{};
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		props.pNext = nullptr;
		return props;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceVulkan12Features vkInit<VkPhysicalDeviceVulkan12Features>() {
		VkPhysicalDeviceVulkan12Features f{};
		f.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		f.pNext = nullptr;
		return f;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceVulkan13Features vkInit<VkPhysicalDeviceVulkan13Features>() {
		VkPhysicalDeviceVulkan13Features f{};
		f.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		f.pNext = nullptr;
		return f;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceAccelerationStructureFeaturesKHR vkInit<VkPhysicalDeviceAccelerationStructureFeaturesKHR>() {
		VkPhysicalDeviceAccelerationStructureFeaturesKHR f{};
		f.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		f.pNext = nullptr;
		return f;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceRayTracingPipelineFeaturesKHR vkInit<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>() {
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR f{};
		f.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		f.pNext = nullptr;
		return f;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceAccelerationStructurePropertiesKHR vkInit<VkPhysicalDeviceAccelerationStructurePropertiesKHR>() {
		VkPhysicalDeviceAccelerationStructurePropertiesKHR prop{};
		prop.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
		prop.pNext = nullptr;
		return prop;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceRayTracingPipelinePropertiesKHR vkInit<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>() {
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR props{};
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		props.pNext = nullptr;
		return props;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPhysicalDeviceMemoryProperties2 vkInit<VkPhysicalDeviceMemoryProperties2>() {
		VkPhysicalDeviceMemoryProperties2 props{};
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
		props.pNext = nullptr;
		return props;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkQueueFamilyProperties2 vkInit<VkQueueFamilyProperties2>() {
		VkQueueFamilyProperties2 props{};
		props.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
		props.pNext = nullptr;
		return props;
	}

	// -------------------------------------------------------------------------
	// Logical device
	// -------------------------------------------------------------------------

	template<> SPEC_VK_BK_FORCEINLINE VkDeviceQueueCreateInfo vkInit<VkDeviceQueueCreateInfo>() {
		VkDeviceQueueCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkDeviceCreateInfo vkInit<VkDeviceCreateInfo>() {
		VkDeviceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	// -------------------------------------------------------------------------
	// Command pools / buffers
	// -------------------------------------------------------------------------

	template<> SPEC_VK_BK_FORCEINLINE VkCommandPoolCreateInfo vkInit<VkCommandPoolCreateInfo>() {
		VkCommandPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkCommandBufferAllocateInfo vkInit<VkCommandBufferAllocateInfo>() {
		VkCommandBufferAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkCommandBufferBeginInfo vkInit<VkCommandBufferBeginInfo>() {
		VkCommandBufferBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = nullptr;
		return info;
	}

	// -------------------------------------------------------------------------
	// Synchronisation
	// -------------------------------------------------------------------------

	template<> SPEC_VK_BK_FORCEINLINE VkFenceCreateInfo vkInit<VkFenceCreateInfo>() {
		VkFenceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkSemaphoreCreateInfo vkInit<VkSemaphoreCreateInfo>() {
		VkSemaphoreCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkSemaphoreTypeCreateInfo vkInit<VkSemaphoreTypeCreateInfo>() {
		VkSemaphoreTypeCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkSemaphoreSignalInfo vkInit<VkSemaphoreSignalInfo>() {
		VkSemaphoreSignalInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkSemaphoreWaitInfo vkInit<VkSemaphoreWaitInfo>() {
		VkSemaphoreWaitInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkSubmitInfo2 vkInit<VkSubmitInfo2>() {
		VkSubmitInfo2 info{};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkSemaphoreSubmitInfo vkInit<VkSemaphoreSubmitInfo>() {
		VkSemaphoreSubmitInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkCommandBufferSubmitInfo vkInit<VkCommandBufferSubmitInfo>() {
		VkCommandBufferSubmitInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		info.pNext = nullptr;
		return info;
	}

	// -------------------------------------------------------------------------
	// Pipeline barriers (synchronization2)
	// -------------------------------------------------------------------------

	template<> SPEC_VK_BK_FORCEINLINE VkMemoryBarrier2 vkInit<VkMemoryBarrier2>() {
		VkMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
		barrier.pNext = nullptr;
		return barrier;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkBufferMemoryBarrier2 vkInit<VkBufferMemoryBarrier2>() {
		VkBufferMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
		barrier.pNext = nullptr;
		return barrier;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkImageMemoryBarrier2 vkInit<VkImageMemoryBarrier2>() {
		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.pNext = nullptr;
		return barrier;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkDependencyInfo vkInit<VkDependencyInfo>() {
		VkDependencyInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		info.pNext = nullptr;
		return info;
	}

	// -------------------------------------------------------------------------
	// Resources
	// -------------------------------------------------------------------------

	template<> SPEC_VK_BK_FORCEINLINE VkBufferCreateInfo vkInit<VkBufferCreateInfo>() {
		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkImageCreateInfo vkInit<VkImageCreateInfo>() {
		VkImageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkImageViewCreateInfo vkInit<VkImageViewCreateInfo>() {
		VkImageViewCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkSamplerCreateInfo vkInit<VkSamplerCreateInfo>() {
		VkSamplerCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkExternalMemoryBufferCreateInfo vkInit<VkExternalMemoryBufferCreateInfo>() {
		VkExternalMemoryBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkExternalMemoryImageCreateInfo vkInit<VkExternalMemoryImageCreateInfo>() {
		VkExternalMemoryImageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkExportMemoryAllocateInfo vkInit<VkExportMemoryAllocateInfo>() {
		VkExportMemoryAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkImportMemoryWin32HandleInfoKHR vkInit<VkImportMemoryWin32HandleInfoKHR>() {
		VkImportMemoryWin32HandleInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkExportSemaphoreCreateInfo vkInit<VkExportSemaphoreCreateInfo>() {
		VkExportSemaphoreCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	// -------------------------------------------------------------------------
	// Swapchain
	// -------------------------------------------------------------------------

	template<> SPEC_VK_BK_FORCEINLINE VkWin32SurfaceCreateInfoKHR vkInit<VkWin32SurfaceCreateInfoKHR>() {
		VkWin32SurfaceCreateInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkSwapchainCreateInfoKHR vkInit<VkSwapchainCreateInfoKHR>() {
		VkSwapchainCreateInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPresentInfoKHR vkInit<VkPresentInfoKHR>() {
		VkPresentInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.pNext = nullptr;
		return info;
	}

	// -------------------------------------------------------------------------
	// Descriptors
	// -------------------------------------------------------------------------

	template<> SPEC_VK_BK_FORCEINLINE VkDescriptorSetLayoutCreateInfo vkInit<VkDescriptorSetLayoutCreateInfo>() {
		VkDescriptorSetLayoutCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkDescriptorSetLayoutBindingFlagsCreateInfo vkInit<VkDescriptorSetLayoutBindingFlagsCreateInfo>() {
		VkDescriptorSetLayoutBindingFlagsCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkDescriptorPoolCreateInfo vkInit<VkDescriptorPoolCreateInfo>() {
		VkDescriptorPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkDescriptorSetAllocateInfo vkInit<VkDescriptorSetAllocateInfo>() {
		VkDescriptorSetAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkWriteDescriptorSet vkInit<VkWriteDescriptorSet>() {
		VkWriteDescriptorSet info{};
		info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkWriteDescriptorSetAccelerationStructureKHR vkInit<VkWriteDescriptorSetAccelerationStructureKHR>() {
		VkWriteDescriptorSetAccelerationStructureKHR info{};
		info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPipelineLayoutCreateInfo vkInit<VkPipelineLayoutCreateInfo>() {
		VkPipelineLayoutCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	// -------------------------------------------------------------------------
	// Shaders & pipelines
	// -------------------------------------------------------------------------

	template<> SPEC_VK_BK_FORCEINLINE VkShaderModuleCreateInfo vkInit<VkShaderModuleCreateInfo>() {
		VkShaderModuleCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPipelineCacheCreateInfo vkInit<VkPipelineCacheCreateInfo>() {
		VkPipelineCacheCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPipelineShaderStageCreateInfo vkInit<VkPipelineShaderStageCreateInfo>() {
		VkPipelineShaderStageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPipelineVertexInputStateCreateInfo vkInit<VkPipelineVertexInputStateCreateInfo>() {
		VkPipelineVertexInputStateCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPipelineInputAssemblyStateCreateInfo vkInit<VkPipelineInputAssemblyStateCreateInfo>() {
		VkPipelineInputAssemblyStateCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPipelineViewportStateCreateInfo vkInit<VkPipelineViewportStateCreateInfo>() {
		VkPipelineViewportStateCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPipelineRasterizationStateCreateInfo vkInit<VkPipelineRasterizationStateCreateInfo>() {
		VkPipelineRasterizationStateCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPipelineMultisampleStateCreateInfo vkInit<VkPipelineMultisampleStateCreateInfo>() {
		VkPipelineMultisampleStateCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPipelineDepthStencilStateCreateInfo vkInit<VkPipelineDepthStencilStateCreateInfo>() {
		VkPipelineDepthStencilStateCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPipelineColorBlendStateCreateInfo vkInit<VkPipelineColorBlendStateCreateInfo>() {
		VkPipelineColorBlendStateCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPipelineDynamicStateCreateInfo vkInit<VkPipelineDynamicStateCreateInfo>() {
		VkPipelineDynamicStateCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkPipelineRenderingCreateInfo vkInit<VkPipelineRenderingCreateInfo>() {
		VkPipelineRenderingCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkGraphicsPipelineCreateInfo vkInit<VkGraphicsPipelineCreateInfo>() {
		VkGraphicsPipelineCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkComputePipelineCreateInfo vkInit<VkComputePipelineCreateInfo>() {
		VkComputePipelineCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkRayTracingShaderGroupCreateInfoKHR vkInit<VkRayTracingShaderGroupCreateInfoKHR>() {
		VkRayTracingShaderGroupCreateInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkRayTracingPipelineCreateInfoKHR vkInit<VkRayTracingPipelineCreateInfoKHR>() {
		VkRayTracingPipelineCreateInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		info.pNext = nullptr;
		return info;
	}

	// -------------------------------------------------------------------------
	// Dynamic rendering
	// -------------------------------------------------------------------------

	template<> SPEC_VK_BK_FORCEINLINE VkRenderingInfo vkInit<VkRenderingInfo>() {
		VkRenderingInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkRenderingAttachmentInfo vkInit<VkRenderingAttachmentInfo>() {
		VkRenderingAttachmentInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		info.pNext = nullptr;
		return info;
	}

	// -------------------------------------------------------------------------
	// Acceleration structures
	// -------------------------------------------------------------------------

	template<> SPEC_VK_BK_FORCEINLINE VkAccelerationStructureCreateInfoKHR vkInit<VkAccelerationStructureCreateInfoKHR>() {
		VkAccelerationStructureCreateInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkAccelerationStructureGeometryKHR vkInit<VkAccelerationStructureGeometryKHR>() {
		VkAccelerationStructureGeometryKHR info{};
		info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkAccelerationStructureGeometryTrianglesDataKHR vkInit<VkAccelerationStructureGeometryTrianglesDataKHR>() {
		VkAccelerationStructureGeometryTrianglesDataKHR info{};
		info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkAccelerationStructureBuildGeometryInfoKHR vkInit<VkAccelerationStructureBuildGeometryInfoKHR>() {
		VkAccelerationStructureBuildGeometryInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		info.pNext = nullptr;
		return info;
	}

	template<> SPEC_VK_BK_FORCEINLINE VkAccelerationStructureDeviceAddressInfoKHR vkInit<VkAccelerationStructureDeviceAddressInfoKHR>() {
		VkAccelerationStructureDeviceAddressInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		info.pNext = nullptr;
		return info;
	}

	// -------------------------------------------------------------------------
	// Queries
	// -------------------------------------------------------------------------

	template<> SPEC_VK_BK_FORCEINLINE VkQueryPoolCreateInfo vkInit<VkQueryPoolCreateInfo>() {
		VkQueryPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		info.pNext = nullptr;
		return info;
	}
}
