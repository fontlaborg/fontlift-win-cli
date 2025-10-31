@echo off
REM this_file: publish.cmd
REM Package fontlift for distribution

echo Creating distribution package...

REM Check if build exists
if not exist build\fontlift.exe (
    echo ERROR: build\fontlift.exe not found
    echo Please run build.cmd first
    exit /b 1
)

REM Create dist directory
if not exist dist mkdir dist
if exist dist\*.* del /Q dist\*.*

REM Copy executable
copy build\fontlift.exe dist\ >nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to copy fontlift.exe
    exit /b 1
)

REM Create distribution README
echo fontlift-win-cli v0.1.0 > dist\README.txt
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

REM Copy license
if exist LICENSE copy LICENSE dist\ >nul

REM Create zip archive (requires PowerShell)
set VERSION=0.1.0
set ZIPFILE=fontlift-v%VERSION%.zip

echo Compressing to %ZIPFILE%...
powershell -Command "Compress-Archive -Path dist\* -DestinationPath dist\%ZIPFILE% -Force"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ===================================
    echo Distribution package created!
    echo Output: dist\%ZIPFILE%
    echo ===================================
    dir dist
    exit /b 0
) else (
    echo ERROR: Failed to create zip archive
    exit /b 1
)
