# Memory: Device, Pinned, Managed, Virtual

**Headers:** `Public/CudaDeviceMemory.h`, `Public/CudaPinnedMemory.h`, `Public/CudaManagedMemory.h`, `Public/CudaVirtualMemory.h`

All classes are in `Spectra::Cuda::Memory`.

## DeviceMemory

Standard on-device allocations. Fast; not directly CPU-accessible.

```cpp
#include "CudaDeviceMemory.h"
using namespace Spectra::Cuda::Memory;

// Allocate
GpuAddress buf = DeviceMemory::deviceAlloc(1024 * sizeof(float));

// Pitched allocation (for 2D operations, hardware-aligned rows)
PitchedAllocation pitch = DeviceMemory::deviceAllocPitch(
    /*widthInBytes=*/256 * sizeof(float),
    /*height=*/256,
    /*elemSizeBytes=*/sizeof(float));
// pitch.m_Address: base GPU address
// pitch.m_Pitch:   actual row stride in bytes (>= widthInBytes, hardware-aligned)

// Query free/total
GpuMemory mem = DeviceMemory::query();
// mem.m_TotalMemory, mem.m_AvailableMemory

// Synchronous memset
DeviceMemory::memsetD32(buf, 0, 1024);  // 1024 x 4-byte elements set to 0

// Async memset (preferred in render loop)
DeviceMemory::memsetD32Async(buf, 0, 1024, stream);

// Device-to-device copy
DeviceMemory::copyDeviceToDevice(dst, src, 1024 * sizeof(float));
DeviceMemory::copyDeviceToDeviceAsync(dst, src, 1024 * sizeof(float), stream);

// Free (zeroes the address)
DeviceMemory::deviceFree(buf);
```

## PinnedMemory

Page-locked host memory. Required for async DMA transfers between CPU and GPU.

```cpp
#include "CudaPinnedMemory.h"
using namespace Spectra::Cuda::Memory;

// Allocate page-locked host buffer
PinnedAddress hostBuf = PinnedMemory::pinnedAlloc(1024 * sizeof(float));

// With flags (e.g. write-combined for upload-only buffers)
uint32_t flags = CudaHelpers::computeAllocFlag({ HostAllocFlags::ALLOC_WRITE_COMBINED });
PinnedAddress wcBuf = PinnedMemory::pinnedAlloc(1024 * sizeof(float), flags);

// Async upload (host -> device)
PinnedMemory::copyHostToDevAsync(deviceBuf, hostBuf, 1024 * sizeof(float), stream);

// Async readback (device -> host)
PinnedMemory::copyDevToHostAsync(hostBuf, deviceBuf, 1024 * sizeof(float), stream);

// Register existing (non-pinned) host memory for DMA
PinnedMemory::hostMemRegister(existingBuf, size, flags);
PinnedMemory::hostMemUnRegister(existingBuf);

// Get a device-accessible address for a pinned buffer (for kernels that write back to host)
GpuAddress devMappedAddr;
PinnedMemory::mapToDevice(devMappedAddr, hostBuf, 0);

// Free
PinnedMemory::pinnedFree(hostBuf);
```

## ManagedMemory

Unified memory accessible from both CPU and GPU. Automatically migrates pages; use hints to avoid thrashing.

Note: `CONCURRENT_MANAGED_ACCESS` is 0 on Windows WDDM. The CPU and GPU cannot access managed memory simultaneously on Windows.

```cpp
#include "CudaManagedMemory.h"
using namespace Spectra::Cuda::Memory;

// Allocate (flags = 0 for attached-globally, 1 for attached-host)
GpuAddress managed = ManagedMemory::allocManaged(1024 * sizeof(float), /*flags=*/0);

DeviceHandle gpu = CudaDeviceManager::getCudaDevice(0);

// Hint: this data is read by both CPU and GPU
ManagedMemory::adviseMemory(managed, 1024 * sizeof(float), MemoryAdvise::SET_READ_MOSTLY, gpu);

// Prefetch to GPU before kernel launch (avoids page-fault stalls)
ManagedMemory::prefetchAsync(managed, 1024 * sizeof(float), gpu, stream);

// Prefetch back to CPU before CPU-side read
DeviceHandle cpu = DeviceHandle::makeCpu();
ManagedMemory::prefetchAsync(managed, 1024 * sizeof(float), cpu, stream);

// Free via DeviceMemory::deviceFree (managed memory uses the same GpuAddress type)
DeviceMemory::deviceFree(managed);
```

`MemoryAdvise` values:

| Value | Effect |
|-------|--------|
| `SET_READ_MOSTLY` | Mark as read-mostly; driver duplicates pages |
| `UNSET_READ_MOSTLY` | Clear read-mostly hint |
| `SET_PREFERRED_LOCATION` | Prefer physical residency on given device |
| `UNSET_PREFERRED_LOCATION` | Clear location preference |
| `SET_ACCESSED_BY` | Allow device to map pages without migration |
| `UNSET_ACCESSED_BY` | Clear accessed-by mapping |

## VirtualMemory

Explicit virtual memory management (VMM). Reserve VA, create physical allocations, map them in. Useful for resizable buffers and multi-GPU memory pooling.

```cpp
#include "CudaVirtualMemory.h"
using namespace Spectra::Cuda::Memory;

// Query required allocation granularity
AllocDesc allocDesc{};
initAllocDesc(allocDesc);
setAllocationType(allocDesc, AllocationType::PINNED);
setAllocationHandleType(allocDesc, AllocationHandleType::NONE);

DeviceHandle gpu = CudaDeviceManager::getCudaDevice(0);
setLocation(allocDesc, gpu);

size_t granularity = VirtualMemory::getAllocationGranularity(
    allocDesc, AllocationGranularityOption::RECOMMENDED);

size_t size = granularity;  // must be a multiple of granularity

// 1. Reserve a virtual address range
GpuAddress va = VirtualMemory::reserveAddress(size, granularity);

// 2. Create a physical allocation
AllocHandle phys = VirtualMemory::createAllocation(size, allocDesc);

// 3. Map physical into VA
VirtualMemory::map(va, size, /*offset=*/0, phys);

// 4. Set access permissions
AccessDesc access{};
initAccessDesc(access);
setAccessLocation(access, gpu);
uint64_t accessFlags = CudaHelpers::computeAccessFlags({ AccessFlagBits::READWRITE });
setAccessFlags(access, accessFlags);
VirtualMemory::setAccess(va, size, &access, 1);

// Use va like a normal GpuAddress in kernel launches and copies

// Teardown (reverse order)
VirtualMemory::unmap(va, size);
VirtualMemory::releaseAllocation(phys);
VirtualMemory::freeAddress(va, size);
```

### Win32 export (for Vulkan interop)

```cpp
HANDLE win32Handle;
VirtualMemory::exportAllocation(&win32Handle, phys, AllocationHandleType::WIN32_HANDLE);
// Import into Vulkan via VkImportMemoryWin32HandleInfoKHR
```
