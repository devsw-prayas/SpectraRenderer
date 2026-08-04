cmake_minimum_required(VERSION 3.20)
project(SpectraLauncher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraLauncher_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraLauncher_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")

add_executable(SpectraLauncher
    ${SpectraLauncher_HEADERS}
    ${SpectraLauncher_SOURCE}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${SpectraLauncher_HEADERS}
    ${SpectraLauncher_SOURCE}
)

target_include_directories(SpectraLauncher PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_link_libraries(SpectraLauncher SpectraPlatformRuntime Hades-Core Leibniz Kerbecs)

target_precompile_headers(SpectraLauncher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraLauncher.h)

target_compile_options(SpectraLauncher PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)
