project(SpectraRenderEngine)

add_library(SpectraRenderEngine SHARED 
	src/Private/SpectraRenderEngine.cpp src/Public/SpectraRenderEngine.h
)

target_include_directories(SpectraRenderEngine PUBLIC src/Public)

target_link_libraries(SpectraRenderEngine SpectraRenderPipeline)
