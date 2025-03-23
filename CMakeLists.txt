cmake_minimum_required(VERSION 3.20)
project(Spectra)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders in IDEs like Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)

# Force the linker to look inside the bin directory also
link_directories(${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)

# Include subprojects
add_subdirectory(src/SpectraEditor)
add_subdirectory(src/SpectraDX12Backend)
add_subdirectory(src/SpectraLauncher)
add_subdirectory(src/SpectraMaterials)
add_subdirectory(src/SpectraRenderEngine)
add_subdirectory(src/SpectraRenderPipeline)
add_subdirectory(src/SpectraUI)
add_subdirectory(src/SpectraVulkanBackend)
add_subdirectory(src/SpectraCore)
add_subdirectory(src/SpectraInstrumentation)

include_directories(common)

if (MSVC)
    add_compile_options(/MP)  # Enables multi-core compilation
endif()
