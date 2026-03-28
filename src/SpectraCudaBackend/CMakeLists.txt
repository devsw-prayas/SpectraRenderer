cmake_minimum_required(VERSION 3.20)
project(SpectraCudaBackend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)
link_directories(${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)

file(GLOB_RECURSE SpectraCudaBackend_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraCudaBackend_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE SpectraCudaBackend_INL      CONFIGURE_DEPENDS "${CMAKE_CURRENT_OURCE_DIR}/src/Public/*.inl")

add_library(SpectraCudaBackend SHARED
    ${SpectraCudaBackend_HEADERS}
    ${SpectraCudaBackend_SOURCE}
    ${SpectraCudaBackend_INL}
)
target_compile_definitions(SpectraCudaBackend PRIVATE SpectraCudaBackend_EXPORTS)

target_include_directories(SpectraCudaBackend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_precompile_headers(SpectraCudaBackend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraCudaBackend.h)

if (MSVC)
    target_compile_options(SpectraCudaBackend PRIVATE
        /W4
        /permissive-
        /Zc:__cplusplus
        /arch:AVX2
    )
else()
    target_compile_options(SpectraCudaBackend PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -mavx2
    )
endif()
