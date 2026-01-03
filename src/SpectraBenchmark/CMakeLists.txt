cmake_minimum_required(VERSION 3.20)
project(SpectraBenchmark)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders in IDEs like Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

file(GLOB_RECURSE BENCH_PUBLIC "src/Public/*.h")
file(GLOB_RECURSE BENCH_PRIVATE "src/Private/*.cpp")

add_library(SpectraBenchmark SHARED ${BENCH_PUBLIC} ${BENCH_PRIVATE})

target_include_directories(SpectraBenchmark PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_precompile_headers(SpectraBenchmark PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraBenchmark.h)