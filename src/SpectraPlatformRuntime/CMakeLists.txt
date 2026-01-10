cmake_minimum_required(VERSION 3.20)
project(SpectraPlatformRuntime)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE RUNTIME_HEADERS CONFIGURE_DEPENDS "src/Public/*.h") 
file(GLOB_RECURSE RUNTIME_SOURCES CONFIGURE_DEPENDS "src/Private/*.cpp")

add_library(SpectraPlatformRuntime SHARED ${RUNTIME_HEADERS} ${RUNTIME_SOURCES})

target_precompile_headers(SpectraPlatformRuntime
 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraPlatformRuntime.h)

target_include_directories(SpectraPlatformRuntime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/Public)

set(SPECTRA_MAX_THREADS 2048)
target_compile_definitions(SpectraPlatformRuntime 
    PRIVATE SPECTRA_PLATFORM_MAX_THREADS=${SPECTRA_MAX_THREADS})