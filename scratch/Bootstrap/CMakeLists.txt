cmake_minimum_required(VERSION 3.20)
project(Bootstrap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE Bootstrap_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE Bootstrap_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE Bootstrap_INL      CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")

add_library(Bootstrap SHARED
    ${Bootstrap_HEADERS}
    ${Bootstrap_SOURCE}
    ${Bootstrap_INL}
)
target_compile_definitions(Bootstrap PRIVATE BOOTSTRAP_EXPORTS)
target_include_directories(Bootstrap PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_precompile_headers(Bootstrap PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/Bootstrap.h)

target_compile_options(Bootstrap PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>>:-mavx2 -Wall -Wextra>
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${Bootstrap_HEADERS}
    ${Bootstrap_SOURCE}
)

target_link_libraries(Bootstrap PRIVATE StormSTL Corium SpectraPlatformRuntime SpectraInstrumentation SpectraMemory Leibniz Kerbecs)
