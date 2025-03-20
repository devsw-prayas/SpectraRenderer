project(SpectraRenderPipeline)

add_library(SpectraRenderPipeline SHARED 
    src/Private/SpectraRenderPipeline.cpp src/Public/SpectraRenderPipeline.h
)

target_include_directories(SpectraRenderPipeline PUBLIC src/Public)

target_link_libraries(SpectraRenderPipeline SpectraDX12Backend SpectraVulkanBackend)
