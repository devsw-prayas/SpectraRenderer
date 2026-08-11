@echo off
setlocal EnableExtensions EnableDelayedExpansion

echo.
echo ============================================================
echo               CUDA Toolkit Detection ^& Installation
echo ============================================================
echo.

:: ===========================================================
:: Step 1: Check for an NVIDIA GPU
:: ===========================================================
call :CheckNvidiaGpu
if not "!GPU_FOUND!"=="1" (
    echo [ERROR] No NVIDIA GPU detected. CUDA requires an NVIDIA graphics card.
    echo [HINT] If you're sure you have one, check Device Manager.
    pause
    exit /b 1
)
echo [OK] NVIDIA GPU detected.

:: ===========================================================
:: Step 2: Check for an existing CUDA installation
:: ===========================================================
call :CheckCudaInstalled
if "!CUDA_FOUND!"=="1" (
    echo [INFO] CUDA is already installed (version: !CUDA_VERSION!).
    echo.
    set /p "CONTINUE=Proceed with installation anyway? [y/N] "
    if /i not "!CONTINUE!"=="y" (
        echo [INFO] Installation aborted by user.
        pause
        exit /b 0
    )
    echo.
)

:: ===========================================================
:: Step 3: Verify Administrator privileges
:: ===========================================================
call :CheckAdmin
if not "!IS_ADMIN!"=="1" (
    echo [ERROR] Administrator privileges required.
    echo [HINT] Right-click this script and select "Run as administrator".
    pause
    exit /b 1
)

:: ===========================================================
:: Step 4: Create temp directory
:: ===========================================================
set "TEMP_DIR=%TEMP%\CudaSetup"
if not exist "%TEMP_DIR%" mkdir "%TEMP_DIR%"
echo [INFO] Using temp directory: %TEMP_DIR%

:: ===========================================================
:: Step 5: Download the CUDA Toolkit installer
:: ===========================================================
call :DownloadCuda
if not "!DOWNLOAD_OK!"=="1" (
    echo [ERROR] Failed to download the CUDA Toolkit installer.
    pause
    exit /b 1
)

:: ===========================================================
:: Step 6: Install the CUDA Toolkit
:: ===========================================================
call :InstallCuda
if not "!INSTALL_OK!"=="1" (
    echo [ERROR] CUDA Toolkit installation failed.
    echo [HINT] Try updating your NVIDIA drivers first: https://www.nvidia.com/Download/index.aspx
    pause
    exit /b 1
)

:: ===========================================================
:: Step 7: Cleanup and verify
:: ===========================================================
call :Cleanup
call :VerifyInstallation

echo.
echo ============================================================
echo                      Setup Complete
echo ============================================================
pause
exit /b 0

:: ===========================================================
:: Functions
:: ===========================================================

:CheckNvidiaGpu
set "GPU_FOUND=0"
for /f "tokens=*" %%a in ('wmic path win32_VideoController get name 2^>nul') do (
    echo %%a | find "NVIDIA" >nul && set "GPU_FOUND=1"
)
if "!GPU_FOUND!"=="1" goto :eof

dxdiag /t "%TEMP%\dxdiag.txt" >nul
find /i "nvidia" "%TEMP%\dxdiag.txt" >nul && set "GPU_FOUND=1"
del "%TEMP%\dxdiag.txt" >nul 2>&1
goto :eof

:CheckCudaInstalled
set "CUDA_FOUND=0"
set "CUDA_VERSION="

if defined CUDA_PATH (
    set "CUDA_FOUND=1"
    for /f "delims=" %%i in ('dir /b /ad "%CUDA_PATH%\..\v*" 2^>nul') do set "CUDA_VERSION=%%~ni"
    goto :eof
)

reg query "HKLM\SOFTWARE\NVIDIA Corporation\CUDA" /v Version >nul 2>&1 && (
    set "CUDA_FOUND=1"
    for /f "tokens=2*" %%a in ('reg query "HKLM\SOFTWARE\NVIDIA Corporation\CUDA" /v Version 2^>nul ^| find "Version"') do set "CUDA_VERSION=%%b"
    goto :eof
)

where nvcc >nul 2>&1 && (
    set "CUDA_FOUND=1"
    for /f "tokens=3" %%i in ('nvcc --version 2^>nul ^| find "release"') do set "CUDA_VERSION=v%%i"
)
goto :eof

:CheckAdmin
set "IS_ADMIN=0"
net session >nul 2>&1
if "!errorlevel!"=="0" set "IS_ADMIN=1"
goto :eof

:DownloadCuda
set "DOWNLOAD_OK=0"
set "CUDA_URL=https://developer.download.nvidia.com/compute/cuda/13.2.1/local_installers/cuda_13.2.1_windows.exe"
set "CUDA_INSTALLER=%TEMP_DIR%\cuda_13.2.1_windows.exe"

echo [INFO] Downloading CUDA Toolkit 13.2 (~3 GB, this may take a while)...
echo [URL] %CUDA_URL%

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "$ProgressPreference = 'SilentlyContinue'; " ^
    "try { Invoke-WebRequest -Uri '!CUDA_URL!' -OutFile '!CUDA_INSTALLER!' -UseBasicParsing; exit 0 } catch { exit 1 }"

if exist "%CUDA_INSTALLER%" (
    for %%F in ("%CUDA_INSTALLER%") do set "FILE_SIZE=%%~zF"
    if !FILE_SIZE! GTR 1000000 (
        set "DOWNLOAD_OK=1"
        echo [OK] Download complete (!FILE_SIZE! bytes)
    )
)
goto :eof

:InstallCuda
set "INSTALL_OK=0"
echo.
echo [INFO] Installing CUDA Toolkit (silent mode)...
echo [INFO] This may take several minutes.

start /wait "" "%CUDA_INSTALLER%" -s -n
if !errorlevel! neq 0 goto :eof

set "CUDA_PATH=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.2"
setx CUDA_PATH "%CUDA_PATH%" /M >nul
setx PATH "%PATH%;%CUDA_PATH%\bin" /M >nul

set "INSTALL_OK=1"
echo [OK] Installation completed.
goto :eof

:VerifyInstallation
echo.
echo [INFO] Verifying installation...

where nvcc >nul 2>&1
if "!errorlevel!"=="0" (
    echo [OK] nvcc available in PATH.
    nvcc --version | find "release"
) else (
    echo [WARN] nvcc not in PATH. Restart terminal or reboot.
)
goto :eof

:Cleanup
echo.
echo [INFO] Cleaning up...
if exist "%CUDA_INSTALLER%" del /f /q "%CUDA_INSTALLER%" >nul 2>&1
if exist "%TEMP_DIR%" rd /s /q "%TEMP_DIR%" >nul 2>&1
echo [OK] Cleanup complete.
goto :eof
