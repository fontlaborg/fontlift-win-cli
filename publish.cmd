@echo off
REM this_file: publish.cmd
REM Package fontlift for distribution
REM Usage: publish.cmd [version]
REM   version: Optional semantic version string.

set "EXIT_CODE=0"
set "SCRIPT_ROOT=%~dp0"
if "%SCRIPT_ROOT%"=="" set "SCRIPT_ROOT=."

pushd "%SCRIPT_ROOT%" >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to change directory to %SCRIPT_ROOT%
    exit /b 1
)

set "REQUESTED_VERSION=%~1"

REM If version provided as argument, use it directly
REM Otherwise, resolve from git
if not "%REQUESTED_VERSION%"=="" (
    set "PUBLISH_SEMVER=%REQUESTED_VERSION%"
    REM Extract base version (before any - or +)
    for /f "tokens=1 delims=-+" %%V in ("%REQUESTED_VERSION%") do set "PUBLISH_VERSION=%%V"
    set "VERSION_TAG=v%REQUESTED_VERSION%"
) else (
    REM No argument provided - resolve from git
    call scripts\get-version.cmd
    if %ERRORLEVEL% NEQ 0 (
        set "EXIT_CODE=%ERRORLEVEL%"
        goto :cleanup
    )
    set "PUBLISH_VERSION=%VERSION_BASE%"
    set "PUBLISH_SEMVER=%VERSION_SEMVER%"
    set "VERSION_TAG=%VERSION_TAG%"
)

if "%PUBLISH_VERSION%"=="" (
    echo ERROR: Version resolution returned empty base value.
    set "EXIT_CODE=1"
    goto :cleanup
)

if "%PUBLISH_SEMVER%"=="" set "PUBLISH_SEMVER=%PUBLISH_VERSION%"
if "%VERSION_TAG%"=="" set "VERSION_TAG=v%PUBLISH_SEMVER%"

echo Creating distribution package for version %VERSION_TAG%...

if not exist build\fontlift.exe (
    echo ERROR: build\fontlift.exe not found
    echo Please run build.cmd first
    set "EXIT_CODE=1"
    goto :cleanup
)

if not exist dist mkdir dist
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
