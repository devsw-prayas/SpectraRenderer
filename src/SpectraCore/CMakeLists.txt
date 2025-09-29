project(SpectraDX12Backend)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(SpectraCore SHARED 
	src/Private/SpectraCore.cpp src/Public/SpectraCore.h
	src/Private/S_int4.cpp src/Public/S_int4.h
	src/Private/S_uint4.cpp src/Public/S_uint4.h
	src/Public/Vector.h src/Private/Vector.cpp src/Public/Vector.inl
)

target_include_directories(SpectraCore PUBLIC src/Public)

target_link_libraries(SpectraCore)

