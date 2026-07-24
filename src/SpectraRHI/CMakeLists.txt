cmake_minimum_required(VERSION 3.20)
project(SpectraRHI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraRHI_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraRHI_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE SpectraRHI_INL      CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")

add_library(SpectraRHI SHARED
    ${SpectraRHI_HEADERS}
    ${SpectraRHI_SOURCE}
    ${SpectraRHI_INL}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${SpectraRHI_HEADERS}
    ${SpectraRHI_SOURCE}
    ${SpectraRHI_INL}
)

target_compile_definitions(SpectraRHI PRIVATE SpectraRHI_EXPORTS)

target_include_directories(SpectraRHI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_precompile_headers(SpectraRHI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraRHI.h)

target_compile_options(SpectraRHI PRIVATE
)
