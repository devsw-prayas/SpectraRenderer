#pragma once

#include "SpectraVulkanBackend.h"
#include "SpecVulkanDiagnostics.h"

namespace Spectra::Vulkan::Utils {
	inline constexpr uint32_t VK_SUPPORTED_EXT_COUNT = 27;
	inline constexpr uint32_t VK_SUPPORTED_LAYERS_COUNT = 1;
	inline constexpr uint32_t MAX_INSTANCE_EXT = 128;

	enum class SPEC_VK_BK_RUNTIME_API VulkanExtensions final : uint8_t {
		// Instance Extensions
		VK_SURFACE,
		VK_WIN32_SURFACE,
		VK_GET_PHYSICAL_DEVICE_PROPERTIES,
		VK_DEBUG_UTILS,

		//Device Extensions
		VK_SWAPCHAIN,
		VK_SYNCHRONIZATION_2,
		VK_DYNAMIC_RENDERING,
		VK_BUFFER_DEVICE_ADDRESS,
		VK_DESCRIPTOR_INDEXING,
		VK_MAINTENANCE4,
		VK_TIMELINE_SEMAPHORE,
		VK_EXTERNAL_MEMORY,
		VK_EXTERNAL_MEMORY_WIN32,
		VK_EXTERNAL_SEMAPHORE,
		VK_EXTERNAL_SEMAPHORE_WIN32,
		VK_DEDICATED_ALLOCATION,
		VK_BIND_MEMORY2,
		VK_GET_MEMORY_REQUIREMENTS2,

		// RT Extensions
		VK_RAY_TRACING_PIPELINE,
		VK_ACCELERATION_STRUCTURE,
		VK_DEFERRED_HOST_OPERATIONS,
		VK_RAY_QUERY,
		VK_PIPELINE_LIBRARY,

		// Optional Extensions
		VK_MEMORY_BUDGET,
		VK_SHADER_FLOAT16_INT8,
		VK_16BIT_STORAGE,
		VK_MESH_SHADER,

		NONE
	};

	enum class SPEC_VK_BK_RUNTIME_API VulkanLayers final : uint8_t {
		VK_VALIDATION,
		NONE
	};

	enum class SPEC_VK_BK_RUNTIME_API ExtensionRequirement final : uint8_t {
		REQUIRED,
		NON_ESSENTIAL
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) VkExtension final {
		const char* m_Extension;
		VulkanExtensions       m_ExtensionName;
		ExtensionRequirement   m_Requirement;

		VkExtension() = default;

		VkExtension(const char* v_Ext, VulkanExtensions v_Name, ExtensionRequirement v_Req)
			: m_Extension(v_Ext), m_ExtensionName(v_Name), m_Requirement(v_Req) {
		}

		VkExtension(const VkExtension&) = default;
		VkExtension& operator=(const VkExtension&) = default;
		VkExtension(VkExtension&&) noexcept = default;
		VkExtension& operator=(VkExtension&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) VkLayer final {
		const char* m_Layer;
		VulkanLayers       m_LayerName;

		VkLayer() = default;

		VkLayer(const char* v_Ext, VulkanLayers v_Name)
			: m_Layer(v_Ext), m_LayerName(v_Name) {
		}

		VkLayer(const VkLayer&) = default;
		VkLayer& operator=(const VkLayer&) = default;
		VkLayer(VkLayer&&) noexcept = default;
		VkLayer& operator=(VkLayer&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(16) InitDesc final {
		const char* m_ApplicationName = nullptr;
		bool m_EnableValidation = false;

		InitDesc(const char* v_Name, bool v_Enabled)
			: m_ApplicationName(v_Name), m_EnableValidation(v_Enabled) {
		}
		~InitDesc() = default;

		InitDesc(const InitDesc&) = default;
		InitDesc& operator=(const InitDesc&) = default;

		InitDesc(InitDesc&&) noexcept = default;
		InitDesc& operator=(InitDesc&&) noexcept = default;
	};

	struct SPEC_VK_BK_RUNTIME_API SPEC_VK_BK_ALIGNAS(8) PhysicalDevice final {
		void* m_DeviceHandle;
		void* m_PropertiesHandle;

		bool isValid() const {
			return m_DeviceHandle != nullptr && m_PropertiesHandle != nullptr;
		}

		PhysicalDevice() : m_DeviceHandle(nullptr), m_PropertiesHandle(nullptr) {}

		PhysicalDevice(const PhysicalDevice&) = default;
		PhysicalDevice& operator=(const PhysicalDevice&) = default;

		PhysicalDevice(PhysicalDevice&&) noexcept = default;
		PhysicalDevice& operator=(PhysicalDevice&&) noexcept = default;
	};
}
