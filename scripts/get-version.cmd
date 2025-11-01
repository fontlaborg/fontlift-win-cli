@echo off
REM this_file: scripts/get-version.cmd
REM Resolve semantic version information from git tags or provided input.

set "VERSION_BASE="
set "VERSION_SEMVER="
set "VERSION_TAG="

set "REQUESTED_VERSION=%~1"

if "%REQUESTED_VERSION%"=="" (
    for /f "usebackq tokens=1,2 delims==" %%A in (`powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0get-version.ps1"`) do call :__assign %%A %%B
) else (
    for /f "usebackq tokens=1,2 delims==" %%A in (`powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0get-version.ps1" -TargetVersion "%REQUESTED_VERSION%"`) do call :__assign %%A %%B
)

if not defined VERSION_BASE (
    >&2 echo Error: Failed to resolve version information.
    exit /b 1
)

echo %VERSION_BASE%
exit /b 0

:__assign
if /I "%~1"=="BASE" set "VERSION_BASE=%~2"
if /I "%~1"=="SEMVER" set "VERSION_SEMVER=%~2"
if /I "%~1"=="TAG" set "VERSION_TAG=%~2"
goto :eof
