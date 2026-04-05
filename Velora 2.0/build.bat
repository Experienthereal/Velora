@echo off
:: Velora Build Script for Windows
:: Builds the vlc compiler using CMake + Ninja or MSBuild

setlocal EnableDelayedExpansion

set BUILD_DIR=build
set BUILD_TYPE=Release

echo.
echo  ==========================================
echo   Velora Programming Language - Build
echo  ==========================================
echo.

:: Check for CMake
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake not found. Please install CMake from https://cmake.org
    exit /b 1
)

:: Check for a C++ compiler
where g++ >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    set GENERATOR=MinGW Makefiles
    set MAKE_CMD=make
    echo [INFO] Using GCC/MinGW compiler
) else (
    where cl >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        set GENERATOR=Visual Studio 17 2022
        echo [INFO] Using MSVC compiler
    ) else (
        echo [ERROR] No C++ compiler found. Install GCC (MinGW) or Visual Studio.
        exit /b 1
    )
)

:: Create build directory
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

:: Configure
echo [1/3] Configuring...
cmake -B %BUILD_DIR% -G "%GENERATOR%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% . >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed.
    exit /b 1
)

:: Build
echo [2/3] Building...
cmake --build %BUILD_DIR% --config %BUILD_TYPE%
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed.
    exit /b 1
)

:: Find the built binary
set VLC_PATH=%BUILD_DIR%\velora.exe
if not exist %VLC_PATH% set VLC_PATH=%BUILD_DIR%\Release\velora.exe
if not exist %VLC_PATH% set VLC_PATH=%BUILD_DIR%\%BUILD_TYPE%\velora.exe

echo [3/3] Copying to project root...
if exist %VLC_PATH% (
    copy /Y %VLC_PATH% velora.exe >nul
    echo.
    echo  Build successful!
    echo  Run Velora with: velora.exe run examples\hello.vel
    echo.
) else (
    echo [WARN] Could not find built binary. Check %BUILD_DIR% manually.
)

endlocal
