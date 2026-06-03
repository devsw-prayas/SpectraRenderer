# SpectraVulkanBackend: Internal Reference

For contributors writing new `.cpp` files or extending the backend. None of this is visible to RHI code.

## Include order for a new `.cpp`

Every implementation file that needs Vulkan follows this pattern:

```cpp
#include "SpectraVulkanBackend.h"   // DLL export macros, asserts, SPEC_VK_BK_* macros
#include "VulkanFoo.h"              // the matching public header
#include "VulkanInternalHelpers.h"  // vkInit<T>, VulkanRegistry, toVkAllocationCallbacks
#include "VulkanState.h"            // g_GlobalInstance, g_IsInitialized
```

`VulkanInternalHelpers.h` and `VulkanState.h` both define `ALLOW_SYSCALL` themselves before including `SpecVulkanSyscalls.h`. You do not define it manually.

## ALLOW_SYSCALL

`SpecVulkanSyscalls.h` includes `vulkan.h` and `vk_mem_alloc.h`. It is gated:

```cpp
#ifdef ALLOW_SYSCALL
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <vk_mem_alloc.h>
#endif
```

Public headers never touch this. Internal headers (`VulkanState.h`, `VulkanInternalHelpers.h`, `VulkanDeferredRelease.h`) define it at the top before including `SpecVulkanSyscalls.h`. Implementation `.cpp` files get it transitively through those headers. Do not re-define it.

## g_GlobalInstance

Declared in `VulkanState.h`, defined in `VulkanBoostrap.cpp`.

```cpp
extern VulkanInstance g_GlobalInstance;  // Spectra::Vulkan namespace
extern bool           g_IsInitialized;
```

`VulkanInstance` is the flat bag that holds everything:

| Field | Type | What it is |
|-------|------|------------|
| `m_GlobalInstance` | `VkInstance` | The Vulkan instance |
| `m_GlobalDebugMessager` | `VkDebugUtilsMessengerEXT` | Debug messenger (debug builds only) |
| `m_AppInfo` | `VkApplicationInfo` | Stored for lifetime of instance |
| `m_DeviceProps` | `DeviceProperties` | All queried physical device capability structs |
| `m_Device` | `Utils::PhysicalDevice` | Selected physical device handle |
| `m_LogicalDevice` | `LogicalDevice` | VkDevice + three VkQueue handles |
| `m_Allocator` | `VulkanAllocator` | VmaAllocator wrapper |
| `m_CommandPools` | `CommandPools` | One VkCommandPool per unique queue family |
| `m_GraphicsFamily` | `uint32_t` | Queue family index |
| `m_ComputeFamily` | `uint32_t` | Aliases graphics if no dedicated family |
| `m_TransferFamily` | `uint32_t` | Aliases graphics if no dedicated family |
| `m_EnabledExtensions` | `VkExtension[27]` | Extensions that passed device validation |
| `m_EnableLayers` | `VkLayer[1]` | Enabled validation layers |

Most `.cpp` files only need the device:

```cpp
VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
```

## vkInit\<T\>

Zero-inits a Vulkan struct and sets `sType` + `pNext = nullptr`. All Vulkan struct construction goes through this. Never set `sType` by hand.

```cpp
auto info = Internal::vkInit<VkFenceCreateInfo>();
// info.sType == VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
// info.pNext == nullptr
// all other fields == 0
```

Specializations live in `VulkanInternalHelpers.h`. The primary template static-asserts if you call it with an unsupported type:

```cpp
template<typename T>
SPEC_VK_BK_FORCEINLINE T vkInit() {
    SPEC_VK_BK_STATIC_ASSERT(sizeof(T) == 0, "vkInit not supported for this type");
    return {};
}
```

### Adding a new specialization

Add to the matching section block in `VulkanInternalHelpers.h`:

```cpp
template<> SPEC_VK_BK_FORCEINLINE VkSomeNewStruct vkInit<VkSomeNewStruct>() {
    VkSomeNewStruct info{};
    info.sType = VK_STRUCTURE_TYPE_SOME_NEW_STRUCT;
    info.pNext = nullptr;
    return info;
}
```

Current count: ~59 specializations across bootstrap, sync, resources, pipelines, RT, and queries.

### pNext chaining

Set `pNext` after `vkInit`, not inside the specialization. The specialization always clears it:

```cpp
auto typeInfo   = Internal::vkInit<VkSemaphoreTypeCreateInfo>();
auto exportInfo = Internal::vkInit<VkExportSemaphoreCreateInfo>();
auto createInfo = Internal::vkInit<VkSemaphoreCreateInfo>();

typeInfo.semaphoreType  = VK_SEMAPHORE_TYPE_TIMELINE;
typeInfo.initialValue   = 0;
exportInfo.handleTypes  = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;

createInfo.pNext = &typeInfo;
typeInfo.pNext   = &exportInfo;   // only when CUDA export needed
```

Chains are always stack-local. Never store pointers into them past the Vulkan call.

## toVkAllocationCallbacks

Converts the public `AllocationCallbacksDesc` into a `VkAllocationCallbacks`. All `reinterpret_cast` for PFN translation is centralised here. Never cast PFN types directly in `.cpp` files.

```cpp
VkAllocationCallbacks alloc = Internal::toVkAllocationCallbacks(r_Alloc);
const VkAllocationCallbacks* pA = r_Alloc.m_pfnAllocation ? &alloc : nullptr;
// pA is what you pass to vkCreate* / vkDestroy* calls
```

Zero-init `AllocationCallbacksDesc{}` means `m_pfnAllocation == nullptr`, so `pA` becomes `nullptr` and Vulkan uses its default allocator. This is always safe.

## VulkanRegistry

Holds the master extension and layer lists. Populated once in `VulkanRegistry::initRegistry()` (called from `initializeVulkan`).

```cpp
class VulkanRegistry final {
    static std::vector<Utils::VkExtension> s_Extensions;  // 27 entries
    static std::vector<Utils::VkLayer>     s_Layers;       // 1 entry

    static constexpr int INSTANCE_EXTENSIONS  = 4;
    static constexpr int DEVICE_EXTENSIONS    = 16;
    static constexpr int RT_EXTENSIONS        = 4;
    static constexpr int OPTIONAL_EXTENSIONS  = 3;

    static const Utils::VkExtension* find(Utils::VulkanExtensions name);
};
```

Extensions are indexed by `VulkanExtensions` enum (defined in `VulkanConstants.h`). `find()` is used during device selection to check whether a given extension was actually enabled.

Layout of `s_Extensions` by index range:

| Range | Content |
|-------|---------|
| `[0, INSTANCE_EXTENSIONS)` | Instance extensions (surface, win32 surface, properties2, debug utils) |
| `[INSTANCE_EXTENSIONS, INSTANCE_EXTENSIONS + DEVICE_EXTENSIONS)` | Required device extensions |
| `[..., ... + RT_EXTENSIONS)` | Optional RT extensions |
| `[..., ... + OPTIONAL_EXTENSIONS)` | Optional misc extensions |

## DeferReleaseQueue

Declared in `VulkanDeferredRelease.h`, defined in `VulkanDeferredRelease.cpp`. Global: `Internal::g_DeferRelease`.

```
Frame N:    deferFence(f) / deferSemaphore(s)  ->  pushed into slot [N % 2]
Frame N+1:  advance()  ->  rotates to slot [(N+1) % 2], destroys its old contents, resets it
Frame N+2:  advance()  ->  rotates back to slot [N % 2], destroys what was deferred at frame N
```

Resources deferred at frame N are destroyed after exactly `FRAMES_IN_FLIGHT` (2) frames. GPU cannot reference them by then assuming frames-in-flight is respected.

### Adding a new resource type

1. Add a fixed array + count to `FrameSlot` in `VulkanDeferredRelease.h`:

```cpp
static constexpr uint32_t MAX_IMAGE_VIEWS = 128;

struct FrameSlot {
    // ... existing fields ...
    Utils::ImageViewHandle m_ImageViews[MAX_IMAGE_VIEWS];
    uint32_t               m_ImageViewCount;
};
```

2. Update `FrameSlot::reset()` to zero the new count.

3. Add a `deferImageView` method and implement it in `VulkanDeferredRelease.cpp` following the same pattern as `deferFence`.

4. Add the destroy call in `advance()`:

```cpp
for (uint32_t i = 0; i < slot.m_ImageViewCount; ++i)
    vkDestroyImageView(dev, static_cast<VkImageView>(slot.m_ImageViews[i].m_Handle), nullptr);
```

## Dynamic extension function loading

Extensions not in the Vulkan core (or promoted to 1.3) need their function pointers loaded at runtime via `vkGetDeviceProcAddr`. Load on demand inside the method that uses them, no global PFN table yet.

```cpp
VkDevice dev = g_GlobalInstance.m_LogicalDevice.m_Device;
auto fn = reinterpret_cast<PFN_vkGetSemaphoreWin32HandleKHR>(
    vkGetDeviceProcAddr(dev, "vkGetSemaphoreWin32HandleKHR"));
SPEC_VK_BK_ASSERT(fn != nullptr);
fn(dev, &info, &handle);
```

Current uses: `vkSetDebugUtilsObjectNameEXT` (bootstrap), `vkGetSemaphoreWin32HandleKHR` (sync).

If a function pointer is called on a hot path it should be cached. Add it to `VulkanInstance` or a dedicated function table struct when that becomes necessary.

## Handle casting rules

Public handles are `void* m_Handle` (dispatchable) or `void* m_Handle` (non-dispatchable). Casting to the Vulkan type:

| Handle kind | Cast to use | Example |
|-------------|-------------|---------|
| Single `void* m_Handle` | `static_cast<VkFoo>(h.m_Handle)` | `VkFence`, `VkSemaphore`, `VkImage`, `VkImageView` |
| Array of handles for batch call | `reinterpret_cast<const VkFoo*>(p_Handles)` | `vkWaitForFences`, `vkResetFences` |
| `BufferHandle.m_Handle` | `static_cast<VkBuffer>(h.m_Handle)` | `m_Allocation` is `VmaAllocation` |
| `ImageHandle.m_Handle` | `static_cast<VkImage>(h.m_Handle)` | `m_Allocation` is `VmaAllocation` |

The batch `reinterpret_cast` is safe because `FenceHandle` / `SemaphoreHandle` are single-`void*` structs with no padding before `m_Handle`, so their in-memory layout matches an array of `VkFence` / `VkSemaphore`.
