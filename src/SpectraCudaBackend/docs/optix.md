# OptiX: Context, Pipeline, Acceleration Structures, Launch

**Headers:** `Public/OptixContextManager.h`, `Public/OptixPipeline.h`, `Public/OptixAccel.h`, `Public/OptixLaunch.h`  
**Namespace:** `Spectra::Cuda::Optix`

OptiX runs on top of CUDA. You need a valid `CudaContext` before calling any OptiX function. All classes are static-method collections.

## DeviceOptixContext: init and context

```cpp
#include "OptixContextManager.h"
using namespace Spectra::Cuda::Optix;
using namespace Spectra::Cuda::Utils;

// Load OptiX function table (call once after initCuda / context creation)
bool ok = DeviceOptixContext::initOptix();

// Create an OptiX context tied to the current CUDA context
OptixContextOptions ctxOpts{};
ctxOpts.logCallbackFunction = myLogCallback;  // optional
ctxOpts.logCallbackLevel    = 4;              // 0=none 1=fatal 2=error 3=warn 4=info

GpuOptixContext optixCtx = DeviceOptixContext::createContext(cudaCtx, ctxOpts);

// Tune disk cache sizes (bytes)
DeviceOptixContext::setCacheEnabled(optixCtx, 1);
DeviceOptixContext::setCacheDatabaseSizes(optixCtx,
    /*lowWatermark=*/  512 * 1024 * 1024,
    /*highWatermark=*/ 1024 * 1024 * 1024);

// Shutdown
DeviceOptixContext::destroyContext(optixCtx);
```

## DeviceOptixPipeline: module compilation, program groups, pipeline

### Step 1: compile module from PTX

```cpp
#include "OptixPipeline.h"

OptixModuleCompileOptions modOpts{};
modOpts.maxRegisterCount = 0;           // 0 = no limit
modOpts.optLevel         = OPTIX_COMPILE_OPTIMIZATION_LEVEL_3;
modOpts.debugLevel       = OPTIX_COMPILE_DEBUG_LEVEL_NONE;

OptixPipelineCompileOptions pipeOpts{};
pipeOpts.usesMotionBlur                   = 0;
pipeOpts.traversableGraphFlags            = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_GAS;
pipeOpts.numPayloadValues                 = 2;
pipeOpts.numAttributeValues               = 2;
pipeOpts.pipelineLaunchParamsVariableName = "g_Params";

GpuOptixModule mod = DeviceOptixPipeline::createModule(
    optixCtx, modOpts, pipeOpts, ptxSource);
```

### Step 2: create program groups

```cpp
// Raygen group
OptixProgramGroupDesc rgDesc{};
rgDesc.kind                     = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;
rgDesc.raygen.module            = static_cast<OptixModule>(mod.m_Handle);
rgDesc.raygen.entryFunctionName = "__raygen__main";

// Miss group
OptixProgramGroupDesc missDesc{};
missDesc.kind                    = OPTIX_PROGRAM_GROUP_KIND_MISS;
missDesc.miss.module             = static_cast<OptixModule>(mod.m_Handle);
missDesc.miss.entryFunctionName  = "__miss__radiance";

// Hit group (closest hit + optional any hit)
OptixProgramGroupDesc hitDesc{};
hitDesc.kind                         = OPTIX_PROGRAM_GROUP_KIND_HITGROUP;
hitDesc.hitgroup.moduleCH            = static_cast<OptixModule>(mod.m_Handle);
hitDesc.hitgroup.entryFunctionNameCH = "__closesthit__radiance";

OptixProgramGroupDesc descs[3] = { rgDesc, missDesc, hitDesc };
GpuOptixProgramGroup groups[3];

DeviceOptixPipeline::createProgramGroups(optixCtx, descs, 3, groups);
```

### Step 3: link pipeline

```cpp
OptixPipelineLinkOptions linkOpts{};
linkOpts.maxTraceDepth = 2;

GpuOptixPipeline pipeline = DeviceOptixPipeline::createPipeline(
    optixCtx, pipeOpts, linkOpts, groups, 3);

// Configure stack sizes (query each group's needs first)
OptixStackSizes rgStack   = DeviceOptixPipeline::getProgramGroupStackSize(groups[0], pipeline);
OptixStackSizes missStack = DeviceOptixPipeline::getProgramGroupStackSize(groups[1], pipeline);
OptixStackSizes hitStack  = DeviceOptixPipeline::getProgramGroupStackSize(groups[2], pipeline);

// Calculate and set (simplified; use OptiX utility functions for exact computation)
DeviceOptixPipeline::setPipelineStackSize(pipeline,
    /*directCallableFromTraversal=*/0,
    /*directCallableFromState=*/0,
    /*continuationStack=*/2048,
    /*maxTraversableDepth=*/1);
```

## DeviceOptixAccel: acceleration structures (BVH)

```cpp
#include "OptixAccel.h"

OptixAccelBuildOptions buildOpts{};
buildOpts.buildFlags = OPTIX_BUILD_FLAG_ALLOW_COMPACTION | OPTIX_BUILD_FLAG_PREFER_FAST_TRACE;
buildOpts.operation  = OPTIX_BUILD_OPERATION_BUILD;

// Describe triangle geometry
OptixBuildInputDesc input{};
input.type = OPTIX_BUILD_INPUT_TYPE_TRIANGLES;
// ... fill triangleArray fields (vertex / index buffers, format, count) ...

// Query memory requirements
OptixAccelBufferSizes sizes = DeviceOptixAccel::computeMemoryUsage(
    optixCtx, buildOpts, &input, 1);
// sizes.outputSizeInBytes, sizes.tempSizeInBytes, sizes.tempUpdateSizeInBytes

// Allocate buffers
GpuAddress tempBuf   = DeviceMemory::deviceAlloc(sizes.tempSizeInBytes);
GpuAddress outputBuf = DeviceMemory::deviceAlloc(sizes.outputSizeInBytes);

// Property buffer for compacted size query
GpuAddress compactedSizeProp = DeviceMemory::deviceAlloc(sizeof(uint64_t));

// Build
GpuOptixTraversableHandle handle = DeviceOptixAccel::build(
    optixCtx, stream,
    buildOpts, &input, 1,
    tempBuf.m_GpuAddr,    sizes.tempSizeInBytes,
    outputBuf.m_GpuAddr,  sizes.outputSizeInBytes,
    compactedSizeProp.m_GpuAddr);

DeviceStreams::syncStream(stream);

// Compact (read back compacted size first)
uint64_t compactedSize = 0;
PinnedMemory::copyDevToHostAsync(PinnedAddress(&compactedSize), compactedSizeProp, sizeof(uint64_t), stream);
DeviceStreams::syncStream(stream);

GpuAddress compactedBuf = DeviceMemory::deviceAlloc(compactedSize);
GpuOptixTraversableHandle compactedHandle = DeviceOptixAccel::compact(
    optixCtx, stream,
    handle,
    compactedBuf.m_GpuAddr, compactedSize);

DeviceMemory::deviceFree(outputBuf);
DeviceMemory::deviceFree(tempBuf);
```

### Relocation (after defrag / memory move)

```cpp
GpuOptixRelocationInfo info = DeviceOptixAccel::getRelocationInfo(optixCtx, compactedHandle);
GpuOptixTraversableHandle relocated = DeviceOptixAccel::relocate(
    optixCtx, stream, info, newBuf.m_GpuAddr, newBufSize);
```

## DeviceOptixLaunch: SBT setup and ray dispatch

```cpp
#include "OptixLaunch.h"

// Pack SBT record headers (one per program group)
// Allocate host-side SBT records:  header (32 bytes) + user data
struct RaygenRecord  { char header[OPTIX_SBT_RECORD_HEADER_SIZE]; /* user data */ };
struct MissRecord    { char header[OPTIX_SBT_RECORD_HEADER_SIZE]; /* user data */ };
struct HitgroupRecord{ char header[OPTIX_SBT_RECORD_HEADER_SIZE]; /* user data */ };

RaygenRecord   rgRec{};
MissRecord     missRec{};
HitgroupRecord hitRec{};

DeviceOptixLaunch::packSbtRecordHeader(groups[0], rgRec.header);
DeviceOptixLaunch::packSbtRecordHeader(groups[1], missRec.header);
DeviceOptixLaunch::packSbtRecordHeader(groups[2], hitRec.header);

// Upload to GPU (via pinned memory + async copy)
// ... copy rgRec, missRec, hitRec to device buffers ...

// Build SBT descriptor
ShaderBindingTable sbt{};
sbt.m_RaygenRecord            = rgDeviceAddr;
sbt.m_MissRecordBase          = missDeviceAddr;
sbt.m_MissRecordStrideInBytes = sizeof(MissRecord);
sbt.m_MissRecordCount         = 1;
sbt.m_HitgroupRecordBase          = hitDeviceAddr;
sbt.m_HitgroupRecordStrideInBytes = sizeof(HitgroupRecord);
sbt.m_HitgroupRecordCount         = 1;

// Dispatch
DeviceOptixLaunch::launch(
    pipeline,
    stream,
    paramsDeviceAddr,    // device address of __constant__ launch params struct
    sizeof(MyParams),
    sbt,
    /*width=*/1920, /*height=*/1080, /*depth=*/1);
```
