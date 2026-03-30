#include "SpectraVulkanBackend.h"
#include "VulkanBootstrap.h"

#define ALLOW_SYSCALL
#include "SpecVulkanSyscalls.h"
#include "VulkanInternalHelpers.h"

namespace Spectra::Vulkan {
	struct VulkanInstance final {
		constexpr static uint32_t MAX_DEVICES = 32; // TODO replace with build macro
		VkInstance m_GlobalInstance;
		VkDebugUtilsMessengerEXT m_GlobalDebugMessager;
		VkApplicationInfo m_AppInfo;

		Utils::VkExtension m_EnabledExtensions[Utils::VK_SUPPORTED_EXT_COUNT];
		Utils::VkLayer m_EnableLayers[Utils::VK_SUPPORTED_LAYERS_COUNT];
		uint32_t s_LayerCount;
		uint32_t s_ExtensionCount;
		uint32_t s_DeviceCount;

		VulkanInstance() = default;
		~VulkanInstance() = default;

		VulkanInstance(const VulkanInstance&) = default;
		VulkanInstance& operator=(const VulkanInstance&) = default;

		VulkanInstance(VulkanInstance&&) noexcept = default;
		VulkanInstance& operator=(VulkanInstance&&) noexcept = default;
	};

	namespace {
		VulkanInstance g_GlobalInstance;
		bool g_IsInitialized = false;

		VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT,
			const VkDebugUtilsMessengerCallbackDataEXT* data,
			void*) {
			SPEC_VK_BK_UNUSED(data);

			if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
				SPEC_VK_BK_ASSERT(false);
			}

			return VK_FALSE;
		}
	}

	// TODO Upgrade the array to support StormSTL VA Heap
	InstanceHandle VulkanBootstrap::initializeVulkan(const Utils::InitDesc& ro_Desc) {
		Internal::VulkanRegistry::initRegistry();

		// Build the info
		auto info = Internal::vkInit<VkApplicationInfo>();
		info.apiVersion = VK_API_VERSION_1_3;
		info.pEngineName = "Spectra";
		info.pApplicationName = ro_Desc.m_ApplicationName;
		info.engineVersion = 1;
		info.applicationVersion = 1;
		info.pNext = nullptr;

		g_GlobalInstance.m_AppInfo = info;

		auto createInstance = Internal::vkInit<VkInstanceCreateInfo>();
		createInstance.pApplicationInfo = &g_GlobalInstance.m_AppInfo;

		uint32_t extCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
		VkExtensionProperties props[64];
		SPEC_VK_BK_ASSERT(extCount <= 64);
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, props);

		g_GlobalInstance.s_ExtensionCount = 0;
		for (uint32_t i = 0; i < 4; ++i) {
			const auto& ext = Internal::VulkanRegistry::s_Extensions[i];
			bool found = false;
			for (uint32_t j = 0; j < extCount; ++j) {
				if (strcmp(props[j].extensionName, ext.m_Extension) == 0) {
					found = true;
					break;
				}
			}

			// Debug extension gating
			if (ext.m_ExtensionName == Utils::VulkanExtensions::VK_DEBUG_UTILS &&
				!ro_Desc.m_EnableValidation) {
				continue;
			}

			if (ext.m_Requirement == Utils::ExtensionRequirement::REQUIRED && !found) {
				SPEC_VK_BK_ASSERT(false);
			}

			if (found) {
				g_GlobalInstance.m_EnabledExtensions[g_GlobalInstance.s_ExtensionCount++] = ext;
			}
		}

		const char* enabledExts[Utils::VK_SUPPORTED_EXT_COUNT];

		for (uint32_t i = 0; i < g_GlobalInstance.s_ExtensionCount; ++i) {
			enabledExts[i] = g_GlobalInstance.m_EnabledExtensions[i].m_Extension;
		}

		createInstance.enabledExtensionCount = g_GlobalInstance.s_ExtensionCount;
		createInstance.ppEnabledExtensionNames = enabledExts;

		const char* layers[1];
		g_GlobalInstance.s_LayerCount = 0;

		if (ro_Desc.m_EnableValidation) {
			layers[0] = VK_LAYERS_KHRONOS_VALIDATION;
			g_GlobalInstance.s_LayerCount = 1;

			g_GlobalInstance.m_EnableLayers[0] =
			{ VK_LAYERS_KHRONOS_VALIDATION, Utils::VulkanLayers::VK_VALIDATION };
		}

		createInstance.enabledLayerCount = g_GlobalInstance.s_LayerCount;
		createInstance.ppEnabledLayerNames = g_GlobalInstance.s_LayerCount ? layers : nullptr;

		auto debugInfo = Internal::vkInit<VkDebugUtilsMessengerCreateInfoEXT>();
		if (ro_Desc.m_EnableValidation) {
			debugInfo.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

			debugInfo.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

			debugInfo.pfnUserCallback = debugCallback;

			createInstance.pNext = &debugInfo;
		} else {
			createInstance.pNext = nullptr;
		}

		VkResult res = vkCreateInstance(&createInstance, nullptr, &g_GlobalInstance.m_GlobalInstance);
		SPEC_VK_BK_ASSERT(res == VK_SUCCESS);

		if (ro_Desc.m_EnableValidation) {
			auto raw = vkGetInstanceProcAddr(
				g_GlobalInstance.m_GlobalInstance,
				"vkCreateDebugUtilsMessengerEXT");

			SPEC_VK_BK_ASSERT(raw != nullptr);

			auto fn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(raw);

			SPEC_VK_BK_ASSERT(fn);

			VkResult dbgRes = fn(
				g_GlobalInstance.m_GlobalInstance,
				&debugInfo,
				nullptr,
				&g_GlobalInstance.m_GlobalDebugMessager);

			SPEC_VK_BK_ASSERT(dbgRes == VK_SUCCESS);
		}
		return &g_GlobalInstance;
	}

	InstanceHandle VulkanBootstrap::getVulkanInstance() {
		return &g_GlobalInstance;
	}
}
