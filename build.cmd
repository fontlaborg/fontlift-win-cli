@echo off
REM this_file: build.cmd
REM Build script for fontlift-win-cli
REM Requires Visual Studio 2017 or later with MSVC compiler
REM Usage: build.cmd [version]
REM   version: Optional version string (e.g., "0.1.0"). If not provided, extracted from git tags.

REM Get version from parameter or git tags
set BUILD_VERSION=%~1
if "%BUILD_VERSION%"=="" (
    for /f %%i in ('scripts\get-version.cmd') do set BUILD_VERSION=%%i
)

echo Building fontlift v%BUILD_VERSION%...

REM Check if cl.exe is available
where cl.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: MSVC compiler (cl.exe) not found
    echo Please run this from Visual Studio Developer Command Prompt
    echo Or run: "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    exit /b 1
)

REM Create output directory
if not exist "build" mkdir "build"

REM Generate version resource file
echo Generating version resource...
call scripts\generate-version-rc.cmd %BUILD_VERSION%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to generate version resource
    exit /b 1
)

REM Compile with C++17, warnings, optimizations, including version resource
cl.exe /std:c++17 /EHsc /W4 /O2 ^
    /Fobuild\ ^
    src\main.cpp src\sys_utils.cpp src\font_parser.cpp src\font_ops.cpp src\version.rc ^
    /link /OUT:build\fontlift.exe Advapi32.lib Shlwapi.lib User32.lib Gdi32.lib

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ===================================
    echo Build successful!
    echo Version: %BUILD_VERSION%
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
