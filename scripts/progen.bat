@echo off
setlocal EnableDelayedExpansion

echo ===================================
echo  CMake Project Generator
echo ===================================
echo.

:loop_start

:: --- Project Name ---
:ask_name
set "PROJECT_NAME="
set /p PROJECT_NAME="Project name (no spaces): "
if "!PROJECT_NAME!"=="" (
    echo Name cannot be empty. Try again.
    goto :ask_name
)

:: --- Project Path ---
set "PROJECT_PATH="
set /p PROJECT_PATH="Project location (Enter for current dir): "
if "!PROJECT_PATH!"=="" set "PROJECT_PATH=%CD%"

:: --- Project Type ---
echo.
echo Select project type:
echo   1. Executable
echo   2. Shared Library (DLL)
echo   3. Static Library (LIB)
echo.
:ask_type
set "BUILD_TYPE="
set /p BUILD_TYPE="Choice (1-3): "
if "!BUILD_TYPE!"=="1" set "BUILD_TYPE_NAME=EXE"
if "!BUILD_TYPE!"=="2" set "BUILD_TYPE_NAME=DLL"
if "!BUILD_TYPE!"=="3" set "BUILD_TYPE_NAME=LIB"
if not defined BUILD_TYPE_NAME (
    echo Invalid choice. Enter 1, 2, or 3.
    goto :ask_type
)

:: --- CXX Standard ---
echo.
echo Select C++ standard:
echo   1. C++17
echo   2. C++20  (default)
echo   3. C++23
echo.
:ask_std
set "CXX_STD="
set /p CXX_STD="Choice (1-3, Enter for C++20): "
if "!CXX_STD!"==""  set "CXX_STD=2"
if "!CXX_STD!"=="1" set "CXX_STD_VAL=17"
if "!CXX_STD!"=="2" set "CXX_STD_VAL=20"
if "!CXX_STD!"=="3" set "CXX_STD_VAL=23"
if not defined CXX_STD_VAL (
    echo Invalid choice. Enter 1, 2, or 3.
    goto :ask_std
)

echo.
echo Generating !BUILD_TYPE_NAME! project "!PROJECT_NAME!" at !PROJECT_PATH!...
echo.

:: --- Directory Structure ---
set "ROOT=!PROJECT_PATH!\!PROJECT_NAME!"
mkdir "!ROOT!"      2>nul || (echo [ERROR] Cannot create project root. & goto :error)
cd /d "!ROOT!"           || (echo [ERROR] Cannot enter project root.  & goto :error)
mkdir "src\Public"  2>nul || (echo [ERROR] Cannot create src\Public.  & goto :error)
mkdir "src\Private" 2>nul || (echo [ERROR] Cannot create src\Private. & goto :error)
mkdir "build"       2>nul || (echo [ERROR] Cannot create build dir.   & goto :error)
mkdir "bin"         2>nul || (echo [ERROR] Cannot create bin dir.     & goto :error)
echo [OK] Directories created.

:: --- Source Files ---
if "!BUILD_TYPE!"=="1" (
    call :write_exe_source
) else if "!BUILD_TYPE!"=="2" (
    call :write_dll_source
) else (
    call :write_lib_source
)
if errorlevel 1 goto :error
echo [OK] Source files written.

:: --- CMakeLists.txt ---
call :write_cmake
if errorlevel 1 goto :error
echo [OK] CMakeLists.txt written.

:: --- .gitignore ---
call :write_gitignore
echo [OK] .gitignore written.

echo.
echo Done! Project ready at: !ROOT!
echo Build with:
echo   cd "!ROOT!\build"
echo   cmake ..
echo   cmake --build . --config Release
echo.

:loop_prompt
set "LOOP_CHOICE="
set /p LOOP_CHOICE="Generate another project? (y/n): "
if /i "!LOOP_CHOICE!"=="y" (
    cd /d "!PROJECT_PATH!"
    echo.
    goto :loop_start
) else if /i "!LOOP_CHOICE!"=="n" (
    goto :end
) else (
    echo Enter y or n.
    goto :loop_prompt
)

:: ============================================================
:: Subroutines
:: ============================================================

:write_exe_source
(
    echo #pragma once
    echo #include ^<iostream^>
    echo #include ^<string^>
    echo #include ^<vector^>
    echo #include ^<memory^>
) > "src\Public\!PROJECT_NAME!.h" || exit /b 1
(
    echo #include "!PROJECT_NAME!.h"
    echo.
    echo int main^(^) {
    echo     std::cout ^<^< "Hello from !PROJECT_NAME!^^n";
    echo     return 0;
    echo }
) > "src\Private\main.cpp" || exit /b 1
exit /b 0

:write_dll_source
(
    echo #pragma once
    echo #include ^<iostream^>
    echo #include ^<string^>
    echo #include ^<vector^>
    echo #include ^<memory^>
    echo.
    echo #ifdef !PROJECT_NAME!_EXPORTS
    echo #  define !PROJECT_NAME!_API __declspec^(dllexport^)
    echo #else
    echo #  define !PROJECT_NAME!_API __declspec^(dllimport^)
    echo #endif
    echo.
    echo !PROJECT_NAME!_API void Init^(^);
) > "src\Public\!PROJECT_NAME!.h" || exit /b 1
(
    echo #include "!PROJECT_NAME!.h"
    echo.
    echo void Init^(^) {
    echo     std::cout ^<^< "!PROJECT_NAME! initialised^^n";
    echo }
) > "src\Private\!PROJECT_NAME!.cpp" || exit /b 1
exit /b 0

:write_lib_source
(
    echo #pragma once
    echo #include ^<iostream^>
    echo #include ^<string^>
    echo #include ^<vector^>
    echo #include ^<memory^>
) > "src\Public\!PROJECT_NAME!.h" || exit /b 1
(
    echo #pragma once
    echo #include "!PROJECT_NAME!.h"
    echo.
    echo class !PROJECT_NAME! {
    echo public:
    echo     void SayHello^(^) const;
    echo };
) > "src\Public\!PROJECT_NAME!.h" || exit /b 1
(
    echo #include "!PROJECT_NAME!.h"
    echo.
    echo void !PROJECT_NAME!::SayHello^(^) const {
    echo     std::cout ^<^< "Hello from !PROJECT_NAME!^^n";
    echo }
) > "src\Private\!PROJECT_NAME!.cpp" || exit /b 1
exit /b 0

:write_cmake
(
    echo cmake_minimum_required^(VERSION 3.20^)
    echo project^(!PROJECT_NAME! LANGUAGES CXX^)
    echo.
    echo set^(CMAKE_CXX_STANDARD !CXX_STD_VAL!^)
    echo set^(CMAKE_CXX_STANDARD_REQUIRED ON^)
    echo set^(CMAKE_CXX_EXTENSIONS OFF^)
    echo set^(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$^<CONFIG^>^)
    echo link_directories^(${CMAKE_SOURCE_DIR}/bin/$^<CONFIG^>^)
    echo.
    echo file^(GLOB_RECURSE !PROJECT_NAME!_HEADERS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h"^)
    echo file^(GLOB_RECURSE !PROJECT_NAME!_SOURCE   CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp"^)
    echo file^(GLOB_RECURSE !PROJECT_NAME!_INL      CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.inl"^)
    echo.
    if "!BUILD_TYPE!"=="1" (
        echo add_executable^(!PROJECT_NAME!
        echo     ${!PROJECT_NAME!_HEADERS}
        echo     ${!PROJECT_NAME!_SOURCE}
        echo     ${!PROJECT_NAME!_INL}
        echo ^)
    ) else if "!BUILD_TYPE!"=="2" (
        echo add_library^(!PROJECT_NAME! SHARED
        echo     ${!PROJECT_NAME!_HEADERS}
        echo     ${!PROJECT_NAME!_SOURCE}
        echo     ${!PROJECT_NAME!_INL}
        echo ^)
        echo target_compile_definitions^(!PROJECT_NAME! PRIVATE !PROJECT_NAME!_EXPORTS^)
    ) else (
        echo add_library^(!PROJECT_NAME! STATIC
        echo     ${!PROJECT_NAME!_HEADERS}
        echo     ${!PROJECT_NAME!_SOURCE}
        echo     ${!PROJECT_NAME!_INL}
        echo ^)
    )
    echo.
    echo target_include_directories^(!PROJECT_NAME! PRIVATE
    echo     ${CMAKE_CURRENT_SOURCE_DIR}/src/Public
    echo ^)
    echo.
    echo target_precompile_headers^(!PROJECT_NAME! PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/!PROJECT_NAME!.h^)
    echo.
    echo if ^(MSVC^)
    echo     target_compile_options^(!PROJECT_NAME! PRIVATE
    echo         /W4
    echo         /permissive-
    echo         /Zc:__cplusplus
    echo         /arch:AVX2
    echo     ^)
    echo else^(^)
    echo     target_compile_options^(!PROJECT_NAME! PRIVATE
    echo         -Wall
    echo         -Wextra
    echo         -Wpedantic
    echo         -mavx2
    echo     ^)
    echo endif^(^)
) > CMakeLists.txt || exit /b 1
exit /b 0

:write_gitignore
(
    echo build/
    echo bin/
    echo .cache/
    echo CMakeFiles/
    echo CMakeCache.txt
    echo cmake_install.cmake
    echo *.pdb
    echo *.ilk
    echo *.exp
) > .gitignore
exit /b 0

:error
echo.
echo [ERROR] Project generation failed. See messages above.
pause
goto :end

:end
echo Goodbye.
pause