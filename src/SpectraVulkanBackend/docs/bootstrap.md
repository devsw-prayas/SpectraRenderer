# Bootstrap & Teardown

**Header:** `Public/VulkanBootstrap.h`

The five init calls must run in order. Teardown is a single call that reverses all of them.

## Startup

```cpp
#include "VulkanBootstrap.h"

// 1. Instance + registry
Utils::InitDesc desc{};
desc.m_ApplicationName = "SpectraEditor";
desc.m_EnableValidation = true;   // false in Release

InstanceHandle instance = VulkanBootstrap::initializeVulkan(desc);

// 2. GPU selection (scores discrete GPUs by VRAM, RT depth, dedicated queues)
VulkanBootstrap::selectPhysicalDevice();

// 3. VkDevice + graphics / compute / transfer queues
VulkanBootstrap::createLogicalDevice();

// 4. VMA allocator (buffer device address + memory budget always enabled)
VulkanBootstrap::createAllocator();

// 5. One command pool per unique queue family
VulkanBootstrap::createCommandPools();
```

`initializeVulkan` returns an `InstanceHandle`. Keep it and pass it to `shutdownVulkan`. Everything else lives in the internal `g_GlobalInstance` global; RHI code never needs to store the handle itself beyond passing it back at shutdown.

## Teardown

```cpp
VulkanBootstrap::shutdownVulkan(instance);
// Destroys: command pools -> VMA -> logical device -> debug messenger -> VkInstance
```

Call `vkDeviceWaitIdle` (or drain your `DeferReleaseQueue`) before this if any GPU work may still be in flight. The RHI frame loop should guarantee this. `shutdownVulkan` does not wait for you.

## Validation

Validation is enabled when `m_EnableValidation = true`. The debug messenger fires `SPEC_VK_BK_ASSERT(false)` on any error-severity message, so validation failures are immediately fatal in debug builds. Warnings are recorded but do not assert.

Typical config:

| Build | `m_EnableValidation` |
|-------|---------------------|
| `Debug_win64` | `true` |
| `Release_win64` | `false` |
| `Release_Kerbecs_*` | `true` |

## Querying device capabilities from the RHI

After `selectPhysicalDevice()`, device info is in `g_GlobalInstance.m_DeviceProps` (internal, behind `ALLOW_SYSCALL`). The RHI should query what it needs through a future `VulkanDevice::getCapabilities()` accessor. Do not reach into `g_GlobalInstance` directly from RHI code.
