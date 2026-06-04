# Acceleration Structures

**Header:** `Public/VulkanAccelStructure.h`  
**Namespace:** `Spectra::Vulkan`

All methods are static. `VulkanAccelStructure` owns the backing `VkBuffer`/`VmaAllocation` — it is stored inside `AccelerationStructureHandle` and destroyed by `destroyAccelStructure`.

## Typical BLAS build

```cpp
#include "VulkanAccelStructure.h"
#include "VulkanResources.h"
using namespace Spectra::Vulkan;
using namespace Spectra::Vulkan::Utils;

// 1. Describe the geometry
ASGeometryTriangles tri{};
tri.m_VertexFormat        = Format::R32G32B32_SFLOAT;
tri.m_VertexDeviceAddress = VulkanBuffer::getDeviceAddress(vertexBuf);
tri.m_VertexStride        = sizeof(float) * 3;
tri.m_MaxVertex           = vertexCount - 1;
tri.m_IndexType           = IndexType::UINT32;
tri.m_IndexDeviceAddress  = VulkanBuffer::getDeviceAddress(indexBuf);

uint32_t triangleCount = indexCount / 3;

ASBuildDesc buildDesc{};
buildDesc.m_Type            = AccelerationStructureType::BOTTOM_LEVEL;
buildDesc.m_Flags           = AccelerationStructureBuildFlags::PREFER_FAST_TRACE;
buildDesc.m_Geometries      = &tri;
buildDesc.m_GeometryCount   = 1;
buildDesc.m_PrimitiveCounts = &triangleCount;   // required

// 2. Query sizes
ASBuildSizes sizes = VulkanAccelStructure::getBuildSizes(buildDesc);

// 3. Allocate scratch buffer
BufferDesc scratchDesc{};
scratchDesc.m_Size  = sizes.m_ScratchSize;
scratchDesc.m_Usage = BufferUsage::STORAGE_BUFFER | BufferUsage::SHADER_DEVICE_ADDRESS;
BufferHandle scratchBuf = VulkanBuffer::createBuffer(scratchDesc, {});

// 4. Create the BLAS
AccelerationStructureHandle blas = VulkanAccelStructure::createAccelStructure(
    AccelerationStructureType::BOTTOM_LEVEL,
    AccelerationStructureBuildFlags::PREFER_FAST_TRACE,
    sizes.m_AccelStructureSize);

// 5. Record the build
buildDesc.m_Dst                  = blas;
buildDesc.m_ScratchDeviceAddress = VulkanBuffer::getDeviceAddress(scratchBuf);
VulkanAccelStructure::cmdBuild(cmd, buildDesc);

// 6. Barrier — AS read after AS write
MemoryBarrierDesc barrier{};
barrier.m_SrcStage  = PipelineStage::ACCELERATION_STRUCTURE_BUILD;
barrier.m_DstStage  = PipelineStage::ACCELERATION_STRUCTURE_BUILD;
barrier.m_SrcAccess = AccessType::ACCELERATION_STRUCTURE_WRITE;
barrier.m_DstAccess = AccessType::ACCELERATION_STRUCTURE_READ;
VulkanBarrier::cmdMemoryBarrier(cmd, barrier);

// 7. Scratch can be destroyed once the build command has finished executing.
VulkanBuffer::destroyBuffer(scratchBuf);
```

## Typical TLAS build

```cpp
// Instance buffer containing VkAccelerationStructureInstanceKHR structs.
// Fill the buffer on the CPU side before building.
ASGeometryInstances inst{};
inst.m_InstancesDeviceAddress = VulkanBuffer::getDeviceAddress(instanceBuf);
inst.m_ArrayOfPointers        = false;

uint32_t instanceCount = numInstances;

ASBuildDesc tlasBuildDesc{};
tlasBuildDesc.m_Type            = AccelerationStructureType::TOP_LEVEL;
tlasBuildDesc.m_Flags           = AccelerationStructureBuildFlags::PREFER_FAST_TRACE;
tlasBuildDesc.m_Instances       = &inst;    // set m_Instances, not m_Geometries
tlasBuildDesc.m_GeometryCount   = 1;
tlasBuildDesc.m_PrimitiveCounts = &instanceCount;

ASBuildSizes tlasSizes = VulkanAccelStructure::getBuildSizes(tlasBuildDesc);

AccelerationStructureHandle tlas = VulkanAccelStructure::createAccelStructure(
    AccelerationStructureType::TOP_LEVEL,
    AccelerationStructureBuildFlags::PREFER_FAST_TRACE,
    tlasSizes.m_AccelStructureSize);

tlasBuildDesc.m_Dst                  = tlas;
tlasBuildDesc.m_ScratchDeviceAddress = VulkanBuffer::getDeviceAddress(tlasScratchBuf);
VulkanAccelStructure::cmdBuild(cmd, tlasBuildDesc);
```

## Compaction

Compaction reduces BLAS memory by up to 50 % for static geometry. It requires a query pool of type `ACCELERATION_STRUCTURE_COMPACTED_SIZE`.

```cpp
#include "VulkanQuery.h"

// Pool must be created before the build pass
QueryPoolDesc qpDesc{};
qpDesc.m_Type  = QueryType::ACCELERATION_STRUCTURE_COMPACTED_SIZE;
qpDesc.m_Count = 1;
QueryPoolHandle compactPool = VulkanQuery::createQueryPool(qpDesc, {});

// Reset the pool before the build frame
VulkanQuery::cmdResetPool(cmd, compactPool, 0, 1);

// After cmdBuild, write the compacted size
VulkanAccelStructure::cmdWriteCompactedSize(cmd, blas, compactPool, 0);

// --- submit and wait ---

// Read back the compacted size on the CPU
uint64_t compactedSize = 0;
VulkanQuery::getResults(compactPool, 0, 1, &compactedSize, true);

// Create the compact destination
AccelerationStructureHandle compactBlas = VulkanAccelStructure::createAccelStructure(
    AccelerationStructureType::BOTTOM_LEVEL,
    AccelerationStructureBuildFlags::NONE,
    compactedSize);

// Record the compact copy
VulkanAccelStructure::cmdCompact(cmd, blas, compactBlas);

// --- submit and wait ---

// Destroy the original; keep compactBlas
VulkanAccelStructure::destroyAccelStructure(blas);

VulkanQuery::destroyQueryPool(compactPool, {});
```

## `ASBuildDesc` fields

| Field | Type | Default | Notes |
|-------|------|---------|-------|
| `m_Type` | `AccelerationStructureType` | `BOTTOM_LEVEL` | |
| `m_Flags` | `AccelerationStructureBuildFlags` | `NONE` | Bitwise-OR |
| `m_Geometries` | `const ASGeometryTriangles*` | `nullptr` | BLAS path — one per geometry |
| `m_Instances` | `const ASGeometryInstances*` | `nullptr` | TLAS path — overrides `m_Geometries` |
| `m_GeometryCount` | `uint32_t` | `0` | Count for whichever path is used |
| `m_PrimitiveCounts` | `const uint32_t*` | `nullptr` | **Required.** Triangles or instance count per geometry |
| `m_GeometryFlags` | `const GeometryFlags*` | `nullptr` | Per-geometry flags; `nullptr` = all `OPAQUE` |
| `m_Dst` | `AccelerationStructureHandle` | `{}` | Must be valid when calling `cmdBuild` |
| `m_ScratchDeviceAddress` | `uint64_t` | `0` | Device address of a scratch buffer >= `ASBuildSizes::m_ScratchSize` |

## `AccelerationStructureHandle` layout

```
m_Handle        — VkAccelerationStructureKHR
m_BackingBuffer — VkBuffer  (owned; ACCELERATION_STRUCTURE_STORAGE | SHADER_DEVICE_ADDRESS)
m_BackingAlloc  — VmaAllocation
```

`destroyAccelStructure` frees all three. Do not pass the backing buffer to `VulkanBuffer::destroyBuffer`.

## Notes

- `MAX_GEOMETRIES = 32` — maximum geometry count per build call.
- All extension function pointers are loaded once per method via a static local `vkGetDeviceProcAddr` call.
- The scratch buffer is not managed by this class; destroy it once the build GPU work has completed.
- For per-primitive build range offsets (e.g. interleaved mesh data), build manually using Vulkan directly — `cmdBuild` always sets `primitiveOffset = 0` and `firstVertex = 0`.
