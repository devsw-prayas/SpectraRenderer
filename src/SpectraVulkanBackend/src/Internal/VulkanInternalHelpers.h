#pragma once
#include "VulkanUtils.h"
#include <vector>

namespace Spectra::Vulkan::Internal {
    class VulkanExtensionRegistry final {
    public:
        static std::vector<Utils::VkExtension> s_Extensions;

        static void initRegistry();

        static const Utils::VkExtension* find(Utils::VulkanExtensions name) {
            for (const auto& ext : s_Extensions)
                if (ext.m_ExtensionName == name) return &ext;
            return nullptr;
        }

        VulkanExtensionRegistry() = delete;
        ~VulkanExtensionRegistry() = delete;
        VulkanExtensionRegistry(const VulkanExtensionRegistry&) = delete;
        VulkanExtensionRegistry& operator=(const VulkanExtensionRegistry&) = delete;
        VulkanExtensionRegistry(VulkanExtensionRegistry&&) noexcept = delete;
        VulkanExtensionRegistry& operator=(VulkanExtensionRegistry&&) noexcept = delete;
    };
}