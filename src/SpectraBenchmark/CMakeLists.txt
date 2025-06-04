cmake_minimum_required(VERSION 3.20)
project(SpectraBenchmark)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include_directories(src/Public)
add_library(SpectraBenchmark SHARED 
src/Private/SpectraBenchmark.cpp src/Public/SpectraBenchmark.h )

target_link_libraries(SpectraBenchmark SpectraInstrumentation Instrumenta)
