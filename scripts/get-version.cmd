@echo off
REM this_file: scripts/get-version.cmd
REM Extract version from git tags or use provided parameter

if "%~1"=="" (
    REM No parameter - get from git tag
    for /f "tokens=*" %%i in ('git describe --tags --abbrev^=0 2^>nul') do set VERSION=%%i
    if "%VERSION%"=="" (
        REM No tags exist - use default development version
        set VERSION=v0.0.0
    )
) else (
    REM Use provided parameter
    set VERSION=%~1
)

REM Strip leading 'v' if present
if "%VERSION:~0,1%"=="v" set VERSION=%VERSION:~1%

REM Strip any suffix after hyphen (e.g., "1.2.3-dev" -> "1.2.3")
REM This avoids batch script issues with hyphens in version strings
for /f "tokens=1 delims=-" %%a in ("%VERSION%") do set VERSION=%%a

REM Output the version
echo %VERSION%
