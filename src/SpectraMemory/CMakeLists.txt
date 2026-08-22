cmake_minimum_required(VERSION 3.20)
project(SpectraMemory LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SpectraMemory_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SpectraMemory_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE SpectraMemory_INL      CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")
file(GLOB_RECURSE SpectraMemory_INTERNAL_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Internal/*.h")

add_library(SpectraMemory SHARED
    ${SpectraMemory_HEADERS}
    ${SpectraMemory_SOURCE}
    ${SpectraMemory_INL}
    ${SpectraMemory_INTERNAL_HEADERS}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${SpectraMemory_HEADERS}
    ${SpectraMemory_SOURCE}
    ${SpectraMemory_INL}
    ${SpectraMemory_INTERNAL_HEADERS}
)

target_compile_definitions(SpectraMemory PRIVATE
    SPECTRA_MEMORY_SHARED
    SPECTRA_MEMORY_EXPORTS
)

target_include_directories(SpectraMemory
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Internal
)

target_precompile_headers(SpectraMemory PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/SpectraMemory.h)

target_compile_options(SpectraMemory PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)

target_link_libraries(SpectraMemory SpectraPlatformRuntime)

# +---------------------------------------------------------------------------+
# All sizes are mentioned in GiB, unless stated
# +---------------------------------------------------------------------------+

target_compile_definitions(SpectraMemory PRIVATE SPECTRA_VA_PER_NODE_GIB=256)
target_compile_definitions(SpectraMemory PRIVATE SPECTRA_VA_MAX_NUMA_NODES=4)

target_compile_definitions(SpectraMemory PUBLIC SPEC_MEM_PER_THREAD_TLS_MIN_SIZE=2)         # In MiB
target_compile_definitions(SpectraMemory PUBLIC SPEC_MEM_PER_THREAD_TLS_MAX_SIZE=100)       # In MiB

# +---------------------------------------------------------------------------+
# Safe memory reclamation limits (Hazard Pointers / EBR)
# +---------------------------------------------------------------------------

target_compile_definitions(SpectraMemory PUBLIC SPECTRA_MAX_EBR_INSTANCES=32)
target_compile_definitions(SpectraMemory PUBLIC SPECTRA_MAX_EBR_PARTICIPANTS=64)
target_compile_definitions(SpectraMemory PUBLIC SPECTRA_MAX_HAZARD_PTRS_PER_THREAD=16)
target_compile_definitions(SpectraMemory PUBLIC SPECTRA_MAX_RETIRED_PER_THREAD=256)
target_compile_definitions(SpectraMemory PUBLIC SPECTRA_MAX_SHARED_CTRL_BLOCKS=1048576)


