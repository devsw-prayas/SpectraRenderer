project(SpectraNodes)

add_library(SpectraNodes SHARED 
    src/Private/SpectraNodes.cpp src/Public/SpectraNodes.h
)

target_include_directories(SpectraNodes PUBLIC src/Public)

target_link_libraries(SpectraNodes SpectraImGuiWrapper)
