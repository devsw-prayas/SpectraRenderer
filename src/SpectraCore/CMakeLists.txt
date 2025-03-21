project(SpectraDX12Backend)

add_library(SpectraCore SHARED 
	src/Private/SpectraCore.cpp src/Public/SpectraCore.h
)

target_include_directories(SpectraCore PUBLIC src/Public)
