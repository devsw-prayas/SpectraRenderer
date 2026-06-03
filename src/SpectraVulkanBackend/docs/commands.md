# Command Recording

**Header:** `Public/VulkanCommands.h`  
**Namespace:** `Spectra::Vulkan`

## Lifecycle

```cpp
#include "VulkanCommands.h"
using namespace Spectra::Vulkan;
using namespace Spectra::Vulkan::Utils;

// Allocate from the graphics queue pool
CommandBufferHandle cmd = VulkanCommandBuffer::allocate(QueueType::GRAPHICS);

// One-time submit (transfer, staging uploads)
VulkanCommandBuffer::begin(cmd);
// ... record ...
VulkanCommandBuffer::end(cmd);

// Per-frame: reset and re-record each frame
VulkanCommandBuffer::reset(cmd);
VulkanCommandBuffer::beginReusable(cmd);
// ... record ...
VulkanCommandBuffer::end(cmd);

// Free when done
VulkanCommandBuffer::free(cmd, QueueType::GRAPHICS);
```

## Dynamic rendering pass

```cpp
RenderingAttachmentDesc colorAttach{};
colorAttach.m_ImageView   = swapchainView;
colorAttach.m_Layout      = ImageLayout::COLOR_ATTACHMENT;
colorAttach.m_LoadOp      = LoadOp::CLEAR;
colorAttach.m_StoreOp     = StoreOp::STORE;
colorAttach.m_ClearValue.m_Color[0] = 0.0f;
colorAttach.m_ClearValue.m_Color[1] = 0.0f;
colorAttach.m_ClearValue.m_Color[2] = 0.0f;
colorAttach.m_ClearValue.m_Color[3] = 1.0f;

RenderingAttachmentDesc depthAttach{};
depthAttach.m_ImageView                    = depthView;
depthAttach.m_Layout                       = ImageLayout::DEPTH_STENCIL_ATTACHMENT;
depthAttach.m_LoadOp                       = LoadOp::CLEAR;
depthAttach.m_StoreOp                      = StoreOp::DONT_CARE;
depthAttach.m_ClearValue.m_DepthStencil.m_Depth = 1.0f;

RenderingDesc renderDesc{};
renderDesc.m_Width                  = 1920;
renderDesc.m_Height                 = 1080;
renderDesc.m_ColorAttachments       = &colorAttach;
renderDesc.m_ColorAttachmentCount   = 1;
renderDesc.m_DepthAttachment        = &depthAttach;

VulkanCommandBuffer::beginRendering(cmd, renderDesc);

// ... draw calls inside ...

VulkanCommandBuffer::endRendering(cmd);
```

## Graphics draw

```cpp
VulkanCommandBuffer::bindPipeline(cmd, gfxPipeline, PipelineBindPoint::GRAPHICS);

VulkanCommandBuffer::setViewport(cmd, 0.0f, 0.0f, 1920.0f, 1080.0f, 0.0f, 1.0f);
VulkanCommandBuffer::setScissor (cmd, 0, 0, 1920, 1080);

VulkanCommandBuffer::bindDescriptorSets(cmd, layout, PipelineBindPoint::GRAPHICS,
                                         /*firstSet=*/0, 1, &descSet);

glm::mat4 mvp = proj * view * model;
VulkanCommandBuffer::pushConstants(cmd, layout, ShaderStage::VERTEX,
                                    /*offset=*/0, 64, &mvp);

uint64_t vbOffset = 0;
VulkanCommandBuffer::bindVertexBuffers(cmd, 0, 1, &vertexBuf, &vbOffset);
VulkanCommandBuffer::bindIndexBuffer  (cmd, indexBuf, 0, IndexType::UINT32);

VulkanCommandBuffer::drawIndexed(cmd, indexCount, 1, 0, 0, 0);
```

## Compute dispatch

```cpp
VulkanCommandBuffer::bindPipeline    (cmd, computePipeline, PipelineBindPoint::COMPUTE);
VulkanCommandBuffer::bindDescriptorSets(cmd, layout, PipelineBindPoint::COMPUTE, 0, 1, &descSet);
VulkanCommandBuffer::dispatch(cmd, (width + 7) / 8, (height + 7) / 8, 1);
```

## Ray tracing dispatch

```cpp
VulkanCommandBuffer::bindPipeline(cmd, rtPipeline, PipelineBindPoint::RAY_TRACING);
VulkanCommandBuffer::bindDescriptorSets(cmd, layout, PipelineBindPoint::RAY_TRACING, 0, 1, &descSet);

ShaderBindingTableRegion raygen   { raygenAddr,   handleStride, handleSize };
ShaderBindingTableRegion miss     { missAddr,     handleStride, handleSize };
ShaderBindingTableRegion hit      { hitAddr,      handleStride, handleSize };
ShaderBindingTableRegion callable {};   // unused

VulkanCommandBuffer::dispatchRays(cmd, raygen, miss, hit, callable, 1920, 1080, 1);
```

## Transfer

```cpp
// Staging buffer → GPU buffer
VulkanCommandBuffer::copyBuffer(cmd, stagingBuf, gpuBuf, 0, 0, dataSize);

// Staging buffer → texture (must be in TRANSFER_DST layout first)
VulkanCommandBuffer::copyBufferToImage(cmd,
    stagingBuf, 0,
    texImage, ImageLayout::TRANSFER_DST,
    1024, 1024,
    ImageAspect::COLOR, /*mip=*/0, /*layer=*/0);

// Blit mip N → mip N+1 (for mip chain generation)
VulkanCommandBuffer::blitImage(cmd,
    texImage, ImageLayout::TRANSFER_SRC,
    texImage, ImageLayout::TRANSFER_DST,
    /*srcW=*/512, /*srcH=*/512, /*srcMip=*/0,
    /*dstW=*/256, /*dstH=*/256, /*dstMip=*/1,
    ImageAspect::COLOR, FilterMode::LINEAR);
```
