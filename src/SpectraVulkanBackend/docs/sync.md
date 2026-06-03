# Synchronisation

**Header:** `Public/VulkanSync.h`

Three static classes covering all CPU/GPU synchronisation needs. All live in `Spectra::Vulkan::Sync`.

## VulkanFence

CPU-visible gate on GPU completion. Use one fence per frame-in-flight.

```cpp
#include "VulkanSync.h"
using namespace Spectra::Vulkan::Sync;
using namespace Spectra::Vulkan::Utils;

AllocationCallbacksDesc noAlloc{};  // zero-init = no custom allocator

// Create pre-signaled (frame 0 doesn't need to wait)
FenceDesc desc{};
desc.m_PreSignaled = true;
FenceHandle fence = VulkanFence::createFence(desc, noAlloc);

// Per-frame pattern
VulkanFence::waitForFences(1, &fence, /*waitAll=*/true, /*timeout=*/UINT64_MAX);
VulkanFence::resetFences(1, &fence);

// Submit GPU work, passing fence to vkQueueSubmit2 (see queue submission doc)
// ...

// Poll without blocking (e.g. for streaming)
bool signaled = VulkanFence::getFenceStatus(fence);  // true = GPU done

// Shutdown
VulkanFence::destroyFence(fence, noAlloc);
```

**Batch wait** (waiting on multiple fences at once):

```cpp
FenceHandle fences[2] = { frameA, frameB };
VulkanFence::waitForFences(2, fences, /*waitAll=*/false, UINT64_MAX);
// Returns as soon as ANY fence signals (waitAll=false)
```

## VulkanTimelineSemaphore

Monotonically increasing counter semaphore. Use for cross-queue ordering and Vulkan/CUDA synchronisation.

### Basic usage

```cpp
TimelineSemaphoreDesc desc{};
desc.m_InitialValue  = 0;
desc.m_ExportForCuda = false;

SemaphoreHandle sem = VulkanTimelineSemaphore::create(desc, noAlloc);

// CPU signal (unblock a CPU waiter or advance a GPU queue's wait value)
VulkanTimelineSemaphore::signal(sem, 1);

// CPU wait (block until semaphore reaches value 1)
VulkanTimelineSemaphore::wait(sem, 1, /*timeout=*/UINT64_MAX);

// Poll current counter without blocking
uint64_t current = VulkanTimelineSemaphore::getCounter(sem);

VulkanTimelineSemaphore::destroy(sem, noAlloc);
```

### CUDA interop

Export the semaphore as a Win32 HANDLE for `cudaImportExternalSemaphore`:

```cpp
TimelineSemaphoreDesc desc{};
desc.m_InitialValue  = 0;
desc.m_ExportForCuda = true;   // chains VkExportSemaphoreCreateInfo

SemaphoreHandle sem = VulkanTimelineSemaphore::create(desc, noAlloc);

void* win32Handle = VulkanTimelineSemaphore::getWin32Handle(sem);
// win32Handle is a Win32 HANDLE (void* = HANDLE on Windows)
// Pass to: cudaImportExternalSemaphore(&cudaSem, &importDesc)

// Typical per-frame cross-API pattern:
//   Vulkan signals timeline value N on graphics queue
//   CUDA waits on same value N before starting path trace
//   CUDA signals value N+1 when done
//   Vulkan waits on N+1 before compositing result
```

## VulkanBarrier

Wraps `vkCmdPipelineBarrier2`. All barrier recording goes through here. Never call `vkCmdPipelineBarrier` directly.

**All types referenced below are in `Spectra::Vulkan::Utils`.**

### Image barrier (most common: layout transitions and read/write hazards)

```cpp
ImageBarrierDesc barrier{};
barrier.m_SrcStage  = PipelineStage::TOP_OF_PIPE;
barrier.m_DstStage  = PipelineStage::COLOR_ATTACHMENT_OUTPUT;
barrier.m_SrcAccess = AccessType::NONE;
barrier.m_DstAccess = AccessType::COLOR_ATTACHMENT_WRITE;
barrier.m_OldLayout = ImageLayout::UNDEFINED;
barrier.m_NewLayout = ImageLayout::COLOR_ATTACHMENT;
barrier.m_Image     = myRenderTarget;        // ImageHandle
barrier.m_Aspect    = ImageAspect::COLOR;
barrier.m_BaseMip   = 0;
barrier.m_MipCount  = 1;
barrier.m_BaseLayer = 0;
barrier.m_LayerCount = 1;
// m_SrcQueueFamily / m_DstQueueFamily default to ~0u (VK_QUEUE_FAMILY_IGNORED)

VulkanBarrier::cmdImageBarrier(cmdBuf, barrier);
```

### Buffer barrier (storage buffer write to shader read)

```cpp
BufferBarrierDesc barrier{};
barrier.m_SrcStage  = PipelineStage::COMPUTE_SHADER;
barrier.m_DstStage  = PipelineStage::RAY_TRACING_SHADER;
barrier.m_SrcAccess = AccessType::SHADER_WRITE;
barrier.m_DstAccess = AccessType::SHADER_READ;
barrier.m_Buffer    = myBuffer;              // BufferHandle
barrier.m_Offset    = 0;
barrier.m_Size      = ~0ULL;                 // VK_WHOLE_SIZE

VulkanBarrier::cmdBufferBarrier(cmdBuf, barrier);
```

### Memory barrier (execution dependency, no resource)

```cpp
MemoryBarrierDesc barrier{};
barrier.m_SrcStage  = PipelineStage::ACCELERATION_STRUCTURE_BUILD;
barrier.m_DstStage  = PipelineStage::RAY_TRACING_SHADER;
barrier.m_SrcAccess = AccessType::ACCELERATION_STRUCTURE_WRITE;
barrier.m_DstAccess = AccessType::ACCELERATION_STRUCTURE_READ;

VulkanBarrier::cmdMemoryBarrier(cmdBuf, barrier);
```

### Batch barrier (multiple resources in one `vkCmdPipelineBarrier2` call)

```cpp
ImageBarrierDesc imgBarriers[3] = { ... };
BufferBarrierDesc bufBarriers[1] = { ... };

VulkanBarrier::cmdPipelineBarrier(
    cmdBuf,
    /*memCount=*/0,  /*pMem=*/nullptr,
    /*bufCount=*/1,  bufBarriers,
    /*imgCount=*/3,  imgBarriers);
// All barriers go into a single vkCmdPipelineBarrier2 call.
// Prefer this over multiple single-barrier calls in tight loops.
```

Max 32 barriers per type per call (stack-allocated internally).

## DeferReleaseQueue

**Header:** `Internal/VulkanDeferredRelease.h` (internal, not for RHI public API)

Ring buffer for safe deferred destruction of resources that may still be in GPU flight. Call `advance()` once per frame at the top of your frame loop.

```cpp
#include "VulkanDeferredRelease.h"
using namespace Spectra::Vulkan::Internal;

// Frame start
g_DeferRelease.advance();
// Destroys everything deferred FRAMES_IN_FLIGHT frames ago.
// Safe because the GPU can no longer reference those resources.

// Instead of: VulkanFence::destroyFence(oldFence, noAlloc);
g_DeferRelease.deferFence(oldFence);

// Same for semaphores:
g_DeferRelease.deferSemaphore(oldSem);
```

`FRAMES_IN_FLIGHT = 2`. Each slot holds up to 64 fences and 64 semaphores. Buffer, image, and image-view slots will be added when those resource classes land.

### Queue family ownership transfers

For cross-queue barriers, set `m_SrcQueueFamily` and `m_DstQueueFamily` on buffer or image barriers. Submit the **release** barrier on the source queue, then the **acquire** barrier on the destination queue, with a semaphore signal/wait between them:

```cpp
// On graphics queue command buffer:
ImageBarrierDesc release{};
release.m_SrcQueueFamily = graphicsFamily;
release.m_DstQueueFamily = computeFamily;
release.m_OldLayout      = ImageLayout::COLOR_ATTACHMENT;
release.m_NewLayout      = ImageLayout::GENERAL;
// ... stages / access ...
VulkanBarrier::cmdImageBarrier(graphicsCmdBuf, release);
// Submit graphicsCmdBuf, signal timeline semaphore value N

// On compute queue command buffer (after waiting on value N):
ImageBarrierDesc acquire = release;   // same fields
VulkanBarrier::cmdImageBarrier(computeCmdBuf, acquire);
// GPU now owns the image on the compute queue
```
