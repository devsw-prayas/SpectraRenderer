project(SpectraImGuiWrapper)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(SpectraImGuiWrapper SHARED 
	src/Private/SpectraImGuiWrapper.cpp src/Public/SpectraImGuiWrapper.h
)

target_include_directories(SpectraImGuiWrapper PUBLIC src/Public)

target_link_libraries(SpectraImGuiWrapper )
