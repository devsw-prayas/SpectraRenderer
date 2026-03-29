#include "SpectraVulkanBackend.h"
#include "VulkanInternalHelpers.h"
#include "VulkanUtils.h"

#define ALLOW_SYSCALL
#include "SpecVulkanSyscalls.h"

namespace Spectra::Vulkan::Internal {
    std::vector<Utils::VkExtension> VulkanExtensionRegistry::s_Extensions;

    void VulkanExtensionRegistry::initRegistry() {
        s_Extensions.resize(27);

        // Instance
        s_Extensions[0] = { VK_KHR_SURFACE_EXTENSION_NAME,                          Utils::VulkanExtensions::VK_SURFACE,                       Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[1] = { VK_KHR_WIN32_SURFACE_EXTENSION_NAME,                    Utils::VulkanExtensions::VK_WIN32_SURFACE,                  Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[2] = { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, Utils::VulkanExtensions::VK_GET_PHYSICAL_DEVICE_PROPERTIES,  Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[3] = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME,                      Utils::VulkanExtensions::VK_DEBUG_UTILS,                    Utils::ExtensionRequirement::NON_ESSENTIAL };

        // Device - Required
        s_Extensions[4] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME,                        Utils::VulkanExtensions::VK_SWAPCHAIN,                      Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[5] = { VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,                Utils::VulkanExtensions::VK_SYNCHRONIZATION_2,              Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[6] = { VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,                Utils::VulkanExtensions::VK_DYNAMIC_RENDERING,              Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[7] = { VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,            Utils::VulkanExtensions::VK_BUFFER_DEVICE_ADDRESS,          Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[8] = { VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,              Utils::VulkanExtensions::VK_DESCRIPTOR_INDEXING,            Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[9] = { VK_KHR_MAINTENANCE_4_EXTENSION_NAME,                    Utils::VulkanExtensions::VK_MAINTENANCE4,                   Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[10] = { VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,               Utils::VulkanExtensions::VK_TIMELINE_SEMAPHORE,             Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[11] = { VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,                  Utils::VulkanExtensions::VK_EXTERNAL_MEMORY,                Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[12] = { VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,            Utils::VulkanExtensions::VK_EXTERNAL_MEMORY_WIN32,          Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[13] = { VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,               Utils::VulkanExtensions::VK_EXTERNAL_SEMAPHORE,             Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[14] = { VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,         Utils::VulkanExtensions::VK_EXTERNAL_SEMAPHORE_WIN32,       Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[15] = { VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,             Utils::VulkanExtensions::VK_DEDICATED_ALLOCATION,           Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[16] = { VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,                    Utils::VulkanExtensions::VK_BIND_MEMORY2,                   Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[17] = { VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,        Utils::VulkanExtensions::VK_GET_MEMORY_REQUIREMENTS2,       Utils::ExtensionRequirement::REQUIRED };

        // RT - Required
        s_Extensions[18] = { VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,             Utils::VulkanExtensions::VK_RAY_TRACING_PIPELINE,           Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[19] = { VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,           Utils::VulkanExtensions::VK_ACCELERATION_STRUCTURE,         Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[20] = { VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,         Utils::VulkanExtensions::VK_DEFERRED_HOST_OPERATIONS,       Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[21] = { VK_KHR_RAY_QUERY_EXTENSION_NAME,                        Utils::VulkanExtensions::VK_RAY_QUERY,                      Utils::ExtensionRequirement::REQUIRED };
        s_Extensions[22] = { VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,                 Utils::VulkanExtensions::VK_PIPELINE_LIBRARY,               Utils::ExtensionRequirement::REQUIRED };

        // Optional
        s_Extensions[23] = { VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,                    Utils::VulkanExtensions::VK_MEMORY_BUDGET,                  Utils::ExtensionRequirement::NON_ESSENTIAL };
        s_Extensions[24] = { VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,              Utils::VulkanExtensions::VK_SHADER_FLOAT16_INT8,            Utils::ExtensionRequirement::NON_ESSENTIAL };
        s_Extensions[25] = { VK_KHR_16BIT_STORAGE_EXTENSION_NAME,                    Utils::VulkanExtensions::VK_16BIT_STORAGE,                  Utils::ExtensionRequirement::NON_ESSENTIAL };
        s_Extensions[26] = { VK_NV_MESH_SHADER_EXTENSION_NAME,                       Utils::VulkanExtensions::VK_MESH_SHADER,                    Utils::ExtensionRequirement::NON_ESSENTIAL };
    }
}