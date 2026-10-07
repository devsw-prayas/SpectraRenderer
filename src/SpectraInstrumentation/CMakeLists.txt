cmake_minimum_required(VERSION 3.20)
project(SpectraInstrumentation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraInstrumentation_HEADERS           CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraInstrumentation_SOURCE            CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE SpectraInstrumentation_INL               CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")
file(GLOB_RECURSE SpectraInstrumentation_INTERNAL_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Internal/*.h")

add_library(SpectraInstrumentation SHARED
    ${SpectraInstrumentation_HEADERS}
    ${SpectraInstrumentation_SOURCE}
    ${SpectraInstrumentation_INL}
    ${SpectraInstrumentation_INTERNAL_HEADERS}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${SpectraInstrumentation_HEADERS}
    ${SpectraInstrumentation_SOURCE}
    ${SpectraInstrumentation_INL}
    ${SpectraInstrumentation_INTERNAL_HEADERS}
)

target_compile_definitions(SpectraInstrumentation PRIVATE SPECTRA_INSTRUMENTATION_SHARED)

# OFF = pure path-tracer build: every SPEC_INST_SCOPE_BEGIN collapses to a bare brace pair.
option(SPECTRA_INSTRUMENTATION "Compile instrumentation scopes in" ON)
target_compile_definitions(SpectraInstrumentation PUBLIC SPECTRA_INSTRUMENTATION_ENABLED=$<BOOL:${SPECTRA_INSTRUMENTATION}>)

# AUTO follows _DEBUG (set by the debug CRT); ON/OFF pin it. Module-scoped, so it never collides with a host's DEBUG.
set(SPEC_INST_ENABLE_DEBUG_CHECKS AUTO CACHE STRING "SpectraInstrumentation asserts: AUTO (follow _DEBUG), ON or OFF")
set_property(CACHE SPEC_INST_ENABLE_DEBUG_CHECKS PROPERTY STRINGS AUTO ON OFF)
if (SPEC_INST_ENABLE_DEBUG_CHECKS STREQUAL "ON")
    target_compile_definitions(SpectraInstrumentation PUBLIC SPEC_INST_DEBUG_CHECKS=1)
elseif (SPEC_INST_ENABLE_DEBUG_CHECKS STREQUAL "OFF")
    target_compile_definitions(SpectraInstrumentation PUBLIC SPEC_INST_DEBUG_CHECKS=0)
endif()

target_include_directories(SpectraInstrumentation
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Internal
)

target_precompile_headers(SpectraInstrumentation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraInstrumentation.h)

target_link_libraries(SpectraInstrumentation SpectraPlatformRuntime)

target_compile_options(SpectraInstrumentation PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)
