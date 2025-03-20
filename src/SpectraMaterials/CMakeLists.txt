project(SpectraMaterials)

add_library(SpectraMaterials SHARED 
	src/Private/SpectraMaterials.cpp src/Public/SpectraMaterials.h
)

target_include_directories(SpectraMaterials PUBLIC src/Public)

target_link_libraries(SpectraMaterials SpectraRenderPipeline)
