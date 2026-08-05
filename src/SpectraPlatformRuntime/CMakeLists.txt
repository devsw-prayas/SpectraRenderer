cmake_minimum_required(VERSION 3.20)
project(SpectraPlatformRuntime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraPlatformRuntime_HEADERS           CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraPlatformRuntime_SOURCES           CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE SpectraPlatformRuntime_INTERNAL_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Internal/*.h")

add_library(SpectraPlatformRuntime SHARED
    ${SpectraPlatformRuntime_HEADERS}
    ${SpectraPlatformRuntime_SOURCES}
    ${SpectraPlatformRuntime_INTERNAL_HEADERS}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${SpectraPlatformRuntime_HEADERS}
    ${SpectraPlatformRuntime_SOURCES}
    ${SpectraPlatformRuntime_INTERNAL_HEADERS}
)

set(SPECTRA_MAX_THREADS 1024)
target_compile_definitions(SpectraPlatformRuntime PRIVATE 
    SPECTRA_PLATFORM_MAX_THREADS=${SPECTRA_MAX_THREADS}
    SPECTRA_SHARED
    SPECTRA_BUILDING_RUNTIME
)

target_include_directories(SpectraPlatformRuntime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Internal
)

target_precompile_headers(SpectraPlatformRuntime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraPlatformRuntime.h)

if(WIN32)
    target_link_libraries(SpectraPlatformRuntime PRIVATE
        Synchronization.lib
        Shcore.lib
        Ole32.lib
        Shell32.lib
    )
endif()

target_compile_options(SpectraPlatformRuntime PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)