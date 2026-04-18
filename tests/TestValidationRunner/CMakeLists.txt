cmake_minimum_required(VERSION 3.20)
project(TestValidationRunner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE TestValidationRunner_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE TestValidationRunner_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")
file(GLOB_RECURSE TestValidationRunner_INL      CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl")

add_executable(TestValidationRunner
    ${TestValidationRunner_HEADERS}
    ${TestValidationRunner_SOURCE}
    ${TestValidationRunner_INL}
)

target_link_libraries(TestValidationRunner PRIVATE 
    KerbecsValidation 
    Kerbecs 
    Stratum 
    Hades
)

target_include_directories(TestValidationRunner PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
)

target_compile_options(TestValidationRunner PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:/arch:AVX2 /W4 /permissive->
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-mavx2 -Wall -Wextra -Wpedantic>
)
