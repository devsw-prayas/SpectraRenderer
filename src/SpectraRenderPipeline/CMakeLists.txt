cmake_minimum_required(VERSION 3.20)
project(SpectraRenderPipeline LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraRenderPipeline_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraRenderPipeline_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")

add_library(SpectraRenderPipeline SHARED
    ${SpectraRenderPipeline_HEADERS}
    ${SpectraRenderPipeline_SOURCE}
)

target_include_directories(SpectraRenderPipeline PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_precompile_headers(SpectraRenderPipeline PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraRenderPipeline.h)

target_compile_options(SpectraRenderPipeline PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)
