project(SpectraEditor)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_library(SpectraEditor SHARED 
	src/Private/SpectraEditor.cpp src/Public/SpectraEditor.h
)

target_include_directories(SpectraEditor PUBLIC src/Public)

target_link_libraries(SpectraEditor SpectraUI SpectraRenderEngine)
