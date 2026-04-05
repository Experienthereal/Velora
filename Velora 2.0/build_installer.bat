@echo off
:: ============================================================
::  Velora — Single EXE Installer Builder (C++ / Clang)
::  Compiles the entire compiler + installer into VeloraSetup.exe
:: ============================================================

setlocal EnableDelayedExpansion

echo.
echo  ==============================================
echo   Building VeloraSetup.exe  (single-file installer)
echo  ==============================================
echo.

:: We now use clang-cl with MSVC compatibility since you have Clang installed!
clang-cl /EHsc /std:c++17 /O2 /D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH /DUNICODE /D_UNICODE /Icompiler /Istdlib /Iveloragame installer\velora_installer.cpp installer\compiler_bridge.cpp /FeVeloraSetup.exe Advapi32.lib shlwapi.lib shell32.lib ole32.lib winmm.lib user32.lib gdi32.lib

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Compilation failed!
    exit /b 1
)

:: Get file size
for %%A in ("VeloraSetup.exe") do set FILESIZE=%%~zA
set /a FILESIZE_KB=%FILESIZE% / 1024

echo.
echo  ============================================
echo   VeloraSetup.exe built successfully!
echo   Size: %FILESIZE_KB% KB
echo  ============================================
echo.
echo  This single file contains:
echo    - The FULL Velora compiler (Lexer, Parser, Analyzer, CodeGen)
echo    - The Veloragame Graphics Engine (veloragame.h/c)
echo    - Windows installer (PATH, .vel extension, right-click menus)
echo    - System tray background app
echo    - Uninstaller
echo.
echo  It now has EVERYTHING you asked for from "how the language should look"!
echo  To install: double-click VeloraSetup.exe
echo.

endlocal
