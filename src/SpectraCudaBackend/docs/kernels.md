# Kernel Execution: Modules, Launch, Linker

**Headers:** `Public/CudaModule.h`, `Public/CudaCompute.h`, `Public/CudaLinker.h`

All classes are in `Spectra::Cuda::Modules` or `Spectra::Cuda::Compute`.

## DeviceModules: loading PTX / cubin

```cpp
#include "CudaModule.h"
using namespace Spectra::Cuda::Modules;
using namespace Spectra::Cuda::Utils;

// Load from file path (cubin or PTX)
GpuModule mod = DeviceModules::loadModule("path/to/kernel.cubin");

// Load from in-memory image (e.g. embedded at compile time)
extern const unsigned char g_KernelCubin[];
GpuModule mod = DeviceModules::loadModuleData(g_KernelCubin);

// Load with JIT options (PTX -> machine code at runtime)
JitOptions opts{};
initJitOptions(opts);
setJitOptimization(opts, JitOptimizationLevel::O3, /*debugInfo=*/false, /*lineInfo=*/false);
setJitHardwareOptions(opts, JitTarget::TARGET_SM_86, JitCacheMode::CA, /*maxRegisters=*/64);
GpuModule mod = DeviceModules::loadModuleDataEx(g_PtxData, opts);

// Get a function handle by name (must match __global__ function name in PTX)
GpuFunction fn = DeviceModules::getFunction(mod, "myKernel");

// Get a global variable (e.g. __device__ float g_Scale)
GlobalMemorySegment global = DeviceModules::getGlobal(mod, "g_Scale");
// global.m_Address: device pointer to the variable
// global.m_SizeBytes: size of the variable

// Unload
DeviceModules::unloadModule(mod);
```

## DeviceCompute: kernel configuration and launch

```cpp
#include "CudaCompute.h"
using namespace Spectra::Cuda::Compute;
using namespace Spectra::Cuda::Utils;

// Query function attributes
int maxThreads = DeviceCompute::getFunctionAttribute(fn, FunctionAttribute::MAX_THREADS_PER_BLOCK);
int numRegs    = DeviceCompute::getFunctionAttribute(fn, FunctionAttribute::NUM_REGS);

// Override max dynamic shared memory (if kernel requests more than the default 48KB)
DeviceCompute::setFunctionAttribute(fn, FunctionAttribute::MAX_DYNAMIC_SHARED_SIZE_BYTES, 96 * 1024);

// Cache config
DeviceCompute::setFunctionCacheConfig(fn, FunctionCacheConfig::PREFER_L1);

// Occupancy: find the launch config that maximises active warps
OccupancyMaxBlockSizeResult occ = DeviceCompute::calculateMaxPotentialBlockSize(
    fn, /*dynamicSMem=*/0, /*blockSizeLimit=*/0);
// occ.m_MinGridSize: minimum grid size for full occupancy
// occ.m_BlockSize:   optimal block size

int activeBlocks = DeviceCompute::calculateActiveBlocksPerMultiprocessor(
    fn, occ.m_BlockSize, /*dynamicSMem=*/0);
```

### Kernel launch (raw param array)

```cpp
uint32_t N = 1024 * 1024;
float scale = 2.0f;

void* params[] = { &deviceBuf, &N, &scale };

DeviceCompute::launchKernel(
    fn,
    LaunchDimension{ (N + 255) / 256 },   // grid: ceil(N/256) x 1 x 1
    LaunchDimension{ 256 },               // block: 256 x 1 x 1
    /*sharedMemBytes=*/0,
    stream,
    params);
```

### Kernel launch (variadic helper, no param array needed)

```cpp
DeviceCompute::launchKernelVariadic(
    fn,
    LaunchDimension{ (N + 255) / 256 },
    LaunchDimension{ 256 },
    /*sharedMemBytes=*/0,
    stream,
    deviceBuf, N, scale);   // arguments forwarded directly
```

### Cooperative kernel (grid synchronization)

```cpp
DeviceCompute::launchCooperativeKernel(
    fn,
    LaunchDimension{ activeBlocks },
    LaunchDimension{ occ.m_BlockSize },
    /*sharedMemBytes=*/0,
    stream,
    params);
// Requires COMPUTE_CAPABILITY_MAJOR >= 6 and cooperative launch support
```

## DeviceLinker: link multiple PTX/cubin objects at runtime

Use when you have separate PTX files that call each other (e.g. a kernel PTX that calls into a device library).

```cpp
#include "CudaLinker.h"
using namespace Spectra::Cuda::Modules;

JitOptions opts{};
initJitOptions(opts);
setJitOptimization(opts, JitOptimizationLevel::O3, false, false);

GpuLinkState state = DeviceLinker::createLinkState(opts);

// Add kernel PTX
DeviceLinker::addData(state, JitInputType::PTX, ptxData, ptxSize, "kernel.ptx");

// Add device library cubin
DeviceLinker::addData(state, JitInputType::LIBRARY, libData, libSize, "devicelib.a");

// Link and load into a module
GpuModule linked = DeviceLinker::completeAndLoad(state);

// Clean up link state (module stays valid until unloadModule)
DeviceLinker::destroyLinkState(state);

GpuFunction fn = DeviceModules::getFunction(linked, "myKernel");
```

`JitInputType` values:

| Type | Input |
|------|-------|
| `CUBIN` | Compiled SM-specific binary |
| `PTX` | Parallel Thread Execution assembly |
| `FATBIN` | Fat binary containing multiple SM targets |
| `OBJECT` | Relocatable device object (.o) |
| `LIBRARY` | Device static library (.a) |
