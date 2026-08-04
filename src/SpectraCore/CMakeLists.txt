cmake_minimum_required(VERSION 3.20)
project(SpectraCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraCore_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraCore_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")

add_library(SpectraCore SHARED
    ${SpectraCore_HEADERS}
    ${SpectraCore_SOURCE}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${SpectraCore_HEADERS}
    ${SpectraCore_SOURCE}
)

target_compile_definitions(SpectraCore PRIVATE
    SPECTRA_CORE_SHARED
    SPECTRA_BUILDING_RUNTIME
)

target_include_directories(SpectraCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_precompile_headers(SpectraCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraCore.h)

target_compile_options(SpectraCore PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)
