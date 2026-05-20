# SpectraRenderer

An unbiased, GPU-accelerated spectral path tracer built from scratch.  
Developed by [Prayas Bharadwaj](https://www.linkedin.com/in/prayas-bharadwaj-053886323/)

---

## What it is

Spectra operates entirely in the wavelength domain — no RGB approximations. Every system above the syscall and driver boundary is implemented in-house. No external runtime frameworks. The goal is brute-force physical correctness in spectral light transport, with no compromises on determinism or performance.

The default compute backend is CUDA/OptiX (Ampere, SM 80–86). Vulkan is the graphics layer.

---

## Architecture

```
common/          Standalone infrastructure libraries (git submodules)
src/             Engine and application modules
```

### Infrastructure (`common/`)

| Library | Purpose |
|---|---|
| **StormSTL** | Custom STL replacement — allocator-aware, SIMD-optimized containers |
| **Corium** | NUMA-aware multithreading, task scheduling, CUDA execution runtime |
| **Stratum** | Deterministic instrumentation and profiling |
| **Kerbecs** | Memory and thread sanitation with selective shadowing |
| **Leibniz** | SIMD-optimized math and numerical computation |
| **Hades-Benchmark** | Performance benchmarking framework |

Each library is self-contained and can be used independently outside of Spectra.

### Engine (`src/`)

| Module | Purpose |
|---|---|
| **SpectraCore** | Core engine types and interfaces |
| **SpectraRenderPipeline** | Rendering pipeline stages |
| **SpectraRenderEngine** | High-level renderer coordination |
| **SpectraMaterials** | Material system |
| **SpectraCudaBackend** | CUDA/OptiX path tracing backend (default) |
| **SpectraVulkanBackend** | Vulkan graphics backend |
| **SpectraPlatformRuntime** | OS-level runtime abstraction |
| **SpectraUI** | UI framework |
| **SpectraEditor** | Editor application |
| **SpectraLauncher** | Application launcher |

---

## Build

### Requirements

**Hardware**: NVIDIA GPU, Ampere architecture (SM 80–86) or newer

**Software**:
- Windows 10/11
- Visual Studio 2022
- CMake 3.20+
- Windows SDK
- CUDA Toolkit 12.4+ (required)
- Vulkan SDK (required)

### Setup

Clone with all submodules:

```bash
git clone --recursive -b main https://github.com/devsw-prayas/Spectra.git
cd Spectra
```

Install CUDA (requires admin):

```bash
scripts/cuda.bat
```

Install Vulkan SDK (requires admin):

```bash
scripts/vulkan.bat
```

### Generate and Build

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release_win64
```

Build outputs land in `bin/<config>/`. The pre-generated `Spectra.sln` can also be opened directly in Visual Studio.

### Build Configurations

| Configuration | Purpose |
|---|---|
| `Release_win64` / `Debug_win64` | Standard builds |
| `Release_Kerbecs_NS_win64` | Memory/thread sanitation (normal shadowing) |
| `Release_Kerbecs_ES_win64` | Memory/thread sanitation (enhanced shadowing) |
| `Release_Stratum_win64` | Instrumentation and profiling enabled |
| `Release_Kerbecs_NS_Stratum_win64` | Sanitation + profiling combined |

Debug variants exist for all instrumentation configurations.

### Branch Policy

| Branch | State |
|---|---|
| `main` | Stable releases |
| `bleeding-edge-daily` | Active development — no build guarantees |
| `bleeding-edge` | Experimental features |

---

## License

MIT License. See [LICENSE](LICENSE) for details.
