cmake_minimum_required(VERSION 3.20)
project(SpectraInstrumentation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraInstrumentation_HEADERS           CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraInstrumentation_SOURCE            CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE SpectraInstrumentation_INL               CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")

add_library(SpectraInstrumentation SHARED
    ${SpectraInstrumentation_HEADERS}
    ${SpectraInstrumentation_SOURCE}
    ${SpectraInstrumentation_INL}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${SpectraInstrumentation_HEADERS}
    ${SpectraInstrumentation_SOURCE}
    ${SpectraInstrumentation_INL}
)

target_compile_definitions(SpectraInstrumentation PRIVATE SPECTRA_INSTRUMENTATION_SHARED)

target_include_directories(SpectraInstrumentation
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_precompile_headers(SpectraInstrumentation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraInstrumentation.h)

target_link_libraries(SpectraInstrumentation SpectraPlatformRuntime)

target_compile_options(SpectraInstrumentation PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)
