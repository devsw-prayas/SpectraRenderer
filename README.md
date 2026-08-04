# SpectraRenderer

An unbiased, GPU-accelerated spectral path tracer built from scratch.  
Developed by [Prayas Bharadwaj](https://www.linkedin.com/in/prayas-bharadwaj-053886323/)


## What it is

Spectra operates entirely in the wavelength domain — no RGB approximations. Every system above the syscall and driver boundary is implemented in-house. No external runtime frameworks. The goal is brute-force physical correctness in spectral light transport, with no compromises on determinism or performance.

The default compute backend is CUDA/OptiX (Ampere, SM 80–86). Vulkan is the graphics layer.


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
| **SpectraMemory** | Virtual memory and address space management |
| **SpectraFileSystem** | File I/O and asset access |
| **SpectraRHI** | Render hardware interface abstraction layer |
| **SpectraProfiler** | Engine-side instrumentation and profiling |
| **SpectraEditor** | Editor application |
| **SpectraLauncher** | Application launcher |

## Build

### Requirements

**Hardware**: NVIDIA GPU, Ampere architecture (SM 80–86) or newer

**Software**:
- Windows 10/11
- Visual Studio 2022
- CMake 3.20+
- Windows SDK
- CUDA Toolkit 13.2+ (required)
- Vulkan SDK (required)
- .NET SDK 10+ (required — `driver.bat` runs on it, and the codegen/scaffolding tooling is pure C# for reflection)

### Setup

Clone with all submodules:

```bash
git clone --recursive -b main https://github.com/devsw-prayas/Spectra.git
cd Spectra
```

Install CUDA if missing (requires admin):

```bash
driver.bat cu-check -d
```

Install the Vulkan SDK if missing (requires admin):

```bash
driver.bat vk-check -d
```

### Generate and Build

```bash
driver.bat cmake-init -preq
driver.bat build -c Release_win64
```

`cmake-init -preq` checks that CMake and Visual Studio 2022 are present before configuring (drop `-preq` to skip the check; add `-f` to force a clean reconfigure by deleting `CMakeCache.txt` first). `build -c <Configuration>` then runs `cmake --build` for an already-configured tree; `rebuild -c <Configuration>` does the same with `--clean-first`. `<Configuration>` defaults to `Release_win64` (see [Build Configurations](#build-configurations) below) and must be one of the names listed there.

Or manually:

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release_win64
```

Build outputs land in `bin/<config>/`. The pre-generated `Spectra.sln` can also be opened directly in Visual Studio. `driver.bat run -c <Configuration>` launches the built binary directly.

### Bootstrap Driver

`driver.bat` is a standalone dev-convenience CLI (`scripts/driver/spectra-bootstrap-driver.cs`, a .NET 10 file-based C# app — no `.csproj`) that wraps common day-to-day commands as thin subprocess calls around cmake and the other scripts. It has no opinion on what those tools do internally; it just shells out and reports results.

```bash
driver.bat <command> [options]
```

| Command | Purpose |
|---|---|
| `help`, `-h`, `--help` | Show the command list |
| `cmake-init [-f] [-preq]` | Configure cmake (`-f` deletes `CMakeCache.txt` first; `-preq` checks CMake/VS2022 are present first) |
| `submodule-update [--remote]` | `git submodule update --init --recursive` (`--remote` also pulls the latest commit on each submodule's tracked branch) |
| `module-gen -lib\|-dll\|-exe -cpp17\|-cpp20\|-cpp23 -n "Name" -dir <location>` | Scaffold a new module |
| `cu-check [-d]` | Check for the CUDA toolkit (`-d` installs via `scripts/cuda.bat` if missing) |
| `vk-check [-d]` | Check for the Vulkan SDK (`-d` installs via `scripts/vulkan.bat` if missing) |
| `header-gen -p <Prefix> -np <Namespace> -dir <path>` | Generate a module's `Compiler.h`/`Diagnostic.h` pair |
| `build -c <Configuration>` | `cmake --build` for an already-configured tree |
| `rebuild -c <Configuration>` | Same, with `--clean-first` |
| `run -c <Configuration>` | Launch the configured run target (default `SpectraLauncher`) from `bin/<Configuration>/` |

Every path, tool name, and default — build dir, bin dir, cmake generator, default config, run target, valid `-c` configuration names, the CUDA compiler exe name, the Vulkan header check path, and the CUDA/Vulkan installer script paths — lives in the tracked `scripts/driver/config/config.json`, not hardcoded in the driver itself. A missing field fails loudly rather than silently falling back. `build`/`rebuild`/`run` validate `-c` against `buildConfigurations` and list the valid names if it doesn't match.

### Build Configurations

Also listed in `scripts/driver/config/config.json`'s `buildConfigurations`, which `driver.bat build`/`rebuild`/`run` validate `-c` against.

| Configuration | Purpose |
|---|---|
| `Release_win64` / `Debug_win64` | Standard builds |
| `ReleaseNoOpt_win64` | Release build with optimizations disabled |
| `Release_Kerbecs_NS_win64` / `Debug_Kerbecs_NS_win64` | Memory/thread sanitation (normal shadowing) |
| `Release_Kerbecs_ES_win64` / `Debug_Kerbecs_ES_win64` | Memory/thread sanitation (enhanced shadowing) |

### Branch Policy

| Branch | State |
|---|---|
| `main` | Stable releases |
| `bleeding-edge-daily` | Active development — no build guarantees |
| `bleeding-edge` | Experimental features |


## License

MIT License. See [LICENSE](LICENSE) for details.
