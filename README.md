# 📌 Spectra Render Engine  
*Ultra-High-Fidelity Render Engine for Still Images & Real-Time Rendering*

## 🚀 A Passion Project to Build a Bleeding-Edge, Industry-Grade Renderer from Scratch  
Developed by: [Prayas Bharadwaj](https://www.linkedin.com/in/prayas-bharadwaj-053886323/)

---

## 🌟 Overview  
**Spectra** is a next-generation render engine designed for *ultra-high-fidelity* rendering. It powers both real-time and offline path-traced workflows with a spectral rendering core, a modular editor, custom threading, and a high-performance C++ backbone. Every component—from the renderer core to the scene editor—is handcrafted for speed, modularity, and clarity. Because who needs bloat when you can have brilliance?

---

## 💡 Core Features  

- 🎯 **DirectX 12 Renderer**: Built directly on DX12 for low-level, high-performance rendering.  
- 🌈 **Spectral Path Tracing**: Physically-based illumination with multi-wavelength simulation for that extra *ooh, shiny* factor.  
- 🪓 **devswSTL**: A custom STL replacement with SIMD-accelerated containers to make your CPU sweat.  
- 🔬 **Kerbecs**: Memory and thread sanitizer to keep your code squeaky clean and leak-free.  
- 🧵 **Blaze**: NUMA-aware multithreading with lock-free task scheduling—because threads deserve to be free.  
- 📦 **Modular System**: Runtime DLL loading for the editor, pipeline, materials, and backend. Plug and play, baby!  
- 🛠 **ImGui-Based UI**: Custom node editors for materials and a drag-and-drop scene editor that’s smoother than your morning coffee.  
- ⏱ **Instrumenta**: Internal profiler and metrics layer for CPU/GPU diagnostics. Know your bottlenecks before they know you.  

---

## 📂 Project Structure  

```bash
Spectra/
│── CMakeLists.txt
│── src/
│   │── SpectraEditor/            # Main Editor (Uses UI & Render Engine)
│   │── SpectraLauncher/          # Main Executable Entry Point
│   │── SpectraRenderEngine/      # Core Render Engine (PTGI, RTGI, Pipeline)
│   │── SpectraRenderPipeline/    # DX12 API Layer & Command Abstractions
│   │── SpectraMaterials/         # Material Graph, Shader Authoring & Compilation
│   │── SpectraDX12Backend/       # DirectX 12 Backend (Low-level device layer)
│   │── SpectraUI/                # ImGui-based UI Toolkit & Node Graph Editor
│
│── common/
│   │── devswSTL/                 # STL Rewrite (SIMD containers, allocators, traits)
│   │── Blaze/                    # Multithreading & Fiber Framework
│   │── Kerbecs/                  # Memory & Thread Sanitizer
│   │── Instrumenta/              # Profiling, Metrics & Debug Hooks
│
│── docs/                         # Design Papers, Diagrams, Planning Notes
│── README.md
│── .gitignore
│── .github/                      # GitHub Actions & Issue Templates
```

---

## 🛠️ Setup & Build Instructions  

### 📌 Requirements  
- C++20 Compiler (MSVC recommended, because we’re fancy like that)  
- CMake 3.20+  
- Visual Studio 2022  
- Windows SDK  

### 🔧 Build Steps  
1. Clone the repository:  
   ```bash
   git clone https://github.com/devsw-prayas/Spectra.git
   cd Spectra
   ```  
2. Generate project files:  
   ```bash
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022"
   ```  
3. Build the project:  
   ```bash
   cmake --build . --config Release
   ```  

---

## 📜 Contribution Guidelines  
Want to join the rendering revolution? Here’s how:  
- Fork the repository.  
- Create a new feature branch.  
- Commit and push your changes.  
- Open a pull request and bask in the glory of open-source.  

---

## 🗺️ Roadmap  
- ✅ Project Structure & GitHub Setup  
- 🔄 Render Pipeline (DX12 API Layer) – In Progress  
- ⏳ UI Toolkit & Node-Based Material Editor  
- ⏳ Material System & Shader Compilation Stack  
- ⏳ Final Optimization, Denoising, and Tooling Polish  

---

## 📄 License  
📜 MIT License – Use it, modify it, contribute to it freely! No strings attached, unless you’re into that sort of thing.

---

## 🔥 Follow the Journey  
Spectra started as a humble challenge and has grown into a full-blown mission to push rendering tech to the limit with clean, handcrafted systems. Follow my updates on [LinkedIn](https://www.linkedin.com/in/prayas-bharadwaj-053886323/) for the latest on this pixel-pushing adventure!
