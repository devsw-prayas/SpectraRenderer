# External Memory: Win32 Handle Export

**Header:** `Public/VulkanExternalMemory.h`  
**Namespace:** `Spectra::Vulkan`

Exportable resources are backed by dedicated `VkDeviceMemory` (not VMA). The Win32 handle vended by `getMemoryWin32Handle` is an NT opaque handle (`VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT`) that another API (CUDA, D3D12) can import.

**Important:** resources created through this class store `VkDeviceMemory` in `m_Allocation`, not `VmaAllocation`. Never pass them to `VulkanBuffer::destroyBuffer` or `VulkanImage::destroyImage` — use the matching `destroyExportable*` method.

## Exportable buffer

```cpp
#include "VulkanExternalMemory.h"
using namespace Spectra::Vulkan;
using namespace Spectra::Vulkan::Utils;

// Shared scratch / output buffer for CUDA path tracer + Vulkan display
BufferDesc desc{};
desc.m_Size  = 1920 * 1080 * sizeof(float) * 4;  // RGBA FP32 framebuffer
desc.m_Usage = BufferUsage::STORAGE_BUFFER
             | BufferUsage::TRANSFER_SRC
             | BufferUsage::SHADER_DEVICE_ADDRESS;

BufferHandle sharedBuf = VulkanExternalMemory::createExportableBuffer(desc, {});

// Get the Win32 handle — pass this to CUDA (cuMemImportFromShareableHandle)
void* win32Handle = VulkanExternalMemory::getMemoryWin32Handle(sharedBuf);
// ... hand win32Handle to SpectraRenderPipeline for CUDA import ...

// When the interop session ends, close the Win32 handle
CloseHandle(static_cast<HANDLE>(win32Handle));

// Then destroy the resource
VulkanExternalMemory::destroyExportableBuffer(sharedBuf, {});
```

## Exportable image

```cpp
// Shared render target: CUDA writes, Vulkan samples
ImageDesc imgDesc{};
imgDesc.m_Type      = ImageType::D2;
imgDesc.m_Format    = Format::R32G32B32A32_SFLOAT;
imgDesc.m_Width     = 1920;
imgDesc.m_Height    = 1080;
imgDesc.m_MipLevels = 1;
imgDesc.m_Samples   = SampleCount::x1;
imgDesc.m_Usage     = ImageUsage::STORAGE | ImageUsage::SAMPLED | ImageUsage::TRANSFER_SRC;

ImageHandle sharedImg = VulkanExternalMemory::createExportableImage(imgDesc, {});

void* imgHandle = VulkanExternalMemory::getMemoryWin32Handle(sharedImg);
// ... hand imgHandle to SpectraRenderPipeline for CUDA import ...

CloseHandle(static_cast<HANDLE>(imgHandle));
VulkanExternalMemory::destroyExportableImage(sharedImg, {});
```

The image is created with `VK_IMAGE_TILING_OPTIMAL` and `VK_IMAGE_LAYOUT_UNDEFINED`. Transition to the required layout with `VulkanBarrier::cmdImageBarrier` before first use — see [sync.md](sync.md) for examples.

## Lifetime rules

| Step | Who does it |
|------|-------------|
| Create exportable resource | `VulkanExternalMemory` |
| Export Win32 handle | `VulkanExternalMemory::getMemoryWin32Handle` |
| Import into CUDA | `SpectraRenderPipeline` (outside this module) |
| GPU work with shared resource | normal render / compute commands |
| Close Win32 handle | caller (`CloseHandle`) — after CUDA has imported it |
| Destroy Vulkan resource | `VulkanExternalMemory::destroyExportable*` — only after GPU idle |

The Win32 handle is reference-counted by the OS. Closing it on the Vulkan side is safe once CUDA has duplicated or imported it.
