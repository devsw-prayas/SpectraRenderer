project(SpectraDX12Backend)

add_library(SpectraDX12Backend SHARED 
	src/Private/SpectraDX12Backend.cpp src/Public/SpectraDX12Backend.h
)

target_include_directories(SpectraDX12Backend PUBLIC src/Public)
