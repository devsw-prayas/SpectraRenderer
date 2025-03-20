cmake_minimum_required(VERSION 3.20)
project(Spectra)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders in IDEs like Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Include subprojects
add_subdirectory(src/SpectraEditor)
add_subdirectory(src/SpectraDX12Backend)
add_subdirectory(src/SpectraLauncher)
add_subdirectory(src/SpectraMaterials)
add_subdirectory(src/SpectraRenderEngine)
add_subdirectory(src/SpectraRenderPipeline)
add_subdirectory(src/SpectraUI)
add_subdirectory(src/SpectraVulkanBackend)

include_directories(common)

if (MSVC)
    add_compile_options(/MP)  # Enables multi-core compilation
endif()