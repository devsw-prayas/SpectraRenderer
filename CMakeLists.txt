cmake_minimum_required(VERSION 3.20)
project(Spectra)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CONFIGURATION_TYPES
    "Debug_win64"
    "Release_win64"
    "ReleaseNoOpt_win64"
    "Release_Kerbecs_NS_win64"
    "Release_Kerbecs_ES_win64"
    "Release_Stratum_win64"
    "Release_Kerbecs_NS_Stratum_win64"
    "Release_Kerbecs_ES_Stratum_win64"
    "Debug_Kerbecs_NS_win64"
    "Debug_Kerbecs_ES_win64"
    "Debug_Stratum_win64"
    "Debug_Kerbecs_NS_Stratum_win64"
    "Debug_Kerbecs_ES_Stratum_win64"
    CACHE STRING "Available build configurations" FORCE
)

if(MSVC)
    macro(spectra_release_config NAME OPT)
        set(CMAKE_CXX_FLAGS_${NAME}           "${OPT} /DNDEBUG /MD" CACHE STRING "" FORCE)
        set(CMAKE_EXE_LINKER_FLAGS_${NAME}    "" CACHE STRING "" FORCE)
        set(CMAKE_SHARED_LINKER_FLAGS_${NAME} "" CACHE STRING "" FORCE)
        set(CMAKE_STATIC_LINKER_FLAGS_${NAME} "" CACHE STRING "" FORCE)
    endmacro()

    macro(spectra_debug_config NAME OPT)
        set(CMAKE_CXX_FLAGS_${NAME}           "${OPT} /Zi /MDd" CACHE STRING "" FORCE)
        set(CMAKE_EXE_LINKER_FLAGS_${NAME}    "/DEBUG" CACHE STRING "" FORCE)
        set(CMAKE_SHARED_LINKER_FLAGS_${NAME} "/DEBUG" CACHE STRING "" FORCE)
        set(CMAKE_STATIC_LINKER_FLAGS_${NAME} "" CACHE STRING "" FORCE)
    endmacro()

    spectra_release_config(RELEASE_WIN64                        "/O2")
    spectra_release_config(RELEASENOOPT_WIN64                   "/Od")
    spectra_release_config(RELEASE_KERBECS_NS_WIN64             "/O2")
    spectra_release_config(RELEASE_KERBECS_ES_WIN64             "/O2")
    spectra_release_config(RELEASE_STRATUM_WIN64                "/O2")
    spectra_release_config(RELEASE_KERBECS_NS_STRATUM_WIN64     "/O2")
    spectra_release_config(RELEASE_KERBECS_ES_STRATUM_WIN64     "/O2")

    spectra_debug_config(DEBUG_WIN64                        "/Od")
    spectra_debug_config(DEBUG_KERBECS_NS_WIN64             "/Od")
    spectra_debug_config(DEBUG_KERBECS_ES_WIN64             "/Od")
    spectra_debug_config(DEBUG_STRATUM_WIN64                "/Od")
    spectra_debug_config(DEBUG_KERBECS_NS_STRATUM_WIN64     "/Od")
    spectra_debug_config(DEBUG_KERBECS_ES_STRATUM_WIN64     "/Od")
else()
    macro(spectra_config NAME OPT)
        set(CMAKE_CXX_FLAGS_${NAME} "${OPT} -DNDEBUG" CACHE STRING "" FORCE)
    endmacro()

    macro(spectra_debug_config NAME OPT)
        set(CMAKE_CXX_FLAGS_${NAME} "${OPT} -g" CACHE STRING "" FORCE)
    endmacro()

    spectra_config(RELEASE_WIN64                        "-O3")
    spectra_config(RELEASENOOPT_WIN64                   "-O0")
    spectra_config(RELEASE_KERBECS_NS_WIN64             "-O3")
    spectra_config(RELEASE_KERBECS_ES_WIN64             "-O3")
    spectra_config(RELEASE_STRATUM_WIN64                "-O3")
    spectra_config(RELEASE_KERBECS_NS_STRATUM_WIN64     "-O3")
    spectra_config(RELEASE_KERBECS_ES_STRATUM_WIN64     "-O3")

    spectra_debug_config(DEBUG_WIN64                        "-O0")
    spectra_debug_config(DEBUG_KERBECS_NS_WIN64             "-O0")
    spectra_debug_config(DEBUG_KERBECS_ES_WIN64             "-O0")
    spectra_debug_config(DEBUG_STRATUM_WIN64                "-O0")
    spectra_debug_config(DEBUG_KERBECS_NS_STRATUM_WIN64     "-O0")
    spectra_debug_config(DEBUG_KERBECS_ES_STRATUM_WIN64     "-O0")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)

add_subdirectory(common/Stratum)
add_subdirectory(common/Corium)
add_subdirectory(common/StormSTL)
add_subdirectory(common/Kerbecs)
add_subdirectory(common/Leibniz)
add_subdirectory(common/Hades-Benchmark)

add_compile_definitions(
    # Kerbecs only
    $<$<CONFIG:Release_Kerbecs_NS_win64>:KERBECS_SHADOW_NORMAL>
    $<$<CONFIG:Release_Kerbecs_ES_win64>:KERBECS_SHADOW_ENHANCED>
    $<$<CONFIG:Debug_Kerbecs_NS_win64>:KERBECS_SHADOW_NORMAL>
    $<$<CONFIG:Debug_Kerbecs_ES_win64>:KERBECS_SHADOW_ENHANCED>

    # Stratum only
    $<$<CONFIG:Release_Stratum_win64>:STRATUM_ENABLED>
    $<$<CONFIG:Debug_Stratum_win64>:STRATUM_ENABLED>

    # Kerbecs + Stratum
    $<$<CONFIG:Release_Kerbecs_NS_Stratum_win64>:KERBECS_SHADOW_NORMAL;STRATUM_ENABLED>
    $<$<CONFIG:Release_Kerbecs_ES_Stratum_win64>:KERBECS_SHADOW_ENHANCED;STRATUM_ENABLED>
    $<$<CONFIG:Debug_Kerbecs_NS_Stratum_win64>:KERBECS_SHADOW_NORMAL;STRATUM_ENABLED>
    $<$<CONFIG:Debug_Kerbecs_ES_Stratum_win64>:KERBECS_SHADOW_ENHANCED;STRATUM_ENABLED>
)

add_subdirectory(src/SpectraVulkanBackend)
add_subdirectory(src/SpectraCudaBackend)
add_subdirectory(src/SpectraCore)
add_subdirectory(src/SpectraEditor)
add_subdirectory(src/SpectraLauncher)
add_subdirectory(src/SpectraMaterials)
add_subdirectory(src/SpectraRenderEngine)
add_subdirectory(src/SpectraRenderPipeline)
add_subdirectory(src/SpectraPlatformRuntime)
add_subdirectory(src/SpectraMemory)
add_subdirectory(src/SpectraFileSystem)
add_subdirectory(src/SpectraProfiler)

foreach(target Corium StormSTL Stratum Leibniz)
    set_target_properties(${target} PROPERTIES FOLDER "Common")
endforeach()

foreach(target SpectraCore SpectraRenderEngine SpectraRenderPipeline
               SpectraMaterials)
    set_target_properties(${target} PROPERTIES FOLDER "Engine")
endforeach()

foreach(target SpectraVulkanBackend SpectraCudaBackend)
        set_target_properties(${target} PROPERTIES FOLDER "Backend")
endforeach()

foreach(target Kerbecs Hades SpectraProfiler)
    set_target_properties(${target} PROPERTIES FOLDER "Tools")
endforeach()

foreach(target SpectraMemory SpectraFileSystem)
    set_target_properties(${target} PROPERTIES FOLDER "Engine")
endforeach()


set_target_properties(SpectraEditor          PROPERTIES FOLDER "Editor")
set_target_properties(SpectraLauncher        PROPERTIES FOLDER "Launcher")
set_target_properties(SpectraPlatformRuntime PROPERTIES FOLDER "Runtime")

# ---------------------------------------------------------------
# Global Compiler Options
# ---------------------------------------------------------------
if(MSVC)
    add_compile_options(/MP)
    # /arch:AVX2 applied per-target only — not globally
endif()