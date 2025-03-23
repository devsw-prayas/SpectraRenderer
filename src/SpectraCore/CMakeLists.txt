project(SpectraDX12Backend)

add_library(SpectraCore SHARED 
	src/Private/SpectraCore.cpp src/Public/SpectraCore.h
	src/Private/S_int4.cpp src/Public/S_int4.h
)

target_include_directories(SpectraCore PUBLIC src/Public)

target_link_libraries(SpectraCore SpectraInstrumentation)

