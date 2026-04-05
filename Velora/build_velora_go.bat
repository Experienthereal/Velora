@echo off
:: ============================================================
::  Velora — Go Installer Builder
::  Compiles the Go version of the compiler + installer
:: ============================================================

setlocal EnableDelayedExpansion

echo.
echo  ==============================================
echo   Building Velorainstaller.exe (Go Implementation)
echo  ==============================================
echo.

:: Ensure in the right directory
cd velora_go

:: Download dependencies (registry manipulation library)
echo [1/2] Fetching dependencies...
go mod tidy
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to fetch Go dependencies. Make sure 'go' is accessible.
    exit /b 1
)

:: Build the executable
echo [2/2] Compiling Velorainstaller.exe...
go build -ldflags="-H windowsgui" -o Velorainstaller.exe .
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Go compilation failed.
    exit /b 1
)

:: Move the built file to the root directory
move /y Velorainstaller.exe ..\Velorainstaller.exe >nul

echo.
echo  ============================================
echo   Velorainstaller.exe built successfully!
echo   Check the current root directory.
echo  ============================================
echo.

endlocal
