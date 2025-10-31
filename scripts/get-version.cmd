@echo off
REM this_file: scripts/get-version.cmd
REM Extract version from git tags or use provided parameter

if "%~1"=="" (
    REM No parameter - get from git tag
    for /f "tokens=*" %%i in ('git describe --tags --abbrev^=0 2^>nul') do set VERSION=%%i
    if "%VERSION%"=="" (
        REM No tags exist - use default development version
        set VERSION=v0.0.0-dev
    )
) else (
    REM Use provided parameter
    set VERSION=%~1
)

REM Strip leading 'v' if present
if "%VERSION:~0,1%"=="v" set VERSION=%VERSION:~1%

REM Output the version (without trailing newline to avoid issues in pipes)
echo|set /p="%VERSION%"
