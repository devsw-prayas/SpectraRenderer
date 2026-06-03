# Queue Submission

**Header:** `Public/VulkanQueue.h`  
**Namespace:** `Spectra::Vulkan`

All submission goes through `VkQueueSubmit2` with timeline semaphore support.

## Basic submit (no sync)

```cpp
#include "VulkanQueue.h"
using namespace Spectra::Vulkan;
using namespace Spectra::Vulkan::Utils;

SubmitDesc desc{};
desc.m_CommandBuffers     = &cmd;
desc.m_CommandBufferCount = 1;

// Signal a fence so the CPU can detect completion
VulkanQueue::submit(QueueType::GRAPHICS, desc, frameFence);

// Wait on the CPU side
VulkanSync::Fence::waitForFences(1, &frameFence, true, UINT64_MAX);
VulkanSync::Fence::resetFences  (1, &frameFence);
```

## Submit with timeline semaphore sync

```cpp
// Frame N: GPU work signals renderDone at value N
uint64_t renderValue = frameIndex;

SubmitDesc desc{};
desc.m_CommandBuffers     = &cmd;
desc.m_CommandBufferCount = 1;
desc.m_SignalSemaphores   = &renderDoneSem;
desc.m_SignalValues       = &renderValue;
desc.m_SignalStages       = nullptr;   // defaults to ALL_COMMANDS
desc.m_SignalCount        = 1;

VulkanQueue::submit(QueueType::GRAPHICS, desc, {});

// Frame N+1: compute waits for render to finish before writing the previous result
uint64_t waitValue = frameIndex;
PipelineStage waitStage = PipelineStage::COMPUTE_SHADER;

SubmitDesc computeDesc{};
computeDesc.m_CommandBuffers     = &computeCmd;
computeDesc.m_CommandBufferCount = 1;
computeDesc.m_WaitSemaphores     = &renderDoneSem;
computeDesc.m_WaitValues         = &waitValue;
computeDesc.m_WaitStages         = &waitStage;
computeDesc.m_WaitCount          = 1;

VulkanQueue::submit(QueueType::COMPUTE, computeDesc, {});
```

## Transfer queue upload pattern

```cpp
// Upload on transfer queue, signal when done, graphics waits before sampling
uint64_t uploadValue = 1;
PipelineStage samplerStage = PipelineStage::FRAGMENT_SHADER;

SubmitDesc uploadDesc{};
uploadDesc.m_CommandBuffers     = &transferCmd;
uploadDesc.m_CommandBufferCount = 1;
uploadDesc.m_SignalSemaphores   = &uploadDoneSem;
uploadDesc.m_SignalValues       = &uploadValue;
uploadDesc.m_SignalCount        = 1;

VulkanQueue::submit(QueueType::TRANSFER, uploadDesc, {});

SubmitDesc renderDesc{};
renderDesc.m_CommandBuffers     = &graphicsCmd;
renderDesc.m_CommandBufferCount = 1;
renderDesc.m_WaitSemaphores     = &uploadDoneSem;
renderDesc.m_WaitValues         = &uploadValue;
renderDesc.m_WaitStages         = &samplerStage;
renderDesc.m_WaitCount          = 1;

VulkanQueue::submit(QueueType::GRAPHICS, renderDesc, {});
```

## Device idle

```cpp
// Drain a single queue before destroying its resources
VulkanQueue::waitIdle(QueueType::GRAPHICS);

// Drain everything before shutdown
VulkanQueue::deviceWaitIdle();
```

`SubmitDesc` limits: up to `MAX_SUBMIT_SEMAPHORES` (16) wait/signal semaphores and `MAX_SUBMIT_CMDS` (8) command buffers per submit call.
