@echo off
echo ============================================
echo  Archipelago Integration for Selaco (Windows)
echo ============================================
echo.

REM Check if we're in the right directory
if not exist "CMakeLists.txt" (
    echo ERROR: This must be run from your Selaco source root directory!
    echo        The directory containing CMakeLists.txt and src\
    pause
    exit /b 1
)

if not exist "src" (
    echo ERROR: Cannot find src directory!
    pause
    exit /b 1
)

echo This will install Archipelago integration for Selaco.
echo.
set /p continue="Continue? (y/n): "
if /i not "%continue%"=="y" (
    echo Cancelled.
    pause
    exit /b 0
)

echo.
echo Running PowerShell deployment script...
echo.

REM Run the fixed PowerShell script with bypass execution policy
powershell -ExecutionPolicy Bypass -File deploy_archipelago_windows_fixed.ps1

if errorlevel 1 (
    echo.
    echo ============================================
    echo  ERROR: PowerShell script failed!
    echo ============================================
    echo.
    echo If you see an error about scripts being disabled, try:
    echo 1. Right-click deploy_archipelago_windows_fixed.ps1
    echo 2. Select "Run with PowerShell"
    echo.
    echo OR run this command in PowerShell as Administrator:
    echo    Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
    echo.
    pause
    exit /b 1
)

echo.
echo ============================================
echo  Deployment Complete!
echo ============================================
echo.
pause