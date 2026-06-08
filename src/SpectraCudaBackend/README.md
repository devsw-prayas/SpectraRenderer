# SpectraCudaBackend: Usage Reference

Thin static-class API over the CUDA Driver API and OptiX. No runtime state owned by callers. `cuda.h` and `optix.h` are never exposed through public headers.

All types live in `Spectra::Cuda::Utils` unless otherwise noted. Headers include that namespace with `using namespace Utils` so you only need to qualify when there is ambiguity.

## Sections

| # | Topic | Namespace |
|---|-------|-----------|
| 1 | [Bootstrap: driver init, device enumeration, context](docs/bootstrap.md) | `Bootstrap`, `Context` |
| 2 | [Memory: device, pinned, managed, virtual](docs/memory.md) | `Memory` |
| 3 | [Streams and events](docs/streams-events.md) | `Streams`, `Events` |
| 4 | [Kernel execution: modules, launch, linker](docs/kernels.md) | `Modules`, `Compute` |
| 5 | [CUDA Graphs](docs/graphs.md) | `Graphs` |
| 6 | [Arrays, textures and surfaces](docs/textures.md) | `Arrays` |
| 7 | [OptiX: context, pipeline, acceleration structures, launch](docs/optix.md) | `Optix` |

## Design rules

- All handles (`GpuStream`, `GpuEvent`, `GpuModule`, etc.) are `void*` or integer wrappers. Never store or compare raw CUDA handles; go through the wrapper methods.
- Destroy functions zero the handle after calling the CUDA destructor. Always pass by reference so the handle is invalidated at the call site.
- `DeviceHandle::makeCpu()` returns a CPU-side location handle for managed memory and virtual memory access descriptors.
- `JitOptions`, `Array3dDesc`, `ResourceDesc`, `TextureDesc`, `AllocDesc`, `AccessDesc`, `MemCpy3DDesc` all have `init*` free functions that zero-fill them. Always call the matching `init*` before setting fields.
- The validate free functions (`validateArray3dDesc`, `validateMemCpy3DDesc`, etc.) assert in debug builds. Call them before passing descs into the API if building incrementally.
