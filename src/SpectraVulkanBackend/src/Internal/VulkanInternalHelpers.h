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
}
