project(SpectraVulkanBackend)

add_library(SpectraVulkanBackend SHARED 
	src/Private/SpectraVulkanBackend.cpp src/Public/SpectraVulkanBackend.h
)

target_include_directories(SpectraVulkanBackend PUBLIC src/Public)
