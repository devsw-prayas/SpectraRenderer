cmake_minimum_required(VERSION 3.20)
project(SpectraUI)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add UI submodules
add_subdirectory(SpectraEditors)
add_subdirectory(SpectraImGuiWrapper)
add_subdirectory(SpectraNodes)
add_subdirectory(SpectraViewports)
add_subdirectory(SpectraWidgets)

# Create main UI library
add_library(SpectraUI SHARED 
	src/Private/SpectraUI.cpp src/Public/SpectraUI.h
)

# Include directories
target_include_directories(SpectraUI PUBLIC src/Public)

target_link_libraries(SpectraUI 
    SpectraEditors 
    SpectraImGuiWrapper 
    SpectraNodes 
    SpectraViewports 
    SpectraWidgets
)