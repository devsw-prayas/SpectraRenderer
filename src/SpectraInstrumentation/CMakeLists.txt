cmake_minimum_required(VERSION 3.20)
project(SpectraInstrumentation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraInstrumentation_HEADERS           CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraInstrumentation_SOURCE            CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE SpectraInstrumentation_INL               CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")
file(GLOB_RECURSE SpectraInstrumentation_INTERNAL_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Internal/*.h")

add_library(SpectraInstrumentation SHARED
    ${SpectraInstrumentation_HEADERS}
    ${SpectraInstrumentation_SOURCE}
    ${SpectraInstrumentation_INL}
    ${SpectraInstrumentation_INTERNAL_HEADERS}
)
target_compile_definitions(SpectraInstrumentation PRIVATE SpectraInstrumentation_EXPORTS)

target_include_directories(SpectraInstrumentation
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Internal
)

target_precompile_headers(SpectraInstrumentation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraInstrumentation.h)

target_compile_options(SpectraInstrumentation PRIVATE
)
