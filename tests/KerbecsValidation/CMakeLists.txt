cmake_minimum_required(VERSION 3.20)
project(KerbecsValidation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE KerbecsValidation_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE KerbecsValidation_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE KerbecsValidation_INL      CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")

add_library(KerbecsValidation SHARED
    ${KerbecsValidation_HEADERS}
    ${KerbecsValidation_SOURCE}
    ${KerbecsValidation_INL}
)
target_compile_definitions(KerbecsValidation PRIVATE KerbecsValidation_EXPORTS)

target_link_libraries(KerbecsValidation PUBLIC 
    Kerbecs 
    SpectraValidationKit 
    Stratum 
    Hades
)

target_include_directories(KerbecsValidation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)


target_compile_options(KerbecsValidation PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive- /bigobj>
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)
