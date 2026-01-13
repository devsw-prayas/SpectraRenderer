project(SpectraDX12Backend)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(SpectraCore SHARED 
	src/Private/SpectraCore.cpp src/Public/SpectraCore.h
)

target_include_directories(SpectraCore PUBLIC src/Public)

target_link_libraries(SpectraCore)

