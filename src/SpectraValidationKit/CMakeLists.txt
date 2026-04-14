cmake_minimum_required(VERSION 3.20)
project(SpectraValidationKit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SVK_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE SVK_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")

add_library(SpectraValidationKit SHARED
    ${SVK_HEADERS}
    ${SVK_SOURCE}
)

target_compile_definitions(SpectraValidationKit PRIVATE
    SVK_SHARED
    SVK_BUILDING_RUNTIME
)

target_include_directories(SpectraValidationKit PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

# SpectraValidationKit depends on Stratum for diagnostics and Hades for execution context
target_link_libraries(SpectraValidationKit PUBLIC Stratum Hades)

target_compile_options(SpectraValidationKit PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)

set_target_properties(SpectraValidationKit PROPERTIES FOLDER "Tools")
