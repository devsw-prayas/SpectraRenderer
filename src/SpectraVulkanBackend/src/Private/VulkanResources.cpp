#include "SpectraVulkanBackend.h"
#include "VulkanResources.h"
#include "VulkanInternalHelpers.h"
#include "VulkanState.h"

namespace Spectra::Vulkan {

	// -------------------------------------------------------------------------
	// VulkanBuffer
	// -------------------------------------------------------------------------

	Utils::BufferHandle VulkanBuffer::createBuffer(const Utils::BufferDesc& r_Desc, const Utils::AllocationCallbacksDesc& /*r_Alloc*/) {
		VkBufferCreateInfo bufInfo = Internal::vkInit<VkBufferCreateInfo>();
		bufInfo.size        = r_Desc.m_Size;
		bufInfo.usage       = static_cast<VkBufferUsageFlags>(r_Desc.m_Usage);
		bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		if (r_Desc.m_Dedicated)   allocInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		if (r_Desc.m_HostVisible) allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

		VkBuffer      buf{};
		VmaAllocation allocation{};
		vmaCreateBuffer(g_GlobalInstance.m_Allocator.m_Allocator, &bufInfo, &allocInfo, &buf, &allocation, nullptr);

		Utils::BufferHandle handle{};
		handle.m_Handle     = static_cast<void*>(buf);
		handle.m_Allocation = static_cast<void*>(allocation);
		return handle;
	}

	void* VulkanBuffer::mapBuffer(const Utils::BufferHandle& r_Buf) {
		void* ptr = nullptr;
		vmaMapMemory(g_GlobalInstance.m_Allocator.m_Allocator,
		             static_cast<VmaAllocation>(r_Buf.m_Allocation), &ptr);
		return ptr;
	}

	void VulkanBuffer::unmapBuffer(const Utils::BufferHandle& r_Buf) {
		vmaUnmapMemory(g_GlobalInstance.m_Allocator.m_Allocator,
		               static_cast<VmaAllocation>(r_Buf.m_Allocation));
	}

	uint64_t VulkanBuffer::getDeviceAddress(const Utils::BufferHandle& r_Buf) {
		VkBufferDeviceAddressInfo info = Internal::vkInit<VkBufferDeviceAddressInfo>();
		info.buffer = static_cast<VkBuffer>(r_Buf.m_Handle);
		return vkGetBufferDeviceAddress(g_GlobalInstance.m_LogicalDevice.m_Device, &info);
	}

	void VulkanBuffer::destroyBuffer(const Utils::BufferHandle& r_Buf) {
		vmaDestroyBuffer(g_GlobalInstance.m_Allocator.m_Allocator,
		                 static_cast<VkBuffer>(r_Buf.m_Handle),
		                 static_cast<VmaAllocation>(r_Buf.m_Allocation));
	}

	// -------------------------------------------------------------------------
	// VulkanImage
	// -------------------------------------------------------------------------

	Utils::ImageHandle VulkanImage::createImage(const Utils::ImageDesc& r_Desc, const Utils::AllocationCallbacksDesc& /*r_Alloc*/) {
		VkImageCreateInfo imgInfo = Internal::vkInit<VkImageCreateInfo>();
		imgInfo.imageType     = static_cast<VkImageType>(r_Desc.m_Type);
		imgInfo.format        = static_cast<VkFormat>(r_Desc.m_Format);
		imgInfo.extent        = { r_Desc.m_Width, r_Desc.m_Height, r_Desc.m_Depth };
		imgInfo.mipLevels     = r_Desc.m_MipLevels;
		imgInfo.arrayLayers   = r_Desc.m_ArrayLayers;
		imgInfo.samples       = static_cast<VkSampleCountFlagBits>(r_Desc.m_Samples);
		imgInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
		imgInfo.usage         = static_cast<VkImageUsageFlags>(r_Desc.m_Usage);
		imgInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
		imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		if (r_Desc.m_Dedicated) allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

		VkImage       img{};
		VmaAllocation allocation{};
		vmaCreateImage(g_GlobalInstance.m_Allocator.m_Allocator, &imgInfo, &allocInfo, &img, &allocation, nullptr);

		Utils::ImageHandle handle{};
		handle.m_Handle     = static_cast<void*>(img);
		handle.m_Allocation = static_cast<void*>(allocation);
		return handle;
	}

	void VulkanImage::destroyImage(const Utils::ImageHandle& r_Img) {
		vmaDestroyImage(g_GlobalInstance.m_Allocator.m_Allocator,
		                static_cast<VkImage>(r_Img.m_Handle),
		                static_cast<VmaAllocation>(r_Img.m_Allocation));
	}

	// -------------------------------------------------------------------------
	// VulkanImageView
	// -------------------------------------------------------------------------

	Utils::ImageViewHandle VulkanImageView::createImageView(const Utils::ImageViewDesc& r_Desc, const Utils::AllocationCallbacksDesc& r_Alloc) {
		VkImageViewCreateInfo info = Internal::vkInit<VkImageViewCreateInfo>();
		info.image    = static_cast<VkImage>(r_Desc.m_Image.m_Handle);
		info.viewType = static_cast<VkImageViewType>(r_Desc.m_ViewType);
		info.format   = static_cast<VkFormat>(r_Desc.m_Format);
		info.subresourceRange.aspectMask     = static_cast<VkImageAspectFlags>(r_Desc.m_AspectMask);
		info.subresourceRange.baseMipLevel   = r_Desc.m_BaseMip;
		info.subresourceRange.levelCount     = r_Desc.m_MipCount;
		info.subresourceRange.baseArrayLayer = r_Desc.m_BaseLayer;
		info.subresourceRange.layerCount     = r_Desc.m_LayerCount;

		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;

		VkImageView view{};
		vkCreateImageView(g_GlobalInstance.m_LogicalDevice.m_Device, &info, pA, &view);

		Utils::ImageViewHandle handle{};
		handle.m_Handle = static_cast<void*>(view);
		return handle;
	}

	void VulkanImageView::destroyImageView(const Utils::ImageViewHandle& r_View, const Utils::AllocationCallbacksDesc& r_Alloc) {
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;
		vkDestroyImageView(g_GlobalInstance.m_LogicalDevice.m_Device,
		                   static_cast<VkImageView>(r_View.m_Handle), pA);
	}

	// -------------------------------------------------------------------------
	// VulkanSampler
	// -------------------------------------------------------------------------

	Utils::SamplerHandle VulkanSampler::createSampler(const Utils::SamplerDesc& r_Desc, const Utils::AllocationCallbacksDesc& r_Alloc) {
		VkSamplerCreateInfo info = Internal::vkInit<VkSamplerCreateInfo>();
		info.magFilter               = static_cast<VkFilter>(r_Desc.m_MagFilter);
		info.minFilter               = static_cast<VkFilter>(r_Desc.m_MinFilter);
		info.mipmapMode              = static_cast<VkSamplerMipmapMode>(r_Desc.m_MipmapMode);
		info.addressModeU            = static_cast<VkSamplerAddressMode>(r_Desc.m_AddressModeU);
		info.addressModeV            = static_cast<VkSamplerAddressMode>(r_Desc.m_AddressModeV);
		info.addressModeW            = static_cast<VkSamplerAddressMode>(r_Desc.m_AddressModeW);
		info.anisotropyEnable        = r_Desc.m_AnisotropyEnable ? VK_TRUE : VK_FALSE;
		info.maxAnisotropy           = r_Desc.m_MaxAnisotropy;
		info.compareEnable           = r_Desc.m_CompareEnable ? VK_TRUE : VK_FALSE;
		info.compareOp               = static_cast<VkCompareOp>(r_Desc.m_CompareOp);
		info.minLod                  = r_Desc.m_MinLod;
		info.maxLod                  = r_Desc.m_MaxLod;
		info.borderColor             = static_cast<VkBorderColor>(r_Desc.m_BorderColor);
		info.unnormalizedCoordinates = VK_FALSE;

		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;

		VkSampler sampler{};
		vkCreateSampler(g_GlobalInstance.m_LogicalDevice.m_Device, &info, pA, &sampler);

		Utils::SamplerHandle handle{};
		handle.m_Handle = static_cast<void*>(sampler);
		return handle;
	}

	void VulkanSampler::destroySampler(const Utils::SamplerHandle& r_Sampler, const Utils::AllocationCallbacksDesc& r_Alloc) {
		VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
		const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;
		vkDestroySampler(g_GlobalInstance.m_LogicalDevice.m_Device,
		                 static_cast<VkSampler>(r_Sampler.m_Handle), pA);
	}
}
