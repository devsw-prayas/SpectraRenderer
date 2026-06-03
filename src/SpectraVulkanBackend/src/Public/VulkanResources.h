#pragma once

#include "VulkanUtils.h"

namespace Spectra::Vulkan {

	// Static resource factories backed by VMA (buffer/image) and direct Vulkan calls (image-view/sampler).
	// All methods are stateless — no instance required.

	class SPEC_VK_BK_RUNTIME_API VulkanBuffer final {
	public:
		// Allocates a VkBuffer + VmaAllocation. m_HostVisible=true maps VMA_MEMORY_USAGE_AUTO to a
		// host-accessible heap (sequential-write pattern). m_Dedicated forces a dedicated allocation.
		// AllocationCallbacksDesc is accepted for API uniformity but is not forwarded to VMA.
		static Utils::BufferHandle createBuffer    (const Utils::BufferDesc& r_Desc, const Utils::AllocationCallbacksDesc& r_Alloc);
		static void*               mapBuffer       (const Utils::BufferHandle& r_Buf);
		static void                unmapBuffer     (const Utils::BufferHandle& r_Buf);
		static uint64_t            getDeviceAddress(const Utils::BufferHandle& r_Buf);
		static void                destroyBuffer   (const Utils::BufferHandle& r_Buf);

		VulkanBuffer() = delete;
		~VulkanBuffer() = delete;
		VulkanBuffer(const VulkanBuffer&) = delete;
		VulkanBuffer& operator=(const VulkanBuffer&) = delete;
		VulkanBuffer(VulkanBuffer&&) noexcept = delete;
		VulkanBuffer& operator=(VulkanBuffer&&) noexcept = delete;
	};

	class SPEC_VK_BK_RUNTIME_API VulkanImage final {
	public:
		// Always creates with VK_IMAGE_TILING_OPTIMAL and VK_IMAGE_LAYOUT_UNDEFINED.
		// AllocationCallbacksDesc is accepted for API uniformity but is not forwarded to VMA.
		static Utils::ImageHandle createImage (const Utils::ImageDesc& r_Desc, const Utils::AllocationCallbacksDesc& r_Alloc);
		static void               destroyImage(const Utils::ImageHandle& r_Img);

		VulkanImage() = delete;
		~VulkanImage() = delete;
		VulkanImage(const VulkanImage&) = delete;
		VulkanImage& operator=(const VulkanImage&) = delete;
		VulkanImage(VulkanImage&&) noexcept = delete;
		VulkanImage& operator=(VulkanImage&&) noexcept = delete;
	};

	class SPEC_VK_BK_RUNTIME_API VulkanImageView final {
	public:
		static Utils::ImageViewHandle createImageView (const Utils::ImageViewDesc& r_Desc, const Utils::AllocationCallbacksDesc& r_Alloc);
		static void                   destroyImageView(const Utils::ImageViewHandle& r_View, const Utils::AllocationCallbacksDesc& r_Alloc);

		VulkanImageView() = delete;
		~VulkanImageView() = delete;
		VulkanImageView(const VulkanImageView&) = delete;
		VulkanImageView& operator=(const VulkanImageView&) = delete;
		VulkanImageView(VulkanImageView&&) noexcept = delete;
		VulkanImageView& operator=(VulkanImageView&&) noexcept = delete;
	};

	class SPEC_VK_BK_RUNTIME_API VulkanSampler final {
	public:
		static Utils::SamplerHandle createSampler (const Utils::SamplerDesc& r_Desc, const Utils::AllocationCallbacksDesc& r_Alloc);
		static void                 destroySampler(const Utils::SamplerHandle& r_Sampler, const Utils::AllocationCallbacksDesc& r_Alloc);

		VulkanSampler() = delete;
		~VulkanSampler() = delete;
		VulkanSampler(const VulkanSampler&) = delete;
		VulkanSampler& operator=(const VulkanSampler&) = delete;
		VulkanSampler(VulkanSampler&&) noexcept = delete;
		VulkanSampler& operator=(VulkanSampler&&) noexcept = delete;
	};
}
