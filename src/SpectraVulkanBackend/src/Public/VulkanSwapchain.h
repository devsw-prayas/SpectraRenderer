#pragma once

#include "VulkanUtils.h"

namespace Spectra::Vulkan {

	// Platform surface wrapper. Win32 is the only supported backend on Windows.
	class SPEC_VK_BK_RUNTIME_API VulkanSurface final {
	public:
		// v_Hinstance / v_Hwnd are the Win32 HINSTANCE and HWND cast to void*.
		static Utils::SurfaceHandle createWin32Surface(void* v_Hinstance, void* v_Hwnd,
		                                               const Utils::AllocationCallbacksDesc& r_Alloc);
		static void destroySurface(const Utils::SurfaceHandle& r_Surface,
		                           const Utils::AllocationCallbacksDesc& r_Alloc);

		VulkanSurface() = delete;
		~VulkanSurface() = delete;
		VulkanSurface(const VulkanSurface&) = delete;
		VulkanSurface& operator=(const VulkanSurface&) = delete;
		VulkanSurface(VulkanSurface&&) noexcept = delete;
		VulkanSurface& operator=(VulkanSurface&&) noexcept = delete;
	};

	// Swapchain management and per-frame acquire/present.
	class SPEC_VK_BK_RUNTIME_API VulkanSwapchain final {
	public:
		static constexpr uint32_t MAX_SWAPCHAIN_IMAGES = 8;

		static Utils::SwapchainHandle createSwapchain(const Utils::SwapchainDesc& r_Desc,
		                                              const Utils::AllocationCallbacksDesc& r_Alloc);
		static void destroySwapchain(const Utils::SwapchainHandle& r_Swapchain,
		                             const Utils::AllocationCallbacksDesc& r_Alloc);

		// Fills p_Out[0..r_Count-1] with swapchain image handles.
		// Swapchain images have m_Allocation == nullptr — the swapchain owns their memory,
		// so isValid() returns false. Only m_Handle is meaningful for barriers and rendering.
		static void getImages(const Utils::SwapchainHandle& r_Swapchain,
		                      Utils::ImageHandle* p_Out, uint32_t& r_Count);

		// Acquires the next presentable image. r_Signal and r_Fence may each be empty ({}).
		// v_Timeout is in nanoseconds; pass UINT64_MAX to wait indefinitely.
		// Returns OUT_OF_DATE when the swapchain must be recreated (window resize, etc.).
		static Utils::SwapchainStatus acquireNextImage(const Utils::SwapchainHandle& r_Swapchain,
		                                               const Utils::SemaphoreHandle& r_Signal,
		                                               const Utils::FenceHandle&     r_Fence,
		                                               uint64_t v_Timeout,
		                                               uint32_t& r_ImageIndex);

		// Queues a present on the given queue type. r_Wait may be empty ({}).
		// Returns OUT_OF_DATE / SUBOPTIMAL when swapchain recreation is advisable.
		static Utils::SwapchainStatus present(Utils::QueueType v_Queue,
		                                      const Utils::SwapchainHandle& r_Swapchain,
		                                      uint32_t v_ImageIndex,
		                                      const Utils::SemaphoreHandle& r_Wait);

		VulkanSwapchain() = delete;
		~VulkanSwapchain() = delete;
		VulkanSwapchain(const VulkanSwapchain&) = delete;
		VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;
		VulkanSwapchain(VulkanSwapchain&&) noexcept = delete;
		VulkanSwapchain& operator=(VulkanSwapchain&&) noexcept = delete;
	};
}
