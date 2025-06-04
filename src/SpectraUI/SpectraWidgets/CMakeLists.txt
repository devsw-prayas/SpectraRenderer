project(SpectraWidgets)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(SpectraWidgets SHARED 
	src/Private/SpectraWidgets.cpp src/Public/SpectraWidgets.h
)

target_include_directories(SpectraWidgets PUBLIC src/Public)

target_link_libraries(SpectraWidgets SpectraImGuiWrapper Instrumenta)
