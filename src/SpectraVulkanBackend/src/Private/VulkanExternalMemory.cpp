#include "SpectraVulkanBackend.h"
#include "VulkanExternalMemory.h"
#include "VulkanInternalHelpers.h"
#include "VulkanState.h"

namespace Spectra::Vulkan {

	// Walks the physical device memory types and returns the index of the first
	// device-local type whose bit is set in v_TypeBits.
	static uint32_t findDeviceLocalMemoryType(uint32_t v_TypeBits) {
		const VkPhysicalDeviceMemoryProperties& props =
			g_GlobalInstance.m_DeviceProps.m_MemoryProperties.memoryProperties;

		for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
			if ((v_TypeBits & (1u << i)) &&
			    (props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
				return i;
		}
		return ~0u;
	}

	// Shared helper: allocate dedicated VkDeviceMemory with export info chained.
	static VkDeviceMemory allocateExportableMemory(VkMemoryRequirements v_Reqs,
	                                               const VkAllocationCallbacks* p_Alloc) {
		auto exportInfo       = Internal::vkInit<VkExportMemoryAllocateInfo>();
		exportInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

		auto allocInfo            = Internal::vkInit<VkMemoryAllocateInfo>();
		allocInfo.pNext           = &exportInfo;
		allocInfo.allocationSize  = v_Reqs.size;
		allocInfo.memoryTypeIndex = findDeviceLocalMemoryType(v_Reqs.memoryTypeBits);

		VkDeviceMemory memory{};
		vkAllocateMemory(g_GlobalInstance.m_LogicalDevice.m_Device, &allocInfo, p_Alloc, &memory);
		return memory;
	}

	// Shared helper: loads vkGetMemoryWin32HandleKHR on first call.
	static void* exportMemoryHandle(VkDeviceMemory v_Memory) {
		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
		static auto pfn = reinterpret_cast<PFN_vkGetMemoryWin32HandleKHR>(
			vkGetDeviceProcAddr(dev, "vkGetMemoryWin32HandleKHR"));
		SPEC_VK_BK_ASSERT(pfn != nullptr);

		auto info      = Internal::vkInit<VkMemoryGetWin32HandleInfoKHR>();
		info.memory    = v_Memory;
		info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

		HANDLE win32Handle = nullptr;
		pfn(dev, &info, &win32Handle);
		return static_cast<void*>(win32Handle);
	}

	// -------------------------------------------------------------------------
	// VulkanExternalMemory
	// -------------------------------------------------------------------------

	Utils::BufferHandle VulkanExternalMemory::createExportableBuffer(const Utils::BufferDesc& r_Desc, const Utils::AllocationCallbacksDesc& r_Alloc) {
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;

		auto extInfo       = Internal::vkInit<VkExternalMemoryBufferCreateInfo>();
		extInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

		auto bufInfo        = Internal::vkInit<VkBufferCreateInfo>();
		bufInfo.pNext       = &extInfo;
		bufInfo.size        = r_Desc.m_Size;
		bufInfo.usage       = static_cast<VkBufferUsageFlags>(r_Desc.m_Usage);
		bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer buf{};
		vkCreateBuffer(g_GlobalInstance.m_LogicalDevice.m_Device, &bufInfo, pA, &buf);

		VkMemoryRequirements reqs{};
		vkGetBufferMemoryRequirements(g_GlobalInstance.m_LogicalDevice.m_Device, buf, &reqs);

		VkDeviceMemory memory = allocateExportableMemory(reqs, pA);
		vkBindBufferMemory(g_GlobalInstance.m_LogicalDevice.m_Device, buf, memory, 0);

		Utils::BufferHandle handle{};
		handle.m_Handle     = static_cast<void*>(buf);
		handle.m_Allocation = static_cast<void*>(memory);
		return handle;
	}

	Utils::ImageHandle VulkanExternalMemory::createExportableImage(const Utils::ImageDesc& r_Desc, const Utils::AllocationCallbacksDesc& r_Alloc) {
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;

		auto extInfo       = Internal::vkInit<VkExternalMemoryImageCreateInfo>();
		extInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

		auto imgInfo              = Internal::vkInit<VkImageCreateInfo>();
		imgInfo.pNext             = &extInfo;
		imgInfo.imageType         = static_cast<VkImageType>(r_Desc.m_Type);
		imgInfo.format            = static_cast<VkFormat>(r_Desc.m_Format);
		imgInfo.extent            = { r_Desc.m_Width, r_Desc.m_Height, r_Desc.m_Depth };
		imgInfo.mipLevels         = r_Desc.m_MipLevels;
		imgInfo.arrayLayers       = r_Desc.m_ArrayLayers;
		imgInfo.samples           = static_cast<VkSampleCountFlagBits>(r_Desc.m_Samples);
		imgInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
		imgInfo.usage             = static_cast<VkImageUsageFlags>(r_Desc.m_Usage);
		imgInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
		imgInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImage img{};
		vkCreateImage(g_GlobalInstance.m_LogicalDevice.m_Device, &imgInfo, pA, &img);

		VkMemoryRequirements reqs{};
		vkGetImageMemoryRequirements(g_GlobalInstance.m_LogicalDevice.m_Device, img, &reqs);

		VkDeviceMemory memory = allocateExportableMemory(reqs, pA);
		vkBindImageMemory(g_GlobalInstance.m_LogicalDevice.m_Device, img, memory, 0);

		Utils::ImageHandle handle{};
		handle.m_Handle     = static_cast<void*>(img);
		handle.m_Allocation = static_cast<void*>(memory);
		return handle;
	}

	void* VulkanExternalMemory::getMemoryWin32Handle(const Utils::BufferHandle& r_Buf) {
		return exportMemoryHandle(static_cast<VkDeviceMemory>(r_Buf.m_Allocation));
	}

	void* VulkanExternalMemory::getMemoryWin32Handle(const Utils::ImageHandle& r_Img) {
		return exportMemoryHandle(static_cast<VkDeviceMemory>(r_Img.m_Allocation));
	}

	void VulkanExternalMemory::destroyExportableBuffer(const Utils::BufferHandle& r_Buf, const Utils::AllocationCallbacksDesc& r_Alloc) {
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;
		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
		vkDestroyBuffer(dev, static_cast<VkBuffer>(r_Buf.m_Handle), pA);
		vkFreeMemory   (dev, static_cast<VkDeviceMemory>(r_Buf.m_Allocation), pA);
	}

	void VulkanExternalMemory::destroyExportableImage(const Utils::ImageHandle& r_Img, const Utils::AllocationCallbacksDesc& r_Alloc) {
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;
		VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
		vkDestroyImage(dev, static_cast<VkImage>(r_Img.m_Handle), pA);
		vkFreeMemory  (dev, static_cast<VkDeviceMemory>(r_Img.m_Allocation), pA);
	}
}
