# SpectraVulkanBackend: Usage Reference

Thin public API over Vulkan. All classes are static method collections, no instances, no state owned by callers. `vulkan.h` is never exposed through public headers.

## Sections

| # | Topic | Status |
|---|-------|--------|
| - | [Internal reference: vkInit, handles, registry, deferred release](docs/internals.md) | |
| 1 | [Bootstrap & Teardown](docs/bootstrap.md) | Done |
| 2 | [Synchronisation: Fences, Timeline Semaphores, Barriers](docs/sync.md) | Done |
| 3 | [Resources: Buffers, Images, Views, Samplers](docs/resources.md) | Done |
| 4 | [External Memory: Win32 Handle Export](docs/external-memory.md) | Done |
| 5 | [Descriptors & Binding](docs/descriptors.md) | Done |
| 6 | [Pipelines & Shaders](docs/pipelines.md) | Done |
| 7 | [Command Recording](docs/commands.md) | Done |
| 8 | [Queue Submission](docs/queue.md) | Done |
| 9 | Swapchain & Presentation | Pending |
| 10 | Acceleration Structures | Pending |
| 11 | Profiling & Timestamp Queries | Pending |

## Design rules that matter when writing the RHI

- All resource handles are opaque `void* m_Handle` structs. Never store or compare raw `VkHandle` values; go through the wrapper methods.
- `AllocationCallbacksDesc{}` (zero-init) is always valid. Pass it everywhere you don't have a custom allocator; the impls null-check before forwarding.
- `ALLOW_SYSCALL` is an internal guard. Never define it in RHI code. Everything you need is behind the public headers.
- `DeferReleaseQueue::g_DeferRelease` is the safe destruction path for in-flight resources. Call `advance()` once per frame before recording new work.
- `VulkanBarrier` uses `synchronization2` exclusively. Never call `vkCmdPipelineBarrier` directly from RHI code.
