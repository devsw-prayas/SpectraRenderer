cmake_minimum_required(VERSION 3.20)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(WARNING
        "In-source build detected (configuring directly in the repo root). "
        "This scatters generated project files across every module directory "
        "and risks accidentally committing build artifacts. Prefer an "
        "out-of-source build: cmake -S . -B build")
endif()

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
    "Debug_Kerbecs_NS_win64"
    "Debug_Kerbecs_ES_win64"
    CACHE STRING "Available build configurations" FORCE
)

if(MSVC)
    # FORCE-written every configure on purpose: these are derived build types,
    # not user-tunable knobs. Edit them here, never in the CMake cache.
    #
    #   /Z7             debug info inside each .obj/.lib. No scattered vc*.pdb in
    #                   build/, no /FS contention under /MP. The only PDB produced
    #                   is the per-.dll/.exe linker PDB, in bin/<config>/ (see
    #                   CMAKE_PDB_OUTPUT_DIRECTORY below).
    #   /DEBUG          emits that linker PDB; doesn't change codegen.
    #   /OPT:REF        drops unreferenced funcs/data (needs /Gy, set globally).
    #   /OPT:NOICF      keeps identical small funcs distinct so stack traces don't
    #                   mislabel them.
    #   /INCREMENTAL:NO required alongside /OPT:*; also silences LNK4075.
    macro(_spectra_cfg NAME CXX LINK)
        set(CMAKE_CXX_FLAGS_${NAME}           "${CXX}"  CACHE STRING "" FORCE)
        set(CMAKE_EXE_LINKER_FLAGS_${NAME}    "${LINK}" CACHE STRING "" FORCE)
        set(CMAKE_SHARED_LINKER_FLAGS_${NAME} "${LINK}" CACHE STRING "" FORCE)
        set(CMAKE_STATIC_LINKER_FLAGS_${NAME} ""        CACHE STRING "" FORCE)
    endmacro()

    set(_spectra_link_opt   "/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:NOICF")
    set(_spectra_link_plain "/DEBUG /INCREMENTAL:NO")

    #           NAME                       CXX flags               LINK flags
    _spectra_cfg(RELEASE_WIN64            "/O2 /Z7 /DNDEBUG /MD"  "${_spectra_link_opt}")
    _spectra_cfg(RELEASE_KERBECS_NS_WIN64 "/O2 /Z7 /DNDEBUG /MD"  "${_spectra_link_opt}")
    _spectra_cfg(RELEASE_KERBECS_ES_WIN64 "/O2 /Z7 /DNDEBUG /MD"  "${_spectra_link_opt}")
    # ReleaseNoOpt: NDEBUG (asserts compiled out) but /Od, to isolate
    # /O2-exposed UB from genuine error paths. No /OPT:* on an unoptimised build.
    _spectra_cfg(RELEASENOOPT_WIN64       "/Od /Z7 /DNDEBUG /MD"  "${_spectra_link_plain}")

    _spectra_cfg(DEBUG_WIN64              "/Od /Z7 /MDd"          "${_spectra_link_plain}")
    _spectra_cfg(DEBUG_KERBECS_NS_WIN64   "/Od /Z7 /MDd"          "${_spectra_link_plain}")
    _spectra_cfg(DEBUG_KERBECS_ES_WIN64   "/Od /Z7 /MDd"          "${_spectra_link_plain}")

    set(CMAKE_MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug_win64,Debug_Kerbecs_NS_win64,Debug_Kerbecs_ES_win64>:Debug>DLL")
else()
    # GCC/Clang. Config names kept identical to the MSVC set: driver.bat
    # validates -c against them and bin/<config>/ is keyed on them.
    macro(_spectra_cfg NAME CXX)
        set(CMAKE_CXX_FLAGS_${NAME} "${CXX}" CACHE STRING "" FORCE)
    endmacro()

    _spectra_cfg(RELEASE_WIN64            "-O3 -g -DNDEBUG")
    _spectra_cfg(RELEASE_KERBECS_NS_WIN64 "-O3 -g -DNDEBUG")
    _spectra_cfg(RELEASE_KERBECS_ES_WIN64 "-O3 -g -DNDEBUG")
    _spectra_cfg(RELEASENOOPT_WIN64       "-O0 -g -DNDEBUG")

    _spectra_cfg(DEBUG_WIN64              "-O0 -g")
    _spectra_cfg(DEBUG_KERBECS_NS_WIN64   "-O0 -g")
    _spectra_cfg(DEBUG_KERBECS_ES_WIN64   "-O0 -g")
endif()

# Overridable so a clang-cl configure (a separate build tree, since a single
# CMake cache can't mix toolsets) can land in its own output directory instead
# of silently overwriting the default MSVC build's binaries under the same
# configuration names. See driver.bat's `cmake-init -clangcl`.
set(SPECTRA_BIN_SUBDIR "bin" CACHE STRING "Top-level output directory name under the repo root")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/${SPECTRA_BIN_SUBDIR}/$<CONFIG>)

# Linker PDBs (one per .dll/.exe) land beside their binary, not scattered
# through build/. With /Z7 this is the only PDB the build produces.
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/${SPECTRA_BIN_SUBDIR}/$<CONFIG>)

# Must be set before add_subdirectory(common/StormSTL): it reads these via option() as soon
# as it's processed. No FORCE, so a -D override on the cmake invocation sticks across
# cmake-init reruns instead of getting reset.
set(STL_PREFER_SSE    OFF CACHE BOOL "Prefer SSE in StormSTL vectorized ops")
set(STL_PREFER_AVX    OFF CACHE BOOL "Prefer AVX in StormSTL vectorized ops")
set(STL_PREFER_AVX512 ON  CACHE BOOL "Prefer AVX512f in StormSTL vectorized ops")

# Targets that include StormSTL's header-only vectorized ops directly, and so need to
# compile with the same /arch as StormSTL itself. Otherwise they silently compile the
# SSE/AVX2 path regardless of the STL_PREFER_* selection above. Add/remove names here as
# that changes; targets not listed don't pay for an /arch they don't need.
set(SPECTRA_STL_ARCH_CONSUMERS SpectraLauncher)

if(MSVC)
    if(STL_PREFER_AVX512)
        set(SPECTRA_STL_ARCH_FLAG "/arch:AVX512")
    elseif(STL_PREFER_AVX)
        set(SPECTRA_STL_ARCH_FLAG "/arch:AVX2")
    else()
        set(SPECTRA_STL_ARCH_FLAG "")
    endif()
else()
    if(STL_PREFER_AVX512)
        set(SPECTRA_STL_ARCH_FLAG "-mavx512f -mavx512bw")
    elseif(STL_PREFER_AVX)
        set(SPECTRA_STL_ARCH_FLAG "-mavx2")
    else()
        set(SPECTRA_STL_ARCH_FLAG "")
    endif()
endif()

add_subdirectory(common/Corium)
add_subdirectory(common/StormSTL)
add_subdirectory(common/Kerbecs)
add_subdirectory(common/Leibniz)
add_subdirectory(common/Hades-Benchmark)
add_subdirectory(common/Valhalla)

add_compile_definitions(
    $<$<CONFIG:Release_Kerbecs_NS_win64>:KERBECS_SHADOW_NORMAL>
    $<$<CONFIG:Release_Kerbecs_ES_win64>:KERBECS_SHADOW_ENHANCED>
    $<$<CONFIG:Debug_Kerbecs_NS_win64>:KERBECS_SHADOW_NORMAL>
    $<$<CONFIG:Debug_Kerbecs_ES_win64>:KERBECS_SHADOW_ENHANCED>
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
add_subdirectory(src/SpectraRHI)
add_subdirectory(src/SpectraInstrumentation)

foreach(target ${SPECTRA_STL_ARCH_CONSUMERS})
    target_compile_options(${target} PRIVATE
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:${SPECTRA_STL_ARCH_FLAG} /W4 /permissive->
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:${SPECTRA_STL_ARCH_FLAG} -Wall -Wextra -Wpedantic>
    )
endforeach()

set(SPECTRA_COMMON_TARGETS Corium StormSTL Leibniz)
set(SPECTRA_REFLECTION_TARGETS ValhallaCore Valhalla-Gen ValhallaCodegen)
set(SPECTRA_VALHALLA_CODEGEN_DEPENDENTS
    SpectraVulkanBackend SpectraCudaBackend SpectraCore SpectraEditor
    SpectraLauncher SpectraMaterials SpectraRenderEngine SpectraRenderPipeline
    SpectraPlatformRuntime SpectraMemory SpectraFileSystem SpectraProfiler
    SpectraRHI SpectraInstrumentation)
set(SPECTRA_ENGINE_TARGETS
    SpectraCore SpectraRenderEngine SpectraRenderPipeline SpectraMaterials
    SpectraMemory SpectraFileSystem SpectraRHI)
set(SPECTRA_BACKEND_TARGETS SpectraVulkanBackend SpectraCudaBackend)
set(SPECTRA_TOOLS_TARGETS Kerbecs Hades-Core Hades-Codegen Hades-Driver SpectraProfiler SpectraInstrumentation)

foreach(target ${SPECTRA_COMMON_TARGETS})
    set_target_properties(${target} PROPERTIES FOLDER "Common")
endforeach()

foreach(target ${SPECTRA_REFLECTION_TARGETS})
    set_target_properties(${target} PROPERTIES FOLDER "Reflection")
endforeach()

foreach(target ${SPECTRA_VALHALLA_CODEGEN_DEPENDENTS})
    add_dependencies(${target} ValhallaCodegen)
endforeach()

foreach(target ${SPECTRA_ENGINE_TARGETS})
    set_target_properties(${target} PROPERTIES FOLDER "Engine")
endforeach()

foreach(target ${SPECTRA_BACKEND_TARGETS})
    set_target_properties(${target} PROPERTIES FOLDER "Backend")
endforeach()

foreach(target ${SPECTRA_TOOLS_TARGETS})
    set_target_properties(${target} PROPERTIES FOLDER "Tools")
endforeach()

set_target_properties(SpectraEditor          PROPERTIES FOLDER "Editor")
set_target_properties(SpectraLauncher        PROPERTIES FOLDER "Launcher")
set_target_properties(SpectraPlatformRuntime PROPERTIES FOLDER "Runtime")

# Global Compiler / Linker Options
if(MSVC)
    add_compile_options(/MP /Gy /Zc:inline)
else()
    add_compile_options(-ffunction-sections -fdata-sections)
    add_link_options(LINKER:--gc-sections)
endif()