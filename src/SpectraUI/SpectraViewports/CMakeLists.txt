project(SpectraViewports)

add_library(SpectraViewports SHARED 
	src/Private/SpectraViewports.cpp src/Public/SpectraViewports.h
)

target_include_directories(SpectraViewports PUBLIC src/Public)

target_link_libraries(SpectraViewports SpectraImGuiWrapper)
