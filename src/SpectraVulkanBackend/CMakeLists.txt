cmake_minimum_required(VERSION 3.20)
project(SpectraVulkanBackend LANGUAGES CXX)

option(SPECTRA_ENABLE_VULKAN "Enable Vulkan GPU backend" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraVulkanBackend_HEADERS          CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraVulkanBackend_SOURCES          CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE SpectraVulkanBackend_INL              CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")
file(GLOB_RECURSE SpectraVulkanBackend_INTERNAL_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Internal/*.h")

add_library(SpectraVulkanBackend SHARED
    ${SpectraVulkanBackend_HEADERS}
    ${SpectraVulkanBackend_SOURCES}
    ${SpectraVulkanBackend_INL}
    ${SpectraVulkanBackend_INTERNAL_HEADERS}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${SpectraVulkanBackend_HEADERS}
    ${SpectraVulkanBackend_SOURCES}
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

# AUTO follows _DEBUG (set by the debug CRT); ON/OFF pin it. Module-scoped, so it never collides with a host's DEBUG.
set(SPEC_VK_BK_ENABLE_DEBUG_CHECKS AUTO CACHE STRING "SpectraVulkanBackend asserts: AUTO (follow _DEBUG), ON or OFF")
set_property(CACHE SPEC_VK_BK_ENABLE_DEBUG_CHECKS PROPERTY STRINGS AUTO ON OFF)
if (SPEC_VK_BK_ENABLE_DEBUG_CHECKS STREQUAL "ON")
    target_compile_definitions(SpectraVulkanBackend PUBLIC SPEC_VK_BK_DEBUG_CHECKS=1)
elseif (SPEC_VK_BK_ENABLE_DEBUG_CHECKS STREQUAL "OFF")
    target_compile_definitions(SpectraVulkanBackend PUBLIC SPEC_VK_BK_DEBUG_CHECKS=0)
endif()

target_link_libraries(SpectraVulkanBackend PRIVATE SpectraInstrumentation)

target_precompile_headers(SpectraVulkanBackend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraVulkanBackend.h)

target_compile_options(SpectraVulkanBackend PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>:/arch:AVX2 /W4 /permissive- /GL>
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)

if (SPECTRA_ENABLE_VULKAN)
    find_package(Vulkan REQUIRED)
    target_include_directories(SpectraVulkanBackend PRIVATE
        ${Vulkan_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/external/VMA/include
    )
    target_link_libraries(SpectraVulkanBackend PRIVATE
        Vulkan::Vulkan
    )
    target_compile_definitions(SpectraVulkanBackend PRIVATE
        SPECTRA_VULKAN_AVAILABLE
    )
endif()