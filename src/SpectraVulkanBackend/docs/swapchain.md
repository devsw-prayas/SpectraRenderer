# Swapchain & Presentation

**Headers:** `Public/VulkanSwapchain.h`  
**Namespace:** `Spectra::Vulkan`

Both classes are static-method collections. `VulkanSurface` wraps a Win32 `VkSurfaceKHR`; `VulkanSwapchain` manages the swapchain lifetime and per-frame acquire/present cycle.

`AllocationCallbacksDesc{}` (zero-init) is always valid and means "no custom callbacks."

---

## VulkanSurface

```cpp
#include "VulkanSwapchain.h"
using namespace Spectra::Vulkan;
using namespace Spectra::Vulkan::Utils;

// Create from a Win32 window. Cast HINSTANCE and HWND to void*.
SurfaceHandle surface = VulkanSurface::createWin32Surface(hinstance, hwnd, {});

// Destroy before the swapchain that uses it, and before shutdownVulkan().
VulkanSurface::destroySurface(surface, {});
```

The surface must outlive any swapchain built on it. Destroy the swapchain first, then the surface, then call `shutdownVulkan`.

---

## VulkanSwapchain

### Create and destroy

```cpp
SwapchainDesc desc{};
desc.m_Surface       = surface;
desc.m_MinImageCount = 3;                      // triple-buffer
desc.m_Format        = Format::B8G8R8A8_SRGB;
desc.m_ColorSpace    = ColorSpace::SRGB_NONLINEAR;
desc.m_Width         = 1920;
desc.m_Height        = 1080;
desc.m_Usage         = ImageUsage::COLOR_ATTACHMENT;
desc.m_PresentMode   = PresentMode::MAILBOX;
desc.m_OldSwapchain  = {};                     // empty on first create

SwapchainHandle swapchain = VulkanSwapchain::createSwapchain(desc, {});

// On destroy, pass {} as old swapchain — the driver recycles internal resources.
VulkanSwapchain::destroySwapchain(swapchain, {});
```

`SwapchainDesc` fields:

| Field | Type | Default | Notes |
|-------|------|---------|-------|
| `m_Surface` | `SurfaceHandle` | `{}` | Must be valid |
| `m_MinImageCount` | `uint32_t` | `2` | Driver may allocate more |
| `m_Format` | `Format` | `B8G8R8A8_SRGB` | Must be supported by surface |
| `m_ColorSpace` | `ColorSpace` | `SRGB_NONLINEAR` | Must match surface capabilities |
| `m_Width / m_Height` | `uint32_t` | `0` | Must equal the current surface extent |
| `m_Usage` | `ImageUsage` | `COLOR_ATTACHMENT` | Bitwise-OR of usage flags |
| `m_PresentMode` | `PresentMode` | `FIFO` | Always supported; MAILBOX is preferred |
| `m_OldSwapchain` | `SwapchainHandle` | `{}` | Pass the retiring swapchain on resize |

Fixed internally: `preTransform = IDENTITY`, `compositeAlpha = OPAQUE`, `imageSharingMode = EXCLUSIVE`, `clipped = VK_TRUE`.

### Retrieve swapchain images

```cpp
ImageHandle images[VulkanSwapchain::MAX_SWAPCHAIN_IMAGES];
uint32_t    imageCount = 0;
VulkanSwapchain::getImages(swapchain, images, imageCount);
```

Swapchain images have `m_Allocation == nullptr` — the swapchain owns their memory. `isValid()` returns `false` by design; only `m_Handle` is meaningful. Use these handles in `VulkanBarrier::cmdImageBarrier` and `RenderingAttachmentDesc::m_ImageView` (via a `VulkanImageView` created over them).

### Per-frame acquire / present loop

```cpp
uint32_t imageIndex = 0;

SwapchainStatus acquireResult =
    VulkanSwapchain::acquireNextImage(swapchain,
                                      imageReadySemaphore,  // signalled when image is safe to write
                                      {},                   // no fence
                                      UINT64_MAX,           // wait indefinitely
                                      imageIndex);

if (acquireResult == SwapchainStatus::OUT_OF_DATE) {
    recreateSwapchain();
    return;
}

// ... record and submit command buffer that writes to images[imageIndex] ...

SwapchainStatus presentResult =
    VulkanSwapchain::present(QueueType::GRAPHICS,
                              swapchain,
                              imageIndex,
                              renderFinishedSemaphore);  // waited before flip

if (presentResult == SwapchainStatus::OUT_OF_DATE ||
    presentResult == SwapchainStatus::SUBOPTIMAL) {
    recreateSwapchain();
}
```

`SwapchainStatus` values:

| Value | Meaning |
|-------|---------|
| `OK` | Normal operation |
| `SUBOPTIMAL` | Presentation succeeded but swapchain no longer matches surface — recreate soon |
| `OUT_OF_DATE` | Cannot present; must recreate swapchain immediately |
| `ERROR` | Unrecoverable Vulkan error |

### Swapchain recreation (resize)

Pass the retiring handle in `m_OldSwapchain` to allow the driver to reclaim resources before the old swapchain is destroyed:

```cpp
void recreateSwapchain() {
    VulkanQueue::deviceWaitIdle();          // flush in-flight work

    SwapchainDesc desc{};
    desc.m_Surface       = surface;
    desc.m_MinImageCount = 3;
    desc.m_Format        = Format::B8G8R8A8_SRGB;
    desc.m_ColorSpace    = ColorSpace::SRGB_NONLINEAR;
    desc.m_Width         = newWidth;
    desc.m_Height        = newHeight;
    desc.m_PresentMode   = PresentMode::MAILBOX;
    desc.m_OldSwapchain  = swapchain;      // hand the old one to the driver

    SwapchainHandle newSwapchain = VulkanSwapchain::createSwapchain(desc, {});

    VulkanSwapchain::destroySwapchain(swapchain, {});   // safe after create
    swapchain = newSwapchain;

    // Re-query images, recreate image views and framebuffer-side resources.
    VulkanSwapchain::getImages(swapchain, images, imageCount);
}
```
