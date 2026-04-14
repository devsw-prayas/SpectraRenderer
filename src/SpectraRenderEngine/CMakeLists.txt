cmake_minimum_required(VERSION 3.20)
project(SpectraRenderEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraRenderEngine_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraRenderEngine_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")

add_library(SpectraRenderEngine SHARED
    ${SpectraRenderEngine_HEADERS}
    ${SpectraRenderEngine_SOURCE}
)

target_include_directories(SpectraRenderEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_link_libraries(SpectraRenderEngine PRIVATE SpectraRenderPipeline)

target_precompile_headers(SpectraRenderEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraRenderEngine.h)

target_compile_options(SpectraRenderEngine PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)
