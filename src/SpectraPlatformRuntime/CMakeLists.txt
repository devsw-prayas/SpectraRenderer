cmake_minimum_required(VERSION 3.20)
project(SpectraPlatformRuntime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraPlatformRuntime_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraPlatformRuntime_SOURCES  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")

add_library(SpectraPlatformRuntime SHARED
    ${SpectraPlatformRuntime_HEADERS}
    ${SpectraPlatformRuntime_SOURCES}
)

set(SPECTRA_MAX_THREADS 2048)
target_compile_definitions(SpectraPlatformRuntime PRIVATE 
    SPECTRA_PLATFORM_MAX_THREADS=${SPECTRA_MAX_THREADS}
    SPECTRA_SHARED
    SPECTRA_BUILDING_RUNTIME
)

target_include_directories(SpectraPlatformRuntime PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_precompile_headers(SpectraPlatformRuntime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraPlatformRuntime.h)

target_compile_options(SpectraPlatformRuntime PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)