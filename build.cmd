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

set "VERSION_BASE="
set "VERSION_SEMVER="
set "VERSION_TAG="
set "REQUESTED_VERSION=%~1"

if "%REQUESTED_VERSION%"=="" (
    call scripts\get-version.cmd >nul
) else (
    call scripts\get-version.cmd "%REQUESTED_VERSION%" >nul
)
if %ERRORLEVEL% NEQ 0 (
    set "EXIT_CODE=%ERRORLEVEL%"
    goto :cleanup
)

set "BUILD_VERSION=%VERSION_BASE%"
if "%BUILD_VERSION%"=="" (
    echo ERROR: Version resolution returned empty base value.
    set "EXIT_CODE=1"
    goto :cleanup
)

set "BUILD_SEMVER=%VERSION_SEMVER%"
if "%BUILD_SEMVER%"=="" set "BUILD_SEMVER=%BUILD_VERSION%"
set "BUILD_TAG=%VERSION_TAG%"
if "%BUILD_TAG%"=="" set "BUILD_TAG=v%BUILD_SEMVER%"

echo Building fontlift %BUILD_SEMVER%...

where cl.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: MSVC compiler (cl.exe) not found
    echo Please run this from a Visual Studio Developer Command Prompt
    echo Or run: "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    set "EXIT_CODE=1"
    goto :cleanup
)

if not exist "build" mkdir "build"

echo Generating version resource...
call scripts\generate-version-rc.cmd "%BUILD_SEMVER%" >nul
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
