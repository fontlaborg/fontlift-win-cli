@echo off
REM this_file: scripts/generate-version-rc.cmd
REM Thin wrapper around PowerShell generator to keep compatibility with batch callers.

set "VERSION_INPUT=%~1"
if "%VERSION_INPUT%"=="" (
    echo Error: Version string is required
    echo Usage: generate-version-rc.cmd VERSION
    exit /b 1
)

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0generate-version-rc.ps1" -TargetVersion "%VERSION_INPUT%"
exit /b %ERRORLEVEL%
