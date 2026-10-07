cmake_minimum_required(VERSION 3.20)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(WARNING
        "In-source build detected (configuring directly in the repo root). "
        "This scatters generated project files across every module directory "
        "and risks accidentally committing build artifacts. Prefer an "
        "out-of-source build: cmake -S . -B build")
endif()

project(Spectra)

# No vcpkg dependencies; opt out of a user-wide `vcpkg integrate install` (its post-build step needs pwsh).
set(CMAKE_VS_GLOBALS "VcpkgEnabled=false")

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
    # FORCE-written: derived build types, edit here rather than in the cache.
    # /Z7 keeps debug info in the .obj (no vc*.pdb, no /FS contention under /MP), so the linker PDB is the only one.
    # /OPT:NOICF keeps identical functions distinct so stack traces stay accurate; /INCREMENTAL:NO is required with /OPT.
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
    # ReleaseNoOpt: asserts off but /Od, to tell /O2-exposed UB apart from real error paths.
    _spectra_cfg(RELEASENOOPT_WIN64       "/Od /Z7 /DNDEBUG /MD"  "${_spectra_link_plain}")

    _spectra_cfg(DEBUG_WIN64              "/Od /Z7 /MDd"          "${_spectra_link_plain}")
    _spectra_cfg(DEBUG_KERBECS_NS_WIN64   "/Od /Z7 /MDd"          "${_spectra_link_plain}")
    _spectra_cfg(DEBUG_KERBECS_ES_WIN64   "/Od /Z7 /MDd"          "${_spectra_link_plain}")

    set(CMAKE_MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug_win64,Debug_Kerbecs_NS_win64,Debug_Kerbecs_ES_win64>:Debug>DLL")
else()
    # Same config names as MSVC: driver.bat validates -c against them.
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

# Overridden by `driver.bat cmake-init -clangcl` so clang-cl output doesn't overwrite the MSVC binaries.
set(SPECTRA_BIN_SUBDIR "bin" CACHE STRING "Top-level output directory name under the repo root")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/${SPECTRA_BIN_SUBDIR}/$<CONFIG>)

# PDBs stay out of bin/; binaries embed their absolute path. bin -> pdb, bin-clangcl -> pdb-clangcl.
string(REGEX REPLACE "^bin" "pdb" _spectra_pdb_default "${SPECTRA_BIN_SUBDIR}")
set(SPECTRA_PDB_SUBDIR "${_spectra_pdb_default}" CACHE STRING "Top-level linker PDB directory name under the repo root")
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/${SPECTRA_PDB_SUBDIR}/$<CONFIG>)

# Set before add_subdirectory(common/StormSTL), which reads them via option(). No FORCE, so -D overrides stick.
set(STL_PREFER_SSE    OFF CACHE BOOL "Prefer SSE in StormSTL vectorized ops")
set(STL_PREFER_AVX    ON  CACHE BOOL "Prefer AVX in StormSTL vectorized ops")
set(STL_PREFER_AVX512 OFF CACHE BOOL "Prefer AVX512f in StormSTL vectorized ops")

# Targets that inline StormSTL's vectorized headers need its /arch, or they silently get the SSE/AVX2 path.
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

# C4251/C4324 are by design in an all-DLL, alignas-heavy codebase built by one toolchain at a time.
add_compile_options($<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>:/wd4251$<SEMICOLON>/wd4324>)

# Spectra's CUDA backend needs Corium's CUDA layer; Corium alone would only auto-detect it.
set(CORIUM_ENABLE_CUDA ON CACHE BOOL "Enable CUDA GPU backend")

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
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>:${SPECTRA_STL_ARCH_FLAG} /W4 /permissive->
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>>>:${SPECTRA_STL_ARCH_FLAG} -Wall -Wextra -Wpedantic>
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

if(MSVC)
    add_compile_options(/MP /Gy /Zc:inline)
else()
    add_compile_options(-ffunction-sections -fdata-sections)
    add_link_options(LINKER:--gc-sections)
endif()

# clangd needs compile_commands.json, which the VS generator can't emit. Mirror every VS (re)configure
# into a Ninja configure at build_ninja/ (see .clangd) so the database never goes stale. Only from the
# VS generator, so the nested Ninja configure of this same file can't recurse.
option(SPECTRA_CLANGD_DB "Refresh build_ninja/compile_commands.json on every Visual Studio configure (opt-in, adds ~25 s per reconfigure)" OFF)
if(SPECTRA_CLANGD_DB AND CMAKE_GENERATOR MATCHES "Visual Studio" AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # cl.exe is <VS>/VC/Tools/MSVC/<ver>/bin/...; its environment script is <VS>/VC/Auxiliary/Build/vcvars64.bat.
    string(REGEX REPLACE "/VC/Tools/.*$" "/VC/Auxiliary/Build/vcvars64.bat" _spectra_vcvars "${CMAKE_CXX_COMPILER}")
    if(EXISTS "${_spectra_vcvars}")
        file(TO_NATIVE_PATH "${_spectra_vcvars}" _spectra_vcvars_native)
        file(TO_NATIVE_PATH "${CMAKE_COMMAND}" _spectra_cmake_native)
        set(_spectra_clangd_script "${CMAKE_BINARY_DIR}/refresh_clangd_db.bat")
        file(WRITE "${_spectra_clangd_script}"
            "@call \"${_spectra_vcvars_native}\" >nul 2>&1
"
            "\"${_spectra_cmake_native}\" -S \"${CMAKE_SOURCE_DIR}\" -B \"${CMAKE_SOURCE_DIR}/build_ninja\" -G Ninja"
            " -DCMAKE_BUILD_TYPE=Debug_win64 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
")
        message(STATUS "Refreshing clangd compile database (build_ninja/)")
        execute_process(COMMAND cmd /c "${_spectra_clangd_script}" RESULT_VARIABLE _spectra_clangd_result OUTPUT_QUIET ERROR_QUIET)
        if(NOT _spectra_clangd_result EQUAL 0)
            message(WARNING "clangd compile database refresh failed (${_spectra_clangd_result}); run ${_spectra_clangd_script} to see why.")
        endif()
    else()
        message(WARNING "SPECTRA_CLANGD_DB: vcvars64.bat not found next to ${CMAKE_CXX_COMPILER}; clangd database not refreshed.")
    endif()
endif()
