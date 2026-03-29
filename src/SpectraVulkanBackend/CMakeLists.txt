cmake_minimum_required(VERSION 3.20)
project(SpectraVulkanBackend LANGUAGES CXX)

option(SPECTRA_ENABLE_VULKAN "Enable Vulkan GPU backend" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)
link_directories(${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)

file(GLOB_RECURSE SpectraVulkanBackend_HEADERS         CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraVulkanBackend_SOURCE           CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE SpectraVulkanBackend_INL              CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")
file(GLOB_RECURSE SpectraVulkanBackend_INTERNAL_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Internal/*.h")

add_library(SpectraVulkanBackend SHARED
    ${SpectraVulkanBackend_HEADERS}
    ${SpectraVulkanBackend_SOURCE}
    ${SpectraVulkanBackend_INL}
    ${SpectraVulkanBackend_INTERNAL_HEADERS}
)

target_include_directories(SpectraVulkanBackend
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Internal
)

target_compile_definitions(SpectraVulkanBackend PRIVATE
    SPEC_VK_BK_SHARED
)

target_precompile_headers(SpectraVulkanBackend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraVulkanBackend.h
)

if (MSVC)
    target_compile_options(SpectraVulkanBackend PRIVATE
        /W4
        /permissive-
        /Zc:__cplusplus
        /arch:AVX2
    )
else()
    target_compile_options(SpectraVulkanBackend PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -mavx2
    )
endif()

if (SPECTRA_ENABLE_VULKAN)
    find_package(Vulkan REQUIRED)
    target_include_directories(SpectraVulkanBackend PRIVATE
        ${Vulkan_INCLUDE_DIRS}
    )
    target_link_libraries(SpectraVulkanBackend PRIVATE
        Vulkan::Vulkan
    )
    target_compile_definitions(SpectraVulkanBackend PRIVATE
        SPECTRA_VULKAN_AVAILABLE
    )
endif()