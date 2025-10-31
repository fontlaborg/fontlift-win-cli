@echo off
REM this_file: scripts/generate-version-rc.cmd
REM Generate version.rc from template by substituting version placeholders

REM Parse version string (e.g., "1.2.3" -> MAJOR=1, MINOR=2, PATCH=3)
set "VERSION_STRING=%~1"

REM Handle empty version
if "%VERSION_STRING%"=="" (
    echo Error: Version string is required
    echo Usage: generate-version-rc.cmd VERSION_STRING
    exit /b 1
)

REM Parse version components - do this WITHOUT delayed expansion first
for /f "tokens=1-3 delims=." %%a in ("%VERSION_STRING%") do (
    set "MAJOR=%%a"
    set "MINOR=%%b"
    set "PATCH=%%c"
)

REM Handle dev versions (e.g., "0.0.0-dev" -> extract just "0" from "0-dev")
REM Need delayed expansion for this part
setlocal enabledelayedexpansion
if "!PATCH!"=="" set "PATCH=0"
for /f "tokens=1 delims=-" %%a in ("!PATCH!") do set "PATCH=%%a"

REM Set defaults if parsing failed
if "!MAJOR!"=="" set "MAJOR=0"
if "!MINOR!"=="" set "MINOR=0"
if "!PATCH!"=="" set "PATCH=0"

REM Define paths
set "INPUT=templates\version.rc.template"
set "OUTPUT=src\version.rc"

REM Check if template exists
if not exist "!INPUT!" (
    echo Error: Template file not found: !INPUT!
    exit /b 1
)

REM Generate version.rc by substituting placeholders
(for /f "usebackq delims=" %%i in ("!INPUT!") do (
    set "LINE=%%i"
    set "LINE=!LINE:@VERSION_MAJOR@=!MAJOR!!"
    set "LINE=!LINE:@VERSION_MINOR@=!MINOR!!"
    set "LINE=!LINE:@VERSION_PATCH@=!PATCH!!"
    set "LINE=!LINE:@VERSION_STRING@=!VERSION_STRING!!"
    echo !LINE!
))>"!OUTPUT!"

echo Generated !OUTPUT! with version !VERSION_STRING! (MAJOR=!MAJOR!, MINOR=!MINOR!, PATCH=!PATCH!)
endlocal
