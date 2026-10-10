cmake_minimum_required(VERSION 3.20)
project(SpectraCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraCore_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraCore_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE SpectraCore_INL      CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")
file(GLOB_RECURSE SpectraCore_INTERNAL_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Internal/*.h")

add_library(SpectraCore SHARED
    ${SpectraCore_HEADERS}
    ${SpectraCore_SOURCE}
    ${SpectraCore_INL}
    ${SpectraCore_INTERNAL_HEADERS}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${SpectraCore_HEADERS}
    ${SpectraCore_SOURCE}
    ${SpectraCore_INL}
    ${SpectraCore_INTERNAL_HEADERS}
)

# No SPECTRA_BUILDING_RUNTIME: that is PlatformRuntime's export switch, and defining it here would re-export its symbols.
target_compile_definitions(SpectraCore PRIVATE
    SPECTRA_CORE_SHARED
)

# AUTO follows _DEBUG (set by the debug CRT); ON/OFF pin it. Module-scoped, so it never collides with a host's DEBUG.
set(SPEC_CORE_ENABLE_DEBUG_CHECKS AUTO CACHE STRING "SpectraCore asserts: AUTO (follow _DEBUG), ON or OFF")
set_property(CACHE SPEC_CORE_ENABLE_DEBUG_CHECKS PROPERTY STRINGS AUTO ON OFF)
if (SPEC_CORE_ENABLE_DEBUG_CHECKS STREQUAL "ON")
    target_compile_definitions(SpectraCore PUBLIC SPEC_CORE_DEBUG_CHECKS=1)
elseif (SPEC_CORE_ENABLE_DEBUG_CHECKS STREQUAL "OFF")
    target_compile_definitions(SpectraCore PUBLIC SPEC_CORE_DEBUG_CHECKS=0)
endif()

target_include_directories(SpectraCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Internal
)

target_precompile_headers(SpectraCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraCore.h)

target_compile_options(SpectraCore PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)

# PUBLIC: the Corium surface is re-exported through SpectraCore's headers, so consumers need Corium's includes too.
target_link_libraries(SpectraCore PUBLIC SpectraPlatformRuntime SpectraMemory Corium)
