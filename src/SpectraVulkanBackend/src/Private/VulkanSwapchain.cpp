#include "SpectraVulkanBackend.h"
#include "VulkanSwapchain.h"
#include "VulkanInternalHelpers.h"
#include "VulkanState.h"

namespace Spectra::Vulkan {

	// -------------------------------------------------------------------------
	// Internal helpers
	// -------------------------------------------------------------------------

	static VkQueue swapchainQueueForType(Utils::QueueType v_Type) {
		switch (v_Type) {
			case Utils::QueueType::COMPUTE:  return g_GlobalInstance.m_LogicalDevice.m_ComputeQueue;
			case Utils::QueueType::TRANSFER: return g_GlobalInstance.m_LogicalDevice.m_TransferQueue;
			default:                         return g_GlobalInstance.m_LogicalDevice.m_GraphicsQueue;
		}
	}

	// -------------------------------------------------------------------------
	// VulkanSurface
	// -------------------------------------------------------------------------

	Utils::SurfaceHandle VulkanSurface::createWin32Surface(void* v_Hinstance, void* v_Hwnd,
	                                                       const Utils::AllocationCallbacksDesc& r_Alloc) {
		VkWin32SurfaceCreateInfoKHR info = Internal::vkInit<VkWin32SurfaceCreateInfoKHR>();
		info.hinstance = static_cast<HINSTANCE>(v_Hinstance);
		info.hwnd      = static_cast<HWND>(v_Hwnd);

		VkAllocationCallbacks  allocStorage = Internal::toVkAllocationCallbacks(r_Alloc);
		VkAllocationCallbacks* pAlloc       = r_Alloc.m_pfnAllocation ? &allocStorage : nullptr;

		VkSurfaceKHR surface{};
		vkCreateWin32SurfaceKHR(g_GlobalInstance.m_GlobalInstance, &info, pAlloc, &surface);

		Utils::SurfaceHandle handle{};
		handle.m_Handle = static_cast<void*>(surface);
		return handle;
	}

	void VulkanSurface::destroySurface(const Utils::SurfaceHandle& r_Surface,
	                                   const Utils::AllocationCallbacksDesc& r_Alloc) {
		VkAllocationCallbacks  allocStorage = Internal::toVkAllocationCallbacks(r_Alloc);
		VkAllocationCallbacks* pAlloc       = r_Alloc.m_pfnAllocation ? &allocStorage : nullptr;

		vkDestroySurfaceKHR(g_GlobalInstance.m_GlobalInstance,
		                    static_cast<VkSurfaceKHR>(r_Surface.m_Handle), pAlloc);
	}

	// -------------------------------------------------------------------------
	// VulkanSwapchain
	// -------------------------------------------------------------------------

	Utils::SwapchainHandle VulkanSwapchain::createSwapchain(const Utils::SwapchainDesc& r_Desc,
	                                                        const Utils::AllocationCallbacksDesc& r_Alloc) {
		VkSwapchainCreateInfoKHR info = Internal::vkInit<VkSwapchainCreateInfoKHR>();
		info.surface          = static_cast<VkSurfaceKHR>(r_Desc.m_Surface.m_Handle);
		info.minImageCount    = r_Desc.m_MinImageCount;
		info.imageFormat      = static_cast<VkFormat>(r_Desc.m_Format);
		info.imageColorSpace  = static_cast<VkColorSpaceKHR>(r_Desc.m_ColorSpace);
		info.imageExtent      = { r_Desc.m_Width, r_Desc.m_Height };
		info.imageArrayLayers = 1;
		info.imageUsage       = static_cast<VkImageUsageFlags>(r_Desc.m_Usage);
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.presentMode      = static_cast<VkPresentModeKHR>(r_Desc.m_PresentMode);
		info.clipped          = VK_TRUE;
		info.oldSwapchain     = r_Desc.m_OldSwapchain.m_Handle
		                          ? static_cast<VkSwapchainKHR>(r_Desc.m_OldSwapchain.m_Handle)
		                          : VK_NULL_HANDLE;

		VkAllocationCallbacks  allocStorage = Internal::toVkAllocationCallbacks(r_Alloc);
		VkAllocationCallbacks* pAlloc       = r_Alloc.m_pfnAllocation ? &allocStorage : nullptr;

		VkSwapchainKHR swapchain{};
		vkCreateSwapchainKHR(g_GlobalInstance.m_LogicalDevice.m_Device, &info, pAlloc, &swapchain);

		Utils::SwapchainHandle handle{};
		handle.m_Handle = static_cast<void*>(swapchain);
		return handle;
	}

	void VulkanSwapchain::destroySwapchain(const Utils::SwapchainHandle& r_Swapchain,
	                                       const Utils::AllocationCallbacksDesc& r_Alloc) {
		VkAllocationCallbacks  allocStorage = Internal::toVkAllocationCallbacks(r_Alloc);
		VkAllocationCallbacks* pAlloc       = r_Alloc.m_pfnAllocation ? &allocStorage : nullptr;

		vkDestroySwapchainKHR(g_GlobalInstance.m_LogicalDevice.m_Device,
		                      static_cast<VkSwapchainKHR>(r_Swapchain.m_Handle), pAlloc);
	}

	void VulkanSwapchain::getImages(const Utils::SwapchainHandle& r_Swapchain,
	                                Utils::ImageHandle* p_Out, uint32_t& r_Count) {
		VkSwapchainKHR swapchain = static_cast<VkSwapchainKHR>(r_Swapchain.m_Handle);
		VkDevice       device    = g_GlobalInstance.m_LogicalDevice.m_Device;

		uint32_t count = 0;
		vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
		if (count > MAX_SWAPCHAIN_IMAGES) count = MAX_SWAPCHAIN_IMAGES;

		VkImage images[MAX_SWAPCHAIN_IMAGES]{};
		vkGetSwapchainImagesKHR(device, swapchain, &count, images);

		r_Count = count;
		for (uint32_t i = 0; i < count; ++i) {
			p_Out[i].m_Handle     = static_cast<void*>(images[i]);
			p_Out[i].m_Allocation = nullptr;
		}
	}

	Utils::SwapchainStatus VulkanSwapchain::acquireNextImage(const Utils::SwapchainHandle& r_Swapchain,
	                                                         const Utils::SemaphoreHandle& r_Signal,
	                                                         const Utils::FenceHandle&     r_Fence,
	                                                         uint64_t v_Timeout,
	                                                         uint32_t& r_ImageIndex) {
		VkSemaphore signalSem = r_Signal.m_Handle ? static_cast<VkSemaphore>(r_Signal.m_Handle) : VK_NULL_HANDLE;
		VkFence     fence     = r_Fence.m_Handle  ? static_cast<VkFence>(r_Fence.m_Handle)       : VK_NULL_HANDLE;

		VkResult result = vkAcquireNextImageKHR(g_GlobalInstance.m_LogicalDevice.m_Device,
		                                        static_cast<VkSwapchainKHR>(r_Swapchain.m_Handle),
		                                        v_Timeout, signalSem, fence, &r_ImageIndex);

		if (result == VK_SUCCESS)              return Utils::SwapchainStatus::OK;
		if (result == VK_SUBOPTIMAL_KHR)       return Utils::SwapchainStatus::SUBOPTIMAL;
		if (result == VK_ERROR_OUT_OF_DATE_KHR) return Utils::SwapchainStatus::OUT_OF_DATE;
		return Utils::SwapchainStatus::ERROR;
	}

	Utils::SwapchainStatus VulkanSwapchain::present(Utils::QueueType v_Queue,
	                                                const Utils::SwapchainHandle& r_Swapchain,
	                                                uint32_t v_ImageIndex,
	                                                const Utils::SemaphoreHandle& r_Wait) {
		VkSwapchainKHR swapchain = static_cast<VkSwapchainKHR>(r_Swapchain.m_Handle);
		VkSemaphore    waitSem   = r_Wait.m_Handle ? static_cast<VkSemaphore>(r_Wait.m_Handle) : VK_NULL_HANDLE;

		VkPresentInfoKHR info  = Internal::vkInit<VkPresentInfoKHR>();
		info.waitSemaphoreCount = r_Wait.m_Handle ? 1u : 0u;
		info.pWaitSemaphores    = r_Wait.m_Handle ? &waitSem : nullptr;
		info.swapchainCount     = 1;
		info.pSwapchains        = &swapchain;
		info.pImageIndices      = &v_ImageIndex;

		VkResult result = vkQueuePresentKHR(swapchainQueueForType(v_Queue), &info);

		if (result == VK_SUCCESS)               return Utils::SwapchainStatus::OK;
		if (result == VK_SUBOPTIMAL_KHR)        return Utils::SwapchainStatus::SUBOPTIMAL;
		if (result == VK_ERROR_OUT_OF_DATE_KHR) return Utils::SwapchainStatus::OUT_OF_DATE;
		return Utils::SwapchainStatus::ERROR;
	}
}
