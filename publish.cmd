@echo off
REM this_file: publish.cmd
REM Package fontlift for distribution
REM Usage: publish.cmd [version]
REM   version: Optional semantic version string.

REM ULTIMATE FALLBACK - This version is used if ALL other resolution methods fail
REM This ensures the package NEVER fails due to version resolution issues
set "ULTIMATE_FALLBACK_VERSION=0.0.0"
set "ULTIMATE_FALLBACK_SEMVER=0.0.0-unknown"
set "ULTIMATE_FALLBACK_TAG=v0.0.0-unknown"

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
    set "PUBLISH_SEMVER=%REQUESTED_VERSION%"
    REM Extract base version (before any - or +)
    for /f "tokens=1 delims=-+" %%V in ("%REQUESTED_VERSION%") do set "PUBLISH_VERSION=%%V"
    set "VERSION_TAG=v%REQUESTED_VERSION%"
    echo Using provided version: %PUBLISH_SEMVER%
) else (
    REM Level 2: Try to resolve from git
    call scripts\get-version.cmd
    if %ERRORLEVEL% EQU 0 (
        set "PUBLISH_VERSION=%VERSION_BASE%"
        set "PUBLISH_SEMVER=%VERSION_SEMVER%"
        set "VERSION_TAG=%VERSION_TAG%"
        echo Resolved version from git: %PUBLISH_SEMVER%
    ) else (
        REM Level 3: Fallback to hardcoded version
        echo WARNING: Version resolution failed, using fallback version
        set "PUBLISH_VERSION=0.0.0"
        set "PUBLISH_SEMVER=0.0.0-fallback"
        set "VERSION_TAG=v0.0.0-fallback"
    )
)

REM ULTIMATE SAFETY CHECK - ensure variables are ALWAYS set
REM This is the absolute last line of defense against version resolution failures
if "%PUBLISH_VERSION%"=="" (
    echo WARNING: PUBLISH_VERSION is empty, using ULTIMATE_FALLBACK
    set "PUBLISH_VERSION=%ULTIMATE_FALLBACK_VERSION%"
)
if "%PUBLISH_SEMVER%"=="" (
    echo WARNING: PUBLISH_SEMVER is empty, using ULTIMATE_FALLBACK
    set "PUBLISH_SEMVER=%ULTIMATE_FALLBACK_SEMVER%"
)
if "%VERSION_TAG%"=="" (
    echo WARNING: VERSION_TAG is empty, using ULTIMATE_FALLBACK
    set "VERSION_TAG=%ULTIMATE_FALLBACK_TAG%"
)

echo Creating distribution package for version %VERSION_TAG%...

if not exist build\fontlift.exe (
    echo ERROR: build\fontlift.exe not found
    echo Please run build.cmd first
    set "EXIT_CODE=1"
    goto :cleanup
)

if not exist dist (
    mkdir dist
)
if exist dist\*.* del /Q dist\*.*

copy build\fontlift.exe dist\ >nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to copy fontlift.exe
    set "EXIT_CODE=1"
    goto :cleanup
)

echo fontlift-win-cli %VERSION_TAG% > dist\README.txt
echo. >> dist\README.txt
echo Windows CLI tool for font installation/uninstallation >> dist\README.txt
echo. >> dist\README.txt
echo Usage: >> dist\README.txt
echo   fontlift.exe list          List installed fonts >> dist\README.txt
echo   fontlift.exe install font.ttf   Install font >> dist\README.txt
echo   fontlift.exe uninstall -n Arial  Uninstall font >> dist\README.txt
echo   fontlift.exe remove -p font.ttf  Remove font and file >> dist\README.txt
echo. >> dist\README.txt
echo See https://github.com/fontlaborg/fontlift-win-cli for documentation >> dist\README.txt

if exist LICENSE copy LICENSE dist\ >nul

set "ZIPFILE=fontlift-%VERSION_TAG%.zip"

echo Compressing to %ZIPFILE%...
powershell -Command "Compress-Archive -Path dist\* -DestinationPath dist\%ZIPFILE% -Force"

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to create zip archive
    set "EXIT_CODE=1"
    goto :cleanup
)

echo.
echo ===================================
echo Distribution package created!
echo Output: dist\%ZIPFILE%
echo ===================================
dir dist

:cleanup
popd >nul 2>&1
exit /b %EXIT_CODE%
