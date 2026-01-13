# Spectra Render Engine  
*Next-Generation High Performance Spectral Path Tracer*

## A Passion Project to Build a Bleeding-Edge, Industry-Grade Renderer from Scratch  
Developed by: [Prayas Bharadwaj](https://www.linkedin.com/in/prayas-bharadwaj-053886323/)

## Overview  
*Spectra* belongs to the class of next-generation, truly *unbiased* and fully *GPU-Accelerated* Spectral Path Tracers. It is built as a pure implementation: every system required above the sys-call and driver boundary is implemented in-house, with no reliance on external runtime frameworks. This architectural discipline exists for one reason — to support **brute-force spectral light transport** at scale, without compromises in physical correctness, determinism, or performance.

## Core Features  

- **Unbiased Spectral Path Tracing**  
  Physically-based spectral light transport with true multi-wavelength simulation, designed for brute-force correctness rather than heuristic approximation.  
  Path tracing execution targets CUDA with OptiX acceleration as the default backend.

- **StormSTL**  
  A custom standard library replacement providing allocator-aware, SIMD-optimized containers and low-level primitives suitable for engine and HPC workloads.

- **Kerbecs**  
  Integrated memory and thread sanitation system with selective shadowing for detecting lifetime errors, data races, and memory corruption.

- **Corium**  
  NUMA-aware multithreading and task runtime supporting lock-free scheduling, deterministic execution paths, and explicit execution control.

- **Modular Runtime Architecture**  
  Componentized system design with runtime-loadable modules for rendering backends, pipeline stages, and platform integration.

- **Stratum**  
  Deterministic instrumentation and profiling infrastructure providing zero-overhead tracing and CPU/GPU diagnostics.

## Standalone Infrastructure Subsystems

Spectra is built atop a set of standalone, self-contained infrastructure libraries.
Each subsystem is designed to function independently and can be reused outside of
the Spectra engine without modification.

~~~bash
subsystems/
│── StormSTL/     # Allocators, containers, and memory primitives for engine/HPC use
│── Corium/       # Multithreading, task scheduling, and execution runtime
│── Stratum/      # Deterministic instrumentation, tracing, and profiling
│── Kerbecs/      # Memory and thread sanitation with selective shadowing
│── Leibniz/      # SIMD-optimized mathematics and numerical computation
│── Iota/         # Deep learning inference engine for deterministic execution
~~~



## 🛠️ Setup & Build Instructions  
### Requirements
#### Hardware

- An NVIDIA GPU with CUDA and OptiX support

#### Software

- A C++20-compliant compiler (MSVC recommended)
- CMake 3.20 or newer
- Visual Studio 2022
- Windows SDK
- NVIDIA CUDA Toolkit (required for the default OptiX execution backend)

### Repository Setup and Build

Spectra maintains multiple active branches.

- `main` tracks the latest stable state and is recommended for most users.
- `bleeding-edge-daily` is used for active development and experimentation.

**Warning:**  
The `bleeding-edge-daily` branch is not guaranteed to build, run correctly,
or preserve API stability. It may contain incomplete features, breaking
changes, or experimental code paths. Use this branch only if you are
actively developing Spectra or intentionally testing unstable behavior.

1. Clone the repository and all standalone subsystems:

~~~bash
git clone --recursive -b main https://github.com/devsw-prayas/Spectra.git
cd Spectra
~~~

2. Install the CUDA Toolkit required by the default execution backend:

~~~bash
scripts/cuda.bat
~~~

This script installs and configures the NVIDIA CUDA Toolkit used by
the OptiX-based path tracing backend. Additional backend setup scripts
(e.g., Vulkan) may be introduced in the future.

3. Generate project files:

~~~bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
~~~

4. Build the project:

~~~bash
cmake --build . --config Release
~~~

## Contribution Guidelines

Contributions are welcome.

- Fork the repository and create a feature branch.
- Ensure changes are consistent with existing architectural and coding standards.
- Commit and push your changes with clear, descriptive messages.
- Open a pull request describing the motivation and technical details of the change.

## Research Focus Areas

Current areas of active research and development include:

- Brute-force spectral path tracing and wavelength-domain light transport
- Hybrid BVH construction and traversal strategies for GPU execution
- Deterministic execution and reproducibility across parallel GPU workloads
- Runtime infrastructure for memory safety, profiling, and execution control
- Neural-assisted acceleration of spectral transport (exploratory)

## License

This project is licensed under the MIT License. See the LICENSE file for details.


## Project Updates
Development updates, technical notes, and long-form progress discussions related to Spectra are occasionally shared on LinkedIn:
https://www.linkedin.com/in/prayas-bharadwaj-053886323/
