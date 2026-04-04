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
target_compile_definitions(SpectraProfiler PRIVATE SpectraProfiler_EXPORTS)

target_include_directories(SpectraProfiler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_precompile_headers(SpectraProfiler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/Core.h)

if (MSVC)
    target_compile_options(SpectraProfiler PRIVATE
        /W4
        /permissive-
        /Zc:__cplusplus
        /arch:AVX2
    )
else()
    target_compile_options(SpectraProfiler PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -mavx2
    )
endif()
