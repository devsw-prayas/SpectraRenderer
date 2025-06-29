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
add_subdirectory(common/Instrumenta)
add_subdirectory(common/Blaze)
add_subdirectory(common/devswSTL)
add_subdirectory(common/Kerbecs)

add_subdirectory(src/SpectraDX12Backend)
add_subdirectory(src/SpectraVulkanBackend)
add_subdirectory(src/SpectraInstrumentation)
add_subdirectory(src/SpectraBenchmark)
add_subdirectory(src/SpectraCore)
add_subdirectory(src/SpectraEditor)
add_subdirectory(src/SpectraLauncher)
add_subdirectory(src/SpectraMaterials)
add_subdirectory(src/SpectraRenderEngine)
add_subdirectory(src/SpectraRenderPipeline)
add_subdirectory(src/SpectraUI)

include_directories(common)

# Common Libraries
set_target_properties(Blaze PROPERTIES FOLDER "Common")
set_target_properties(devswSTL PROPERTIES FOLDER "Common")
set_target_properties(Instrumenta PROPERTIES FOLDER "Common")

# Engine Modules
set_target_properties(SpectraCore PROPERTIES FOLDER "Engine")
set_target_properties(SpectraRenderEngine PROPERTIES FOLDER "Engine")
set_target_properties(SpectraRenderPipeline PROPERTIES FOLDER "Engine")
set_target_properties(SpectraMaterials PROPERTIES FOLDER "Engine")
set_target_properties(SpectraDX12Backend PROPERTIES FOLDER "Engine")
set_target_properties(SpectraVulkanBackend PROPERTIES FOLDER "Engine")

# Editor
set_target_properties(SpectraEditor PROPERTIES FOLDER "Editor")

# Launcher
set_target_properties(SpectraLauncher PROPERTIES FOLDER "Launcher")

# Tools
set_target_properties(SpectraBenchmark PROPERTIES FOLDER "Tools")
set_target_properties(Kerbecs PROPERTIES FOLDER "Tools")

# UI
set_target_properties(SpectraUI PROPERTIES FOLDER "UI")


if (MSVC)
	add_compile_options(/MP)  # Enables multi-core compilation
	add_compile_options(/arch:AVX2 /O2)
endif()
