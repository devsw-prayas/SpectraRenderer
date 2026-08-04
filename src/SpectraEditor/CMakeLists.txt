cmake_minimum_required(VERSION 3.20)
project(SpectraEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraEditor_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraEditor_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")

add_library(SpectraEditor SHARED
    ${SpectraEditor_HEADERS}
    ${SpectraEditor_SOURCE}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${SpectraEditor_HEADERS}
    ${SpectraEditor_SOURCE}
)

target_include_directories(SpectraEditor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_precompile_headers(SpectraEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraEditor.h)

target_compile_options(SpectraEditor PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)
