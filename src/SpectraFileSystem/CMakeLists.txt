cmake_minimum_required(VERSION 3.20)
project(SpectraFileSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraFileSystem_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraFileSystem_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE SpectraFileSystem_INL      CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")

add_library(SpectraFileSystem SHARED
    ${SpectraFileSystem_HEADERS}
    ${SpectraFileSystem_SOURCE}
    ${SpectraFileSystem_INL}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${SpectraFileSystem_HEADERS}
    ${SpectraFileSystem_SOURCE}
    ${SpectraFileSystem_INL}
)

target_compile_definitions(SpectraFileSystem PRIVATE 
    SPECTRA_FILESYSTEM_SHARED
    SPECTRA_FILESYSTEM_EXPORTS
)

target_include_directories(SpectraFileSystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_precompile_headers(SpectraFileSystem PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraFileSystem.h)

target_compile_options(SpectraFileSystem PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)
