# Queries

**Header:** `Public/VulkanQuery.h`  
**Namespace:** `Spectra::Vulkan`

All methods are static. `VulkanQuery` covers timestamp queries and serves as the pool factory for the compacted-size queries used in acceleration structure compaction.

## Timestamp queries

Used to measure GPU execution time between two pipeline stages. Timestamp period (nanoseconds per tick) is in `VkPhysicalDeviceLimits::timestampPeriod` — query it from the physical device properties if you need real-time values.

```cpp
#include "VulkanQuery.h"
using namespace Spectra::Vulkan;
using namespace Spectra::Vulkan::Utils;

// Create a pool with 2 slots (start + end of a pass)
QueryPoolDesc desc{};
desc.m_Type  = QueryType::TIMESTAMP;
desc.m_Count = 2;
QueryPoolHandle pool = VulkanQuery::createQueryPool(desc, {});

// --- each frame ---

// Reset before use (must be done on the GPU timeline)
VulkanQuery::cmdResetPool(cmd, pool, 0, 2);

// Write start timestamp at the top of the compute pass
VulkanQuery::cmdWriteTimestamp(cmd, pool, 0, PipelineStage::TOP_OF_PIPE);

// ... dispatch work ...

// Write end timestamp at the bottom of the compute pass
VulkanQuery::cmdWriteTimestamp(cmd, pool, 1, PipelineStage::BOTTOM_OF_PIPE);

// --- after submit + wait ---

// Read both 64-bit results; block until available
uint64_t timestamps[2];
VulkanQuery::getResults(pool, 0, 2, timestamps, true);

// Elapsed ticks (multiply by timestampPeriod for nanoseconds)
uint64_t elapsedTicks = timestamps[1] - timestamps[0];

// --- cleanup ---
VulkanQuery::destroyQueryPool(pool, {});
```

## Pipeline statistics queries

```cpp
QueryPoolDesc desc{};
desc.m_Type  = QueryType::PIPELINE_STATISTICS;
desc.m_Count = 1;
QueryPoolHandle statsPool = VulkanQuery::createQueryPool(desc, {});
```

Pipeline statistics queries require `VkQueryPoolCreateInfo::pipelineStatistics` to be set, which maps to the Vulkan struct directly. The current `createQueryPool` leaves `pipelineStatistics = 0` (no statistics bits enabled). Use the raw Vulkan API if fine-grained statistics selection is needed — this wrapper is primarily intended for timestamps and AS compaction.

## Acceleration structure compacted-size queries

See [accel-structure.md](accel-structure.md) for the full compaction workflow. Pool creation:

```cpp
QueryPoolDesc qpDesc{};
qpDesc.m_Type  = QueryType::ACCELERATION_STRUCTURE_COMPACTED_SIZE;
qpDesc.m_Count = numBlas;  // one slot per BLAS being compacted
QueryPoolHandle compactPool = VulkanQuery::createQueryPool(qpDesc, {});
```

## `QueryPoolDesc` fields

| Field | Type | Default | Notes |
|-------|------|---------|-------|
| `m_Type` | `QueryType` | `TIMESTAMP` | See `QueryType` enum |
| `m_Count` | `uint32_t` | `0` | Number of query slots in the pool |

## `QueryType` values

| Enumerator | Vulkan value | Use |
|------------|-------------|-----|
| `PIPELINE_STATISTICS` | 1 | Pipeline invocation counters |
| `TIMESTAMP` | 2 | GPU clock measurements |
| `ACCELERATION_STRUCTURE_COMPACTED_SIZE` | 1000150000 | AS compaction size query |

## Notes

- `cmdWriteTimestamp` uses `vkCmdWriteTimestamp2` from the synchronization2 extension — the device must have been initialised with synchronization2 enabled (it always is in this backend).
- `getResults` always uses `VK_QUERY_RESULT_64_BIT`. Pass `v_Wait = false` to return immediately; check for `VK_NOT_READY` by inspecting results (not done here — caller responsibility).
- Query pools must be reset on the GPU (`cmdResetPool`) before each use. CPU-side reset is not supported.
