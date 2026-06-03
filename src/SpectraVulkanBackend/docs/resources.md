# Resources: Buffers, Images, Views, Samplers

**Header:** `Public/VulkanResources.h`  
**Namespace:** `Spectra::Vulkan`

All classes are static-method collections. Buffers and images are backed by VMA; image views and samplers go through direct Vulkan calls with optional allocation callbacks.

`AllocationCallbacksDesc{}` (zero-init) is always valid and means "no custom callbacks."

## VulkanBuffer

```cpp
#include "VulkanResources.h"
using namespace Spectra::Vulkan;
using namespace Spectra::Vulkan::Utils;

// GPU-only storage buffer (device-local, no CPU access)
BufferDesc storageDesc{};
storageDesc.m_Size  = 1024 * 1024;   // 1 MB
storageDesc.m_Usage = BufferUsage::STORAGE_BUFFER | BufferUsage::SHADER_DEVICE_ADDRESS;

BufferHandle storageBuf = VulkanBuffer::createBuffer(storageDesc, {});

// Get the device address for use in shaders / AS build inputs
uint64_t addr = VulkanBuffer::getDeviceAddress(storageBuf);

// Staging buffer (host-visible, for upload)
BufferDesc stagingDesc{};
stagingDesc.m_Size        = 1024 * 1024;
stagingDesc.m_Usage       = BufferUsage::TRANSFER_SRC;
stagingDesc.m_HostVisible = true;

BufferHandle stagingBuf = VulkanBuffer::createBuffer(stagingDesc, {});

// Map, write, unmap
void* ptr = VulkanBuffer::mapBuffer(stagingBuf);
memcpy(ptr, sourceData, dataSize);
VulkanBuffer::unmapBuffer(stagingBuf);

// Destroy immediately (or defer via g_DeferRelease.deferBuffer)
VulkanBuffer::destroyBuffer(stagingBuf);
VulkanBuffer::destroyBuffer(storageBuf);
```

`BufferDesc` fields:

| Field | Type | Default | Notes |
|-------|------|---------|-------|
| `m_Size` | `uint64_t` | `0` | Allocation size in bytes |
| `m_Usage` | `BufferUsage` | `NONE` | Bitwise-OR of usage flags |
| `m_Dedicated` | `bool` | `false` | Forces a dedicated VMA allocation |
| `m_HostVisible` | `bool` | `false` | Maps to a host-accessible heap |

## VulkanImage

```cpp
// 2D RGBA FP16 texture (GPU-only, sampled + transfer destination)
ImageDesc texDesc{};
texDesc.m_Type      = ImageType::D2;
texDesc.m_Format    = Format::R16G16B16A16_SFLOAT;
texDesc.m_Width     = 1920;
texDesc.m_Height    = 1080;
texDesc.m_MipLevels = 1;
texDesc.m_Samples   = SampleCount::x1;
texDesc.m_Usage     = ImageUsage::SAMPLED | ImageUsage::TRANSFER_DST;

ImageHandle tex = VulkanImage::createImage(texDesc, {});

// Depth attachment
ImageDesc depthDesc{};
depthDesc.m_Type      = ImageType::D2;
depthDesc.m_Format    = Format::D32_SFLOAT;
depthDesc.m_Width     = 1920;
depthDesc.m_Height    = 1080;
depthDesc.m_MipLevels = 1;
depthDesc.m_Samples   = SampleCount::x1;
depthDesc.m_Usage     = ImageUsage::DEPTH_STENCIL_ATTACHMENT;

ImageHandle depthImg = VulkanImage::createImage(depthDesc, {});

// Destroy
VulkanImage::destroyImage(tex);
VulkanImage::destroyImage(depthImg);
```

`ImageDesc` fields:

| Field | Type | Default | Notes |
|-------|------|---------|-------|
| `m_Type` | `ImageType` | `D2` | D1 / D2 / D3 |
| `m_Format` | `Format` | `UNDEFINED` | Must be set |
| `m_Width/Height/Depth` | `uint32_t` | `0/0/1` | Depth=1 for 2D |
| `m_MipLevels` | `uint32_t` | `1` | |
| `m_ArrayLayers` | `uint32_t` | `1` | |
| `m_Samples` | `SampleCount` | `x1` | |
| `m_Usage` | `ImageUsage` | `NONE` | Bitwise-OR of usage flags |
| `m_Dedicated` | `bool` | `false` | Forces a dedicated VMA allocation |

All images are created with `VK_IMAGE_TILING_OPTIMAL` and `VK_IMAGE_LAYOUT_UNDEFINED`. Transition to the required layout with `VulkanBarrier::cmdImageBarrier` before first use.

## VulkanImageView

```cpp
ImageViewDesc viewDesc{};
viewDesc.m_Image      = tex;
viewDesc.m_ViewType   = ImageViewType::D2;
viewDesc.m_Format     = Format::R16G16B16A16_SFLOAT;
viewDesc.m_AspectMask = ImageAspect::COLOR;
viewDesc.m_BaseMip    = 0;
viewDesc.m_MipCount   = 1;
viewDesc.m_BaseLayer  = 0;
viewDesc.m_LayerCount = 1;

ImageViewHandle view = VulkanImageView::createImageView(viewDesc, {});

// Depth image view
ImageViewDesc depthViewDesc{};
depthViewDesc.m_Image      = depthImg;
depthViewDesc.m_ViewType   = ImageViewType::D2;
depthViewDesc.m_Format     = Format::D32_SFLOAT;
depthViewDesc.m_AspectMask = ImageAspect::DEPTH;
depthViewDesc.m_BaseMip    = 0;
depthViewDesc.m_MipCount   = 1;
depthViewDesc.m_BaseLayer  = 0;
depthViewDesc.m_LayerCount = 1;

ImageViewHandle depthView = VulkanImageView::createImageView(depthViewDesc, {});

// Destroy
VulkanImageView::destroyImageView(view, {});
VulkanImageView::destroyImageView(depthView, {});
```

## VulkanSampler

```cpp
// Bilinear clamp sampler (common for texture reads in a path tracer)
SamplerDesc samplerDesc{};
samplerDesc.m_MagFilter    = FilterMode::LINEAR;
samplerDesc.m_MinFilter    = FilterMode::LINEAR;
samplerDesc.m_MipmapMode   = MipmapMode::LINEAR;
samplerDesc.m_AddressModeU = AddressMode::CLAMP_TO_EDGE;
samplerDesc.m_AddressModeV = AddressMode::CLAMP_TO_EDGE;
samplerDesc.m_AddressModeW = AddressMode::CLAMP_TO_EDGE;
samplerDesc.m_MinLod       = 0.0f;
samplerDesc.m_MaxLod       = 1000.0f;

SamplerHandle sampler = VulkanSampler::createSampler(samplerDesc, {});

// Anisotropic sampler
SamplerDesc anisoDesc{};
anisoDesc.m_MagFilter          = FilterMode::LINEAR;
anisoDesc.m_MinFilter          = FilterMode::LINEAR;
anisoDesc.m_MipmapMode         = MipmapMode::LINEAR;
anisoDesc.m_AddressModeU       = AddressMode::REPEAT;
anisoDesc.m_AddressModeV       = AddressMode::REPEAT;
anisoDesc.m_AddressModeW       = AddressMode::REPEAT;
anisoDesc.m_AnisotropyEnable   = true;
anisoDesc.m_MaxAnisotropy      = 16.0f;
anisoDesc.m_MinLod             = 0.0f;
anisoDesc.m_MaxLod             = 1000.0f;

SamplerHandle anisoSampler = VulkanSampler::createSampler(anisoDesc, {});

// Destroy
VulkanSampler::destroySampler(sampler, {});
VulkanSampler::destroySampler(anisoSampler, {});
```

## Deferred destruction

Instead of calling destroy immediately, push resources into the deferred queue and let `advance()` flush them once the GPU is done:

```cpp
#include "VulkanDeferredRelease.h"  // internal — RHI code should wrap this in a frame loop helper
using namespace Spectra::Vulkan::Internal;

// At the end of the frame, before submitting new work:
g_DeferRelease.deferBuffer   (oldBuffer);
g_DeferRelease.deferImage    (oldImage);
g_DeferRelease.deferImageView(oldView);
g_DeferRelease.deferSampler  (oldSampler);

// Once per frame, before recording the next frame's commands:
g_DeferRelease.advance();
// Resources deferred 2+ frames ago are now destroyed.
```

Slot capacities per frame: 64 buffers, 32 images, 64 image views, 32 samplers (plus 64 fences and 64 semaphores from the sync layer). Exceeding any limit triggers an assertion.
