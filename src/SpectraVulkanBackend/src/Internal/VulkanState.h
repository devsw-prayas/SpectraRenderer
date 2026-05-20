#pragma once
#include "VulkanUtils.h"
#define ALLOW_SYSCALL
#include "SpecVulkanSyscalls.h"
#include <vk_mem_alloc.h>

namespace Spectra::Vulkan {

	// Flat bag of physical device capability structs.
	// pNext chains are intentionally cleared after query and re-linked on demand
	// (e.g. at logical device creation). Never store pointers into this struct.
	struct DeviceProperties {
		VkPhysicalDeviceProperties2                          m_Properties;
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR      m_RTProperties;
		VkPhysicalDeviceAccelerationStructurePropertiesKHR   m_AccelProperties;

		VkPhysicalDeviceFeatures2                            m_Features;
		VkPhysicalDeviceVulkan12Features                     m_Vk12Features;
		VkPhysicalDeviceVulkan13Features                     m_Vk13Features;
		VkPhysicalDeviceAccelerationStructureFeaturesKHR     m_AccelFeatures;
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR        m_RTPipelineFeatures;

		VkPhysicalDeviceMemoryProperties2                    m_MemoryProperties;

		Utils::VkExtension m_EnabledExtensions[Utils::VK_SUPPORTED_EXT_COUNT];
		uint32_t           s_ExtensionCount;

		char     m_DeviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
		uint32_t m_VendorID;
		uint32_t m_DeviceID;
	};

	// Owns the VkDevice handle and the three queue handles.
	// Queues are retrieved once at createLogicalDevice() and never re-queried.
	// Graphics family always exists. Compute/Transfer may alias Graphics if no
	// dedicated family was found on the physical device.
	struct LogicalDevice {
		VkDevice m_Device;
		VkQueue  m_GraphicsQueue;
		VkQueue  m_ComputeQueue;
		VkQueue  m_TransferQueue;
	};

	// Wraps VmaAllocator. VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT is always
	// set (required for RT). VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT is set only
	// if VK_EXT_memory_budget was present and enabled during physical device selection.
	struct VulkanAllocator {
		VmaAllocator m_Allocator;

		bool isValid() const { return m_Allocator != VK_NULL_HANDLE; }

		VulkanAllocator() : m_Allocator(VK_NULL_HANDLE) {}
	};

	// One VkCommandPool per unique queue family.
	// If compute/transfer alias graphics, the pool handle is shared - do NOT
	// double-destroy aliased pools. Destruction logic in shutdownVulkan() accounts
	// for this via handle comparison.
	struct CommandPools {
		VkCommandPool m_GraphicsPool;
		VkCommandPool m_ComputePool;
		VkCommandPool m_TransferPool;

		bool isValid() const {
			return m_GraphicsPool != VK_NULL_HANDLE
				&& m_ComputePool  != VK_NULL_HANDLE
				&& m_TransferPool != VK_NULL_HANDLE;
		}

		CommandPools()
			: m_GraphicsPool(VK_NULL_HANDLE)
			, m_ComputePool(VK_NULL_HANDLE)
			, m_TransferPool(VK_NULL_HANDLE) {
		}
	};

	// Central opaque state for the entire Vulkan backend.
	// Exposed externally only as InstanceHandle (void*).
	// All subsystems (device, allocator, pools) are flat members - no heap allocation.
	// Lifetime: static global, valid from initializeVulkan() to shutdownVulkan().
	struct SPEC_VK_BK_ALIGNAS(16) VulkanInstance final {
		constexpr static uint32_t MAX_DEVICES        = 32;
		constexpr static uint32_t MAX_QUEUE_FAMILIES = 16;

		VkInstance               m_GlobalInstance;
		VkDebugUtilsMessengerEXT m_GlobalDebugMessager;
		VkApplicationInfo        m_AppInfo;
		DeviceProperties         m_DeviceProps;
		Utils::PhysicalDevice    m_Device;
		LogicalDevice            m_LogicalDevice;
		VulkanAllocator          m_Allocator;
		CommandPools             m_CommandPools;

		Utils::VkExtension m_EnabledExtensions[Utils::VK_SUPPORTED_EXT_COUNT];
		Utils::VkLayer     m_EnableLayers[Utils::VK_SUPPORTED_LAYERS_COUNT];
		uint32_t s_LayerCount;
		uint32_t s_ExtensionCount;
		uint32_t s_DeviceCount;

		uint32_t m_GraphicsFamily;
		uint32_t m_ComputeFamily;
		uint32_t m_TransferFamily;

		VulkanInstance() = default;
		~VulkanInstance() = default;
		VulkanInstance(const VulkanInstance&) = default;
		VulkanInstance& operator=(const VulkanInstance&) = default;
		VulkanInstance(VulkanInstance&&) noexcept = default;
		VulkanInstance& operator=(VulkanInstance&&) noexcept = default;
	};

	extern VulkanInstance g_GlobalInstance;
	extern bool           g_IsInitialized;
}
