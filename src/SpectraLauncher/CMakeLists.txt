project(SpectraLauncher)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(SpectraLauncher 
	src/Private/SpectraLauncher.cpp src/Public/SpectraLauncher.h
)

target_include_directories(SpectraLauncher PUBLIC src/Public)

target_link_libraries(SpectraLauncher SpectraEditor SpectraRenderEngine SpectraUI SpectraCore SpectraInstrumentation)
