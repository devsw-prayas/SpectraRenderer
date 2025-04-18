project(SpectraInstrumentation)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_library(SpectraInstrumentation SHARED 
	src/Private/SpectraInstrumentation.cpp src/Public/SpectraInstrumentation.h
)

target_include_directories(SpectraInstrumentation PUBLIC src/Public)

