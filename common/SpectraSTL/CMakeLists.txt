cmake_minimum_required(VERSION 3.20)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(SpectraSTL)

add_library(SpectraSTL SHARED 
	src/Private/SpectraSTL.cpp src/Public/SpectraSTL.h
	src/Public/Traits.h src/Public/Allocators.h
	src/Public/AVX.h
	src/Public/AlignedVector.h src/Private/AlignedVector.cpp
	src/Public/AvxIntrinsics.h
)

target_link_libraries(SpectraSTL SpectraInstrumentation)

target_include_directories(SpectraSTL PUBLIC src/Public)

if (MSVC)
	add_compile_options(/arch:AVX2 /O2)

endif()
