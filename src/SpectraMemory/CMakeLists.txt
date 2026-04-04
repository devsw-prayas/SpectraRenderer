cmake_minimum_required(VERSION 3.20)
project(SpectraMemory LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraMemory_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraMemory_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE SpectraMemory_INL      CONFIGURE_DEPENDS "${CMAKE_CURRENT_OURCE_DIR}/src/Public/*.inl")

add_library(SpectraMemory SHARED
    ${SpectraMemory_HEADERS}
    ${SpectraMemory_SOURCE}
    ${SpectraMemory_INL}
)
target_compile_definitions(SpectraMemory PRIVATE SpectraMemory_EXPORTS)

target_include_directories(SpectraMemory PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_precompile_headers(SpectraMemory PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraMemory.h)

if (MSVC)
    target_compile_options(SpectraMemory PRIVATE
        /W4
        /permissive-
        /Zc:__cplusplus
        /arch:AVX2
    )
else()
    target_compile_options(SpectraMemory PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -mavx2
    )
endif()
