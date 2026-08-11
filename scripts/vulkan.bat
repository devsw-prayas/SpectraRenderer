@echo off
setlocal EnableExtensions EnableDelayedExpansion

echo.
echo ============================================================
echo               Vulkan SDK Detection ^& Installation
echo ============================================================
echo.

:: ===========================================================
:: Step 1: Check if Vulkan SDK already exists
:: ===========================================================
call :CheckVulkanSDK
if "!VK_FOUND!"=="1" (
    echo [INFO] Vulkan SDK is already installed.
    echo [PATH] !VULKAN_SDK!
    goto :Success
)

echo [INFO] Vulkan SDK not found. Proceeding with installation...
echo.

:: ===========================================================
:: Step 2: Verify Administrator privileges
:: ===========================================================
call :CheckAdmin
if not "!IS_ADMIN!"=="1" (
    echo [ERROR] Administrator privileges required.
    echo [HINT] Right-click this script and select "Run as administrator".
    pause
    exit /b 1
)

:: ===========================================================
:: Step 3: Create temp directory
:: ===========================================================
set "TEMP_DIR=%TEMP%\VulkanSetup"
if not exist "%TEMP_DIR%" mkdir "%TEMP_DIR%"
echo [INFO] Using temp directory: %TEMP_DIR%

:: ===========================================================
:: Step 4: Download Vulkan SDK
:: ===========================================================
call :DownloadVulkanSDK
if not "!DOWNLOAD_OK!"=="1" (
    echo [ERROR] Failed to download Vulkan SDK installer.
    pause
    exit /b 1
)

:: ===========================================================
:: Step 5: Install Vulkan SDK
:: ===========================================================
call :InstallVulkanSDK
if not "!INSTALL_OK!"=="1" (
    echo [ERROR] Vulkan SDK installation failed.
    pause
    exit /b 1
)

:: ===========================================================
:: Step 6: Cleanup and verify
:: ===========================================================
call :Cleanup
call :VerifyInstallation

:Success
echo.
echo ============================================================
echo                      Setup Complete
echo ============================================================
pause
exit /b 0

:: ===========================================================
:: Functions
:: ===========================================================

:CheckVulkanSDK
set "VK_FOUND=0"
if not defined VULKAN_SDK goto :eof
if not exist "%VULKAN_SDK%\Include\vulkan\vulkan.h" goto :eof
set "VK_FOUND=1"
goto :eof

:CheckAdmin
set "IS_ADMIN=0"
net session >nul 2>&1
if "!errorlevel!"=="0" set "IS_ADMIN=1"
goto :eof

:DownloadVulkanSDK
set "DOWNLOAD_OK=0"
set "VK_URL=https://sdk.lunarg.com/sdk/download/latest/windows/vulkan-sdk.exe"
set "VK_INSTALLER=%TEMP_DIR%\vulkan-sdk.exe"

echo [INFO] Downloading Vulkan SDK...
echo [URL] %VK_URL%

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "$ProgressPreference = 'SilentlyContinue'; " ^
    "try { Invoke-WebRequest -Uri '!VK_URL!' -OutFile '!VK_INSTALLER!' -UseBasicParsing; exit 0 } catch { exit 1 }"

if exist "%VK_INSTALLER%" (
    for %%F in ("%VK_INSTALLER%") do set "FILE_SIZE=%%~zF"
    if !FILE_SIZE! GTR 1000000 (
        set "DOWNLOAD_OK=1"
        echo [OK] Download complete (!FILE_SIZE! bytes)
    )
)
goto :eof

:InstallVulkanSDK
set "INSTALL_OK=0"
echo.
echo [INFO] Installing Vulkan SDK (silent mode)...
echo [INFO] This may take a few minutes...

start /wait "" "%VK_INSTALLER%" /S

:: Give time for environment variables to be registered
timeout /t 5 /nobreak >nul

:: Refresh environment variables in current session
for /f "tokens=2*" %%A in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v VULKAN_SDK 2^>nul') do (
    set "VULKAN_SDK=%%B"
)

if defined VULKAN_SDK (
    echo [OK] Installation completed.
    set "INSTALL_OK=1"
) else (
    :: Check common installation paths
    for /d %%P in ("C:\VulkanSDK\*") do (
        if exist "%%P\Include\vulkan\vulkan.h" (
            set "VULKAN_SDK=%%P"
            set "INSTALL_OK=1"
            echo [OK] Found Vulkan SDK at: %%P
        )
    )
)
goto :eof

:VerifyInstallation
echo.
echo [INFO] Verifying installation...

if defined VULKAN_SDK (
    echo [SDK] !VULKAN_SDK!
    if exist "!VULKAN_SDK!\Include\vulkan\vulkan.h" (
        echo [OK] Header files present.
    )
)

where vulkaninfo >nul 2>&1
if "!errorlevel!"=="0" (
    echo [OK] vulkaninfo available in PATH.
) else (
    echo [WARN] vulkaninfo not in PATH. Restart terminal or reboot.
)
goto :eof

:Cleanup
echo.
echo [INFO] Cleaning up...
if exist "%VK_INSTALLER%" del /f /q "%VK_INSTALLER%" >nul 2>&1
if exist "%TEMP_DIR%" rd /s /q "%TEMP_DIR%" >nul 2>&1
echo [OK] Cleanup complete.
goto :eof
