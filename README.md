# **📌 Spectra Render Engine**
*Ultra-High-Fidelity Render Engine for Still Images & Real-Time Rendering*

## 🚀 Part of a 9-month graphics programming challenge to develop an industry-grade rendering engine.

*📌 Developed by: [Prayas Bharadwaj](https://www.linkedin.com/in/prayas-bharadwaj-053886323/)*

## 🌟 Overview
Spectra is a next-generation render engine designed for maximum quality rendering. It supports both real-time and software-based path-traced rendering, with an advanced material system, virtualized geometry, and a modular architecture.

## 💡 Core Features:

Hybrid Rendering: DX12 & Vulkan backend with seamless switching.
Ultra-High-Fidelity: Supports RTGI, PTGI, advanced materials, dynamic LOD, and high-performance shadows/reflections.
Modular Architecture: Uses DLL-based components for flexibility and scalability.
Powerful UI: ImGui-based custom UI toolkit with node-based material editing.
Drag & Drop Editor: Place meshes, tweak materials, and build scenes effortlessly.
## 📂 Project Structure

``` bash
Spectra/
│── CMakeLists.txt
│── src/
│   │── SpectraEditor/            # Main Editor (Uses UI & Render Engine)
│   │── SpectraLauncher/          # Main Executable Entry Point
│   │── SpectraRenderEngine/      # Core Render Engine (Uses Pipeline & Materials)
│   │── SpectraRenderPipeline/    # Base API Wrapper (DX12 & Vulkan)
│   │── SpectraMaterials/         # Material & Shader System
│   │── SpectraDX12Backend/       # DirectX 12 Backend
│   │── SpectraVulkanBackend/     # Vulkan Backend
│   │── SpectraUI/                # UI Toolkit (ImGui Wrapper & Submodules)
│── common/                       # Shared Headers & Utilities
|   |── SpectraSTl                # Rewrite of the Standard Template Library for high performance rendering
│── docs/                         # Documentation & Design Notes
│── README.md                     # Project Readme
│── .gitignore                     # Git Ignore File
│── .github/                        # GitHub Workflows & Issues Templates

```
## 🛠️ Setup & Build Instructions
### 📌 Requirements
C++ 20 Compiler (MSVC recommended)
CMake 3.20+
Visual Studio 2022
Windows SDK & Vulkan SDK

## 🔧 Build Steps
Clone the Repository

~~~bash
git clone https://github.com/devsw-prayas/Spectra.git
cd Spectra
~~~

Generate Project Files (CMake)

~~~bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
~~~

Build the Project
~~~bash
cmake --build . --config Release
~~~
## 📜 Contribution Guidelines
💡 Want to contribute? Follow these steps:

Fork the repository.
Create a new branch for your feature.
Commit & push your changes.
Open a pull request!
## 📌 Roadmap
*✅ Project Structure & GitHub Setup*

*🔄 Render Pipeline (DX12/Vulkan API Layer) - In Progress*

*⏳ UI Toolkit & Node-Based Editor*

*⏳ Material System & Shader Compilation*

*⏳ Final Optimization & Testing*

## 📄 License
📜 MIT License – Use it, modify it, contribute to it!

*🔥 Spectra is part of a 9-month graphics programming challenge aimed at pushing rendering technology forward. Stay tuned for updates!*

*📌 Follow my journey: [My LinkedIn Profile](https://www.linkedin.com/in/prayas-bharadwaj-053886323/)*
