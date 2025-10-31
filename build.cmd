@echo off
REM this_file: build.cmd
REM Build script for fontlift-win-cli
REM Requires Visual Studio 2017 or later with MSVC compiler

echo Building fontlift.exe...

REM Check if cl.exe is available
where cl.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: MSVC compiler (cl.exe) not found
    echo Please run this from Visual Studio Developer Command Prompt
    echo Or run: "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    exit /b 1
)

REM Create output directory
if not exist build mkdir build

REM Compile with C++17, warnings, optimizations
cl.exe /std:c++17 /EHsc /W4 /O2 ^
    /Febuild\fontlift.exe ^
    src\main.cpp ^
    /link Advapi32.lib Shlwapi.lib User32.lib Gdi32.lib

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ===================================
    echo Build successful!
    echo Output: build\fontlift.exe
    echo ===================================
    exit /b 0
) else (
    echo.
    echo ===================================
    echo Build failed!
    echo ===================================
    exit /b 1
)
