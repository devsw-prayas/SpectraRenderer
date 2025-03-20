project(SpectraImGuiWrapper)

add_library(SpectraImGuiWrapper SHARED 
	src/Private/SpectraImGuiWrapper.cpp src/Public/SpectraImGuiWrapper.h
)

target_include_directories(SpectraImGuiWrapper PUBLIC src/Public)
