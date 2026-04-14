cmake_minimum_required(VERSION 3.20)
project(SpectraProfiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraProfiler_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraProfiler_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE SpectraProfiler_INL      CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")

add_library(SpectraProfiler SHARED
    ${SpectraProfiler_HEADERS}
    ${SpectraProfiler_SOURCE}
    ${SpectraProfiler_INL}
)

target_compile_definitions(SpectraProfiler PRIVATE
    SPECTRA_PROFILER_SHARED
    SPECTRA_BUILDING_RUNTIME
    SpectraProfiler_EXPORTS
)

target_include_directories(SpectraProfiler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_precompile_headers(SpectraProfiler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraProfiler.h)

target_compile_options(SpectraProfiler PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)
