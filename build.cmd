@echo off
REM this_file: build.cmd
REM Build script for fontlift-win-cli
REM Requires Visual Studio 2017 or later with MSVC compiler
REM Usage: build.cmd [version]
REM   version: Optional semantic version (e.g., "1.2.3" or "1.2.3-dev.1")

set "EXIT_CODE=0"
set "SCRIPT_ROOT=%~dp0"
if "%SCRIPT_ROOT%"=="" set "SCRIPT_ROOT=."

pushd "%SCRIPT_ROOT%" >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to change directory to %SCRIPT_ROOT%
    exit /b 1
)

set "REQUESTED_VERSION=%~1"

REM Multi-level fallback version resolution strategy
REM Level 1: Use provided argument
REM Level 2: Resolve from git via get-version.cmd
REM Level 3: Use hardcoded fallback
if not "%REQUESTED_VERSION%"=="" (
    REM Level 1: Version provided as argument
    set "BUILD_SEMVER=%REQUESTED_VERSION%"
    REM Extract base version (before any - or +)
    for /f "tokens=1 delims=-+" %%V in ("%REQUESTED_VERSION%") do set "BUILD_VERSION=%%V"
    set "BUILD_TAG=v%REQUESTED_VERSION%"
    echo Using provided version: %BUILD_SEMVER%
) else (
    REM Level 2: Try to resolve from git
    call scripts\get-version.cmd
    if %ERRORLEVEL% EQU 0 (
        set "BUILD_VERSION=%VERSION_BASE%"
        set "BUILD_SEMVER=%VERSION_SEMVER%"
        set "BUILD_TAG=%VERSION_TAG%"
        echo Resolved version from git: %BUILD_SEMVER%
    ) else (
        REM Level 3: Fallback to hardcoded version
        echo WARNING: Version resolution failed, using fallback version
        set "BUILD_VERSION=0.0.0"
        set "BUILD_SEMVER=0.0.0-fallback"
        set "BUILD_TAG=v0.0.0-fallback"
    )
)

REM Final safety check - ensure variables are set
if "%BUILD_VERSION%"=="" set "BUILD_VERSION=0.0.0"
if "%BUILD_SEMVER%"=="" set "BUILD_SEMVER=0.0.0-fallback"
if "%BUILD_TAG%"=="" set "BUILD_TAG=v0.0.0-fallback"

echo Building fontlift %BUILD_SEMVER%...

where cl.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: MSVC compiler (cl.exe) not found
    echo Please run this from a Visual Studio Developer Command Prompt
    echo Or run: "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    set "EXIT_CODE=1"
    goto :cleanup
)

if not exist build mkdir build

echo Generating version resource...
powershell -NoProfile -ExecutionPolicy Bypass -File "scripts\generate-version-rc.ps1" -TargetVersion "%BUILD_SEMVER%" >nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to generate version resource
    set "EXIT_CODE=1"
    goto :cleanup
)

cl.exe /std:c++17 /EHsc /W4 /O2 ^
    /Fobuild\ ^
    src\main.cpp src\sys_utils.cpp src\font_parser.cpp src\font_ops.cpp src\version.rc ^
    /link /OUT:build\fontlift.exe Advapi32.lib Shlwapi.lib User32.lib Gdi32.lib

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ===================================
    echo Build successful!
    echo Version: %BUILD_SEMVER%
    echo Output: build\fontlift.exe
    echo ===================================
) else (
    echo.
    echo ===================================
    echo Build failed!
    echo ===================================
    set "EXIT_CODE=1"
)

:cleanup
popd >nul 2>&1
exit /b %EXIT_CODE%
