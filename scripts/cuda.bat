@echo off
setlocal EnableDelayedExpansion

echo Let's get CUDA installed so your render engine can flex on the GPU...

:: Step 0: Check for NVIDIA GPU (Because CUDA Needs One)
echo Checking for NVIDIA GPU... (crossing fingers)
set "NVIDIA_FOUND=false"

:: Method 1: Check device manager via WMIC
for /f "tokens=*" %%a in ('wmic path win32_VideoController get name 2^>nul') do (
    echo %%a | find "NVIDIA" >nul && set "NVIDIA_FOUND=true"
)

:: Method 2: Check via dxdiag (more reliable)
if "%NVIDIA_FOUND%"=="false" (
    dxdiag /t %TEMP%\dxdiag.txt >nul
    find /i "nvidia" %TEMP%\dxdiag.txt >nul && set "NVIDIA_FOUND=true"
    del %TEMP%\dxdiag.txt >nul 2>&1
)

if "%NVIDIA_FOUND%"=="false" (
    echo ERROR: No NVIDIA GPU detected! CUDA requires an NVIDIA graphics card.
    echo Your system might have Intel/AMD graphics only.
    echo If you're sure you have an NVIDIA GPU, check Device Manager.
    pause
    exit /b 1
)

:: Step 0.5: Check if CUDA is already installed
echo Checking for existing CUDA installation...
set "CUDA_INSTALLED=false"
set "CUDA_VERSION="

:: Check through environment variables
if defined CUDA_PATH (
    set "CUDA_INSTALLED=true"
    for /f "delims=" %%i in ('dir /b /ad "%CUDA_PATH%\..\v*" 2^>nul') do (
        set "CUDA_VERSION=%%~ni"
    )
)

:: Check through registry
if "%CUDA_INSTALLED%"=="false" (
    reg query "HKLM\SOFTWARE\NVIDIA Corporation\CUDA" /v Version 2>nul >nul && (
        set "CUDA_INSTALLED=true"
        for /f "tokens=2*" %%a in ('reg query "HKLM\SOFTWARE\NVIDIA Corporation\CUDA" /v Version 2^>nul ^| find "Version"') do (
            set "CUDA_VERSION=%%b"
        )
    )
)

:: Check through nvcc
if "%CUDA_INSTALLED%"=="false" (
    where nvcc >nul 2>&1 && (
        set "CUDA_INSTALLED=true"
        for /f "tokens=3" %%i in ('nvcc --version 2^>nul ^| find "release"') do (
            set "CUDA_VERSION=v%%i"
        )
    )
)

if "%CUDA_INSTALLED%"=="true" (
    echo WARNING: CUDA is already installed (Version: %CUDA_VERSION%)
    echo.
    set /p "CONTINUE=Do you want to proceed with installation anyway? [y/N] "
    if /i not "!CONTINUE!"=="y" (
        echo Installation aborted by user.
        pause
        exit /b 0
    )
    echo Proceeding with installation...
    echo.
)

:: Step 1: Check for Admin Rights (Because CUDA Doesn't Play Nice Without Them)
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: You need to run this script as Administrator, you peasant!
    echo Right-click this .bat file and select "Run as Administrator".
    pause
    exit /b 1
)

:: Step 2: Create a Temp Directory for the Download
set "TEMP_DIR=C:\Temp\CUDA"
if not exist "%TEMP_DIR%" (
    echo Creating temp directory at %TEMP_DIR% - because I'm nice like that...
    mkdir "%TEMP_DIR%"
    if !errorlevel! neq 0 (
        echo ERROR: Couldn't create temp directory - your drive's probably a mess!
        pause
        exit /b 1
    )
)

:: Step 3: Download CUDA Toolkit 13.2 (Windows, x86_64)
:: URL might change - check NVIDIA's site if this breaks (https://developer.nvidia.com/cuda-downloads)
set "CUDA_URL=https://developer.download.nvidia.com/compute/cuda/13.2.1/local_installers/cuda_13.2.1_windows.exe"
set "CUDA_INSTALLER=%TEMP_DIR%\cuda_13.2.1_windows.exe"
echo Downloading CUDA Toolkit 13.2 - this is ~3 GB, so grab a coffee if your internet's powered by hamsters...
powershell -Command "Invoke-WebRequest -Uri '%CUDA_URL%' -OutFile '%CUDA_INSTALLER%'"
if not exist "%CUDA_INSTALLER%" (
    echo ERROR: Download failed - either NVIDIA moved the file or your internet's a potato!
    pause
    exit /b 1
)

:: Step 4: Silently Install CUDA
echo Installing CUDA Toolkit 13.2 - sit tight, this might take a minute...
start /wait "" "%CUDA_INSTALLER%" -s -n
if !errorlevel! neq 0 (
    echo ERROR: CUDA installation failed - maybe your GPU driver is outdated?
    echo Try updating your NVIDIA drivers first from https://www.nvidia.com/Download/index.aspx
    pause
    exit /b 1
)

:: Step 5: Set Environment Variables
echo Setting up environment variables - because CMake won't find CUDA without a map...
set "CUDA_PATH=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.2"
setx CUDA_PATH "%CUDA_PATH%" /M
setx PATH "%PATH%;%CUDA_PATH%\bin" /M
if !errorlevel! neq 0 (
    echo WARNING: Couldn't set environment variables - you might need to do this manually!
)

:: Step 6: Verify Installation
echo Verifying CUDA installation - let's see if this actually worked...
where nvcc >nul 2>&1
if !errorlevel! neq 0 (
    echo ERROR: CUDA's not in PATH - something's borked! Try rebooting or check your PATH.
    pause
    exit /b 1
)
nvcc --version
if !errorlevel! neq 0 (
    echo ERROR: CUDA's installed, but nvcc isn't happy - maybe a reboot's needed?
    pause
    exit /b 1
)

:: Step 7: Clean Up
echo Cleaning up - because I'm not a slob...
del "%CUDA_INSTALLER%"
if exist "%CUDA_INSTALLER%" (
    echo WARNING: Couldn't delete the installer - clean up %TEMP_DIR% yourself, lazybones!
)

:: Step 8: Victory Lap
echo SUCCESS: CUDA 13.2 is installed and ready to roll!
echo Your NVIDIA GPU is now CUDA-enabled. Check your exact model with:
echo nvidia-smi
echo Current CUDA version: 
nvcc --version | find "release"
echo.
echo You might need to reboot for PATH changes to take effect - Windows loves a good nap.
pause
exit /b 0