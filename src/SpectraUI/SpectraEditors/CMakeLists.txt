project(SpectraEditors)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(SpectraEditors SHARED 
    src/Private/SpectraEditors.cpp src/Public/SpectraEditors.h
)

target_include_directories(SpectraEditors PUBLIC src/Public)

target_link_libraries(SpectraEditors SpectraImGuiWrapper Instrumenta)
