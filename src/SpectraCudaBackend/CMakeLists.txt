cmake_minimum_required(VERSION 3.20)
project(SpectraCudaBackend LANGUAGES CXX)

option(SPECTRA_ENABLE_CUDA "Enable CUDA GPU backend" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)
link_directories(${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)

file(GLOB_RECURSE SpectraCudaBackend_HEADERS          CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraCudaBackend_SOURCE            CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE SpectraCudaBackend_INL               CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")
file(GLOB_RECURSE SpectraCudaBackend_INTERNAL_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Internal/*.h")

add_library(SpectraCudaBackend SHARED
    ${SpectraCudaBackend_HEADERS}
    ${SpectraCudaBackend_SOURCE}
    ${SpectraCudaBackend_INL}
    ${SpectraCudaBackend_INTERNAL_HEADERS}
)

target_include_directories(SpectraCudaBackend
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Internal
)

target_compile_definitions(SpectraCudaBackend PRIVATE
    SPEC_CUDA_BK_SHARED
)

target_precompile_headers(SpectraCudaBackend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraCudaBackend.h
)

if (MSVC)
    target_compile_options(SpectraCudaBackend PRIVATE
        /W4
        /permissive-
        /Zc:__cplusplus
        /arch:AVX2
    )
else()
    target_compile_options(SpectraCudaBackend PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -mavx2
    )
endif()

if (SPECTRA_ENABLE_CUDA)
    find_package(CUDAToolkit REQUIRED)
    target_include_directories(SpectraCudaBackend PRIVATE
        ${CUDAToolkit_INCLUDE_DIRS}
    )
    target_link_libraries(SpectraCudaBackend PRIVATE
        CUDA::cuda_driver
    )
    target_compile_definitions(SpectraCudaBackend PRIVATE
        SPECTRA_CUDA_AVAILABLE
        CUDA_NO_RUNTIME_API
    )

    # OptiX configuration
    find_path(OPTIX_INCLUDE_DIR optix.h
        PATHS
        $ENV{OPTIX_PATH}/include
        $ENV{OPTIX_ROOT_DIR}/include
        "C:/ProgramData/NVIDIA Corporation/OptiX SDK 8.0.0/include"
        "C:/ProgramData/NVIDIA Corporation/OptiX SDK 7.6.0/include"
        "C:/ProgramData/NVIDIA Corporation/OptiX SDK 9.0.0/include"
        "C:/ProgramData/NVIDIA Corporation/OptiX SDK 9.1.0/include"
    )
    
if (OPTIX_INCLUDE_DIR)
    target_include_directories(SpectraCudaBackend PRIVATE ${OPTIX_INCLUDE_DIR})
    target_compile_definitions(SpectraCudaBackend PRIVATE
        SPECTRA_OPTIX_AVAILABLE
    )
else()
    message(FATAL_ERROR "OptiX headers not found")
endif()
endif()