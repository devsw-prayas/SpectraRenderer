cmake_minimum_required(VERSION 3.20)
project(SpectraMaterials LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraMaterials_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraMaterials_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")

add_library(SpectraMaterials SHARED
    ${SpectraMaterials_HEADERS}
    ${SpectraMaterials_SOURCE}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${SpectraMaterials_HEADERS}
    ${SpectraMaterials_SOURCE}
)

target_include_directories(SpectraMaterials PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_link_libraries(SpectraMaterials PRIVATE SpectraRenderPipeline)

target_precompile_headers(SpectraMaterials PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraMaterials.h)

target_compile_options(SpectraMaterials PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)
