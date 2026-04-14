cmake_minimum_required(VERSION 3.20)
project(SpectraMemory LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraMemory_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraMemory_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE SpectraMemory_INL      CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")

add_library(SpectraMemory SHARED
    ${SpectraMemory_HEADERS}
    ${SpectraMemory_SOURCE}
    ${SpectraMemory_INL}
)

target_compile_definitions(SpectraMemory PRIVATE 
    SPECTRA_MEMORY_SHARED
    SPECTRA_MEMORY_EXPORTS
)

target_include_directories(SpectraMemory PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_precompile_headers(SpectraMemory PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraMemory.h)

target_compile_options(SpectraMemory PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)
