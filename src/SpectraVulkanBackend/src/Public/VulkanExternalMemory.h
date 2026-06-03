#pragma once

#include "VulkanUtils.h"

namespace Spectra::Vulkan {

	// Vulkan-side external memory export.
	// Creates buffers/images with dedicated VkDeviceMemory (bypassing VMA — required for export),
	// then vends the Win32 HANDLE for handoff to another API (CUDA, D3D12, etc.).
	//
	// Handle layout for exportable resources:
	//   m_Handle     = VkBuffer / VkImage   (as usual)
	//   m_Allocation = VkDeviceMemory       (NOT VmaAllocation — destroy via this class only)
	//
	// Caller owns the void* returned by getMemoryWin32Handle and must close it
	// with CloseHandle() when the interop session ends.

	class SPEC_VK_BK_RUNTIME_API VulkanExternalMemory final {
	public:
		// Exportable buffer — dedicated device-local allocation, VkExternalMemoryBufferCreateInfo chained.
		// Use SHADER_DEVICE_ADDRESS | STORAGE_BUFFER for CUDA interop scratch/output buffers.
		static Utils::BufferHandle createExportableBuffer(const Utils::BufferDesc& r_Desc, const Utils::AllocationCallbacksDesc& r_Alloc);

		// Exportable image — dedicated device-local allocation, VkExternalMemoryImageCreateInfo chained.
		// Always TILING_OPTIMAL / LAYOUT_UNDEFINED on creation; transition before first use.
		static Utils::ImageHandle  createExportableImage (const Utils::ImageDesc& r_Desc, const Utils::AllocationCallbacksDesc& r_Alloc);

		// Returns an NT Win32 HANDLE (VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT).
		static void* getMemoryWin32Handle(const Utils::BufferHandle& r_Buf);
		static void* getMemoryWin32Handle(const Utils::ImageHandle&  r_Img);

		// Destroy via vkDestroyBuffer/Image + vkFreeMemory — do NOT pass to VulkanBuffer::destroyBuffer.
		static void destroyExportableBuffer(const Utils::BufferHandle& r_Buf, const Utils::AllocationCallbacksDesc& r_Alloc);
		static void destroyExportableImage (const Utils::ImageHandle&  r_Img, const Utils::AllocationCallbacksDesc& r_Alloc);

		VulkanExternalMemory() = delete;
		~VulkanExternalMemory() = delete;
		VulkanExternalMemory(const VulkanExternalMemory&) = delete;
		VulkanExternalMemory& operator=(const VulkanExternalMemory&) = delete;
		VulkanExternalMemory(VulkanExternalMemory&&) noexcept = delete;
		VulkanExternalMemory& operator=(VulkanExternalMemory&&) noexcept = delete;
	};
}
