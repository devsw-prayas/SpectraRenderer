project(SpectraInstrumentation)

add_library(SpectraInstrumentation SHARED 
	src/Private/SpectraInstrumentation.cpp src/Public/SpectraInstrumentation.h
)

target_include_directories(SpectraInstrumentation PUBLIC src/Public)

