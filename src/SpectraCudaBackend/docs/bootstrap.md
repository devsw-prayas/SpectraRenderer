# Bootstrap: Driver Init, Device Enumeration, Context

**Headers:** `Public/CudaBootstrap.h`, `Public/CudaContextManager.h`

## CudaDriver (Spectra::Cuda::Bootstrap)

Call `initCuda()` once before anything else. It initializes the CUDA Driver API (`cuInit`).

```cpp
#include "CudaBootstrap.h"
using namespace Spectra::Cuda::Bootstrap;

bool ok = CudaDriver::initCuda();
// ok == false means no CUDA driver present

uint32_t version = CudaDriver::getCudaDriverVersion();
// e.g. 13020 for CUDA 13.2
```

## CudaDeviceManager (Spectra::Cuda::Bootstrap)

Enumerate and inspect physical GPUs.

```cpp
#include "CudaBootstrap.h"
using namespace Spectra::Cuda::Bootstrap;
using namespace Spectra::Cuda::Utils;

int count = CudaDeviceManager::getCudaDeviceCount();

for (int i = 0; i < count; ++i) {
    DeviceHandle handle = CudaDeviceManager::getCudaDevice(i);

    char name[256];
    CudaDeviceManager::getCudaDeviceName(name, sizeof(name), handle);

    size_t totalMem = CudaDeviceManager::getCudaDeviceTotalMemory(handle);

    int smMajor = 0, smMinor = 0;
    CudaDeviceManager::getCudaDeviceAttribute(&smMajor, CudaDeviceAttribute::COMPUTE_CAPABILITY_MAJOR, handle);
    CudaDeviceManager::getCudaDeviceAttribute(&smMinor, CudaDeviceAttribute::COMPUTE_CAPABILITY_MINOR, handle);

    DeviceUUID uuid = CudaDeviceManager::getCudaDeviceUUID(handle);
    // uuid.m_Lo / uuid.m_Hi for Vulkan interop matching
}
```

Key `CudaDeviceAttribute` values:

| Attribute | What it returns |
|-----------|-----------------|
| `COMPUTE_CAPABILITY_MAJOR` | SM architecture generation |
| `COMPUTE_CAPABILITY_MINOR` | SM architecture revision |
| `MAX_THREADS_PER_BLOCK` | Max block size |
| `MAX_SHARED_MEMORY_PER_BLOCK` | Shared mem per block (bytes) |
| `WARP_SIZE` | Always 32 on current hardware |
| `VIRTUAL_MEMORY_MANAGEMENT_SUPPORTED` | 1 if VMM API is available |
| `CONCURRENT_MANAGED_ACCESS` | 0 on Windows WDDM (important for managed memory use) |

## ContextManager (Spectra::Cuda::Context)

CUDA contexts are thread-bound. Create one per device; push/pop to share across threads.

```cpp
#include "CudaContextManager.h"
using namespace Spectra::Cuda::Context;
using namespace Spectra::Cuda::Utils;

DeviceHandle device = CudaDeviceManager::getCudaDevice(0);

CudaContext ctx = ContextManager::createCudaContext(
    device,
    ContextSchedulingFlags::SCHEDULE_BLOCKING_SYNC,
    ContextCreationFlags::NONE);

// Set as current on this thread
ContextManager::setCurrentCudaContext(ctx);

// CPU-side full flush (avoid in hot paths)
ContextManager::cudaContextSynchronize();

// Multi-thread sharing
ContextManager::pushCudaContext(ctx);   // push on worker thread
// ... do work ...
CudaContext popped = ContextManager::popCudaContext();

// Shutdown
ContextManager::destroyCudaContext(ctx);
```

`ContextSchedulingFlags` options:

| Flag | Behaviour |
|------|-----------|
| `SCHEDULE_AUTO` | Driver decides (default) |
| `SCHEDULE_SPIN` | CPU spins while waiting, lowest latency |
| `SCHEDULE_YIELD` | CPU yields to OS, saves power |
| `SCHEDULE_BLOCKING_SYNC` | CPU sleeps, highest latency but no wasted cycles |

Use `SCHEDULE_BLOCKING_SYNC` for background / asset loading contexts. Use `SCHEDULE_SPIN` for the primary render context if latency is critical and you have spare cores.
