@echo off
setlocal EnableDelayedExpansion

echo Welcome to the Grand CMake Project-O-Tron 5001: Now With Loops AND Debugging!

:loop_start
echo First, whats this masterpiece gonna be called?
set /p PROJECT_NAME="Enter project name (No blank spaces): "

if "!PROJECT_NAME!"=="" (
    echo Brilliant, no name. Defaulting to "UnnamedDisaster".
    set PROJECT_NAME=UnnamedDisaster
)

echo Now, where do you want this gem to live? (Full path, like C:\Code\Stuff)
set /p PROJECT_PATH="Enter location (or hit Enter for right here): "

if "!PROJECT_PATH!"=="" (
    echo No path? Lazy much? Fine, dropping it in %CD%.
    set PROJECT_PATH=%CD%
)

echo What kind of project are you feeling today, your highness?
echo 1. EXE (Just a "Hello World" because you are basic)
echo 2. DLL (No fancy stuff, per your stingy orders)
echo 3. LIB (A class, because you are suddenly classy)
set /p BUILD_TYPE="Pick a number (1-3): "

if "!BUILD_TYPE!"=="1" set BUILD_TYPE_NAME=EXE
if "!BUILD_TYPE!"=="2" set BUILD_TYPE_NAME=DLL
if "!BUILD_TYPE!"=="3" set BUILD_TYPE_NAME=LIB
if not defined BUILD_TYPE_NAME (
    echo Wow, cannot even pick a number? Defaulting to EXE, you indecisive slacker.
    set BUILD_TYPE=1
    set BUILD_TYPE_NAME=EXE
)

echo Generating your !BUILD_TYPE_NAME! project !PROJECT_NAME! at !PROJECT_PATH!...
echo Step 1: Creating directories...
mkdir "!PROJECT_PATH!\!PROJECT_NAME!" 2>nul || (echo Failed to create root dir! & goto :error)
cd "!PROJECT_PATH!\!PROJECT_NAME!" || (echo Failed to cd into project dir! & goto :error)
mkdir src\Public 2>nul || (echo Failed to create src\Public! & goto :error)
mkdir src\Private 2>nul || (echo Failed to create src\Private! & goto :error)
mkdir build 2>nul || (echo Failed to create build! & goto :error)
echo Directories created. If you’re seeing this, we’re not dead yet.

:: DLL: Header and source from your original, no extras
if "!BUILD_TYPE!"=="2" (
    echo Step 2: Writing DLL files...
    (
        echo #pragma once
        echo #ifndef !PROJECT_NAME!
        echo #define !PROJECT_NAME! __declspec^(dllexport^)
        echo #endif
        echo void !PROJECT_NAME! Init^(^);
    ) > "src\Public\!PROJECT_NAME!.h" || (echo Failed to write header! & goto :error)
    (
        echo #include "!PROJECT_NAME!.h"
        echo void !PROJECT_NAME! Init^(^) {}
    ) > "src\Private\!PROJECT_NAME!.cpp" || (echo Failed to write cpp! & goto :error)
)

:: LIB: Header with a class, source with implementation
if "!BUILD_TYPE!"=="3" (
    echo Step 2: Writing LIB files...
    (
        echo #pragma once
        echo #include ^<iostream^>
        echo class !PROJECT_NAME! {
        echo public:
        echo     void SayHello^(^);
        echo };
    ) > "src\Public\!PROJECT_NAME!.h" || (echo Failed to write header! & goto :error)
    (
        echo #include "!PROJECT_NAME!.h"
        echo void !PROJECT_NAME!::SayHello^(^) {
        echo     std::cout ^<^< "Hello from !PROJECT_NAME! LIB!" ^<^< std::endl;
        echo }
    ) > "src\Private\!PROJECT_NAME!.cpp" || (echo Failed to write cpp! & goto :error)
)

:: EXE: Just a cpp with Hello World
if "!BUILD_TYPE!"=="1" (
    echo Step 2: Writing EXE file...
    (
        echo #include ^<iostream^>
        echo int main^(^) {
        echo     std::cout ^<^< "Hello, World!" ^<^< std::endl;
        echo     return 0;
        echo }
    ) > "src\Private\!PROJECT_NAME!.cpp" || (echo Failed to write cpp! & goto :error)
)

:: Create the CMakeLists.txt
echo Step 3: Writing CMakeLists.txt...
(
    echo cmake_minimum_required^(VERSION 3.20^)
    echo project^(!PROJECT_NAME!^)
    if "!BUILD_TYPE!"=="1" (
        echo add_executable^(!PROJECT_NAME! src/Private/!PROJECT_NAME!.cpp^)
    ) else if "!BUILD_TYPE!"=="2" (
        echo include_directories^(src/Public^)
        echo add_library^(!PROJECT_NAME! SHARED src/Private/!PROJECT_NAME!.cpp src/Public/!PROJECT_NAME!.h^)
    ) else (
        echo include_directories^(src/Public^)
        echo add_library^(!PROJECT_NAME! STATIC src/Private/!PROJECT_NAME!.cpp src/Public/!PROJECT_NAME!.h^)
    )
) > CMakeLists.txt || (echo Failed to write CMakeLists.txt! & goto :error)

echo Ta-da! Your !BUILD_TYPE_NAME! project !PROJECT_NAME! is ready at !PROJECT_PATH!\!PROJECT_NAME!.
echo To build, cd into !PROJECT_PATH!\!PROJECT_NAME!\build and run "cmake -B build . && cmake --build build"

:loop_prompt
echo Want to make another one, you project-hoarding maniac? (y/n)
set /p LOOP_CHOICE="Choice: "
if /i "!LOOP_CHOICE!"=="y" (
    cd "!PROJECT_PATH!"
    echo Back to the grind we go...
    goto :loop_start
) else if /i "!LOOP_CHOICE!"=="n" (
    echo Finally done? Good riddance!
    goto :end
) else (
    echo Y or N, not rocket science. Try again.
    goto :loop_prompt
)

:error
echo Something went wrong, you absolute disaster. Check the errors above and try again.
pause
goto :end

:end
echo Exiting the Project-O-Tron. Don’t trip on your way out!
pause