# PLAN.md
<!-- this_file: PLAN.md -->

## Project Overview

**fontlift-win-cli** is a minimal C++ Windows CLI tool for system font management. Install fonts persistently, uninstall by path or name, list installed fonts, and optionally remove font files.

## Technical Architecture

### Core Components

1. **Main Entry Point** (`main.cpp`)
   - Command-line argument parsing (no external libs, use `argv`)
   - Command routing to appropriate handler
   - Exit code management

2. **Font Operations Module** (`font_ops.cpp/h`)
   - `ListFonts()` - enumerate Registry entries
   - `InstallFont()` - copy file + add Registry entry + notify system
   - `UninstallFont()` - remove Registry entry + notify system
   - `RemoveFont()` - uninstall + delete file
   - Helper: `NotifyFontChange()` - send WM_FONTCHANGE

3. **Font Parser Module** (`font_parser.cpp/h`)
   - `GetFontName()` - parse TTF/OTF name table
   - `GetFontsInCollection()` - handle TTC/OTC files
   - Minimal parsing: read file header, locate name table, extract family name

4. **System Utilities** (`sys_utils.cpp/h`)
   - `IsAdmin()` - check administrator privileges
   - `CopyToFontsFolder()` - copy font file to %windir%\fonts
   - `DeleteFromFontsFolder()` - delete font file
   - Registry helpers: read/write/delete

### Windows API Dependencies

**Font Management:**
- `AddFontResourceExA()` - load font into session (temporary)
- `RemoveFontResourceExA()` - unload font from session
- `SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0)` - notify all windows

**Registry:**
- Key: `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts`
- Format: `"Font Display Name (TrueType)"="filename.ttf"`
- APIs: `RegOpenKeyExA()`, `RegSetValueExA()`, `RegDeleteValueA()`, `RegEnumValueA()`

**File Operations:**
- `CopyFileA()` - copy font to system folder
- `DeleteFileA()` - remove font file
- `GetWindowsDirectoryA()` - locate %windir%\fonts
- `PathFileExistsA()` - validate paths (from Shlwapi.h)

**Privilege Check:**
- `OpenProcessToken()` + `GetTokenInformation()` - check admin rights

### Font File Format

**TTF/OTF Structure:**
```
Offset Subtable (12 bytes)
  - scaler type (4 bytes)
  - numTables (2 bytes)
  - ...
Table Directory (16 bytes per table)
  - tag (4 bytes, e.g., "name")
  - checkSum, offset, length
Name Table
  - format, count, stringOffset
  - Name records (platform, encoding, language, nameID, length, offset)
  - Strings storage
```

**What We Need:**
- NameID 1: Font Family name
- NameID 4: Full font name
- Prefer Windows platform (ID=3), Unicode encoding (ID=1), English language (ID=0x0409)

**TTC/OTC Collections:**
- Header: "ttcf" tag + version + numFonts
- Array of offsets to individual font tables
- Parse each font separately

## Implementation Phases

### Phase 0: Build Infrastructure & CI/CD ✅ COMPLETED

**Goal:** Implement semantic versioning and automated build/release pipeline.

**Status:** Implemented and fixed. All GitHub Actions workflows are now operational.

**Problem Analysis:**
Current build system has hardcoded version (`v0.1.0` in publish.cmd), manual builds only, no CI/CD, no automated releases. Need git tag-based versioning that works locally and in GitHub Actions, with automated binary releases.

**Solution Architecture:**

1. **Semantic Versioning Infrastructure**
   - Version source of truth: Git tags (format: `vX.Y.Z`, e.g., `v0.1.0`, `v1.2.3`)
   - Version extraction: Batch script that runs `git describe --tags --abbrev=0`
   - Version embedding: Windows resource file (.rc) with VERSIONINFO structure
   - Dynamic generation: Create version.rc during build from git tag

2. **Build Script Enhancement (build.cmd)**
   - Accept optional version parameter: `build.cmd [version]`
   - If version not provided, extract from git tag
   - Generate `src/version.rc` with version metadata
   - Compile version.rc into executable
   - Link resource file to embed version in fontlift.exe
   - Work in both local Windows environment and GitHub Actions

3. **Publish Script Enhancement (publish.cmd)**
   - Accept optional version parameter: `publish.cmd [version]`
   - If version not provided, extract from git tag
   - Use version in zip filename: `fontlift-vX.Y.Z.zip`
   - Include version in dist/README.txt
   - Generate SHA256 checksum file for release artifacts

4. **GitHub Actions Workflows**

   **CI Build Workflow** (`.github/workflows/build.yml`):
   - Trigger: Push to main, pull requests
   - Runner: `windows-latest`
   - Steps:
     1. Checkout code
     2. Setup MSVC (ilammy/msvc-dev-cmd@v1)
     3. Extract version from git tag or use dev version
     4. Run build.cmd
     5. Upload build artifacts (fontlift.exe)
     6. Run basic smoke tests (executable runs, shows help)

   **Release Workflow** (`.github/workflows/release.yml`):
   - Trigger: Push tags matching `v*.*.*`
   - Runner: `windows-latest`
   - Steps:
     1. Checkout code
     2. Setup MSVC
     3. Extract version from tag: `${{ github.ref_name }}`
     4. Run build.cmd with version
     5. Run publish.cmd with version
     6. Generate SHA256 checksums
     7. Create GitHub Release (softprops/action-gh-release@v1)
     8. Upload assets: fontlift-vX.Y.Z.zip, checksums.txt
     9. Auto-generate release notes from commits

**Technical Implementation Details:**

**Version Resource File Template** (`templates/version.rc.template`):
```rc
#include <winver.h>

#define VER_MAJOR @VERSION_MAJOR@
#define VER_MINOR @VERSION_MINOR@
#define VER_PATCH @VERSION_PATCH@
#define VER_BUILD 0

#define VER_FILEVERSION VER_MAJOR,VER_MINOR,VER_PATCH,VER_BUILD
#define VER_PRODUCTVERSION VER_MAJOR,VER_MINOR,VER_PATCH,VER_BUILD

#define VER_FILEVERSION_STR "@VERSION_STRING@\0"
#define VER_PRODUCTVERSION_STR "@VERSION_STRING@\0"

VS_VERSION_INFO VERSIONINFO
FILEVERSION VER_FILEVERSION
PRODUCTVERSION VER_PRODUCTVERSION
FILEFLAGSMASK VS_FFI_FILEFLAGSMASK
FILEFLAGS 0x0L
FILEOS VOS_NT_WINDOWS32
FILETYPE VFT_APP
FILESUBTYPE VFT2_UNKNOWN
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904b0"
        BEGIN
            VALUE "CompanyName", "fontlaborg"
            VALUE "FileDescription", "Windows CLI tool for font installation"
            VALUE "FileVersion", VER_FILEVERSION_STR
            VALUE "InternalName", "fontlift"
            VALUE "LegalCopyright", "Copyright (c) 2025 fontlaborg"
            VALUE "OriginalFilename", "fontlift.exe"
            VALUE "ProductName", "fontlift-win-cli"
            VALUE "ProductVersion", VER_PRODUCTVERSION_STR
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1200
    END
END
```

**Version Extraction Script** (`scripts/get-version.cmd`):
```batch
@echo off
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
echo %VERSION%
```

**Version Resource Generator** (`scripts/generate-version-rc.cmd`):
```batch
@echo off
setlocal enabledelayedexpansion

REM Parse version string (e.g., "1.2.3" -> MAJOR=1, MINOR=2, PATCH=3)
set VERSION_STRING=%~1
for /f "tokens=1-3 delims=." %%a in ("%VERSION_STRING%") do (
    set MAJOR=%%a
    set MINOR=%%b
    set PATCH=%%c
)

REM Handle dev versions (e.g., "0.0.0-dev")
if "!PATCH!"=="" set PATCH=0
for /f "tokens=1 delims=-" %%a in ("!PATCH!") do set PATCH=%%a

REM Read template and substitute variables
set INPUT=templates\version.rc.template
set OUTPUT=src\version.rc

(for /f "delims=" %%i in (!INPUT!) do (
    set LINE=%%i
    set LINE=!LINE:@VERSION_MAJOR@=%MAJOR%!
    set LINE=!LINE:@VERSION_MINOR@=%MINOR%!
    set LINE=!LINE:@VERSION_PATCH@=%PATCH%!
    set LINE=!LINE:@VERSION_STRING@=%VERSION_STRING%!
    echo !LINE!
))>!OUTPUT!

echo Generated version.rc with version %VERSION_STRING%
```

**Updated build.cmd workflow:**
```batch
@echo off
REM Accept optional version parameter
set BUILD_VERSION=%~1

set "SCRIPT_ROOT=%~dp0"
if "%SCRIPT_ROOT%"=="" set "SCRIPT_ROOT=."
pushd "%SCRIPT_ROOT%" >nul 2>&1

set "VERSION_BASE="
set "VERSION_SEMVER="
set "REQUESTED_VERSION=%~1"

if "%REQUESTED_VERSION%"=="" (
    call scripts\get-version.cmd >nul
) else (
    call scripts\get-version.cmd "%REQUESTED_VERSION%" >nul
)
set "BUILD_VERSION=%VERSION_BASE%"
if "%BUILD_VERSION%"=="" exit /b 1
set "BUILD_SEMVER=%VERSION_SEMVER%"
if "%BUILD_SEMVER%"=="" set "BUILD_SEMVER=%BUILD_VERSION%"

echo Building fontlift %BUILD_SEMVER%...

call scripts\generate-version-rc.cmd "%BUILD_SEMVER%" >nul

cl.exe /std:c++17 /EHsc /W4 /O2 ^
    /Fobuild\ ^
    src\main.cpp src\sys_utils.cpp src\font_parser.cpp src\font_ops.cpp src\version.rc ^
    /link /OUT:build\fontlift.exe Advapi32.lib Shlwapi.lib User32.lib Gdi32.lib
```

**GitHub Actions CI Build** (`.github/workflows/build.yml`):
```yaml
name: CI Build

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  build:
    runs-on: windows-latest

    steps:
    - name: Checkout code
      uses: actions/checkout@v4
      with:
        fetch-depth: 0  # Fetch all history for git describe

    - name: Setup MSVC
      uses: ilammy/msvc-dev-cmd@v1

    - name: Resolve version
      id: version
      shell: pwsh
      run: |
        $json = & scripts/get-version.ps1 -Format Json
        $info = $json | ConvertFrom-Json
        "VERSION_BASE=$($info.base)" | Out-File -FilePath $env:GITHUB_ENV -Append -Encoding utf8
        "VERSION_SEMVER=$($info.semver)" | Out-File -FilePath $env:GITHUB_ENV -Append -Encoding utf8
        "VERSION_TAG=$($info.tag)" | Out-File -FilePath $env:GITHUB_ENV -Append -Encoding utf8

    - name: Build
      shell: cmd
      run: build.cmd %VERSION_SEMVER%

    - name: Test executable exists
      shell: cmd
      run: |
        if not exist build\fontlift.exe exit 1
        echo Build successful!

    - name: Test executable runs
      shell: cmd
      run: build\fontlift.exe

    - name: Upload artifact
      uses: actions/upload-artifact@v4
      with:
        name: fontlift-${{ env.VERSION_TAG }}
        path: build/fontlift.exe
        retention-days: 7
```

**GitHub Actions Release** (`.github/workflows/release.yml`):
```yaml
name: Release

on:
  push:
    tags:
      - 'v*.*.*'

jobs:
  release:
    runs-on: windows-latest
    permissions:
      contents: write

    steps:
    - name: Checkout code
      uses: actions/checkout@v4
      with:
        fetch-depth: 0

    - name: Setup MSVC
      uses: ilammy/msvc-dev-cmd@v1

    - name: Resolve version from tag
      id: version
      shell: pwsh
      run: |
        $ref = $env:GITHUB_REF_NAME
        if ($ref.StartsWith('v')) { $ref = $ref.Substring(1) }
        $json = & scripts/get-version.ps1 -Version $ref -Format Json
        $info = $json | ConvertFrom-Json
        "VERSION_BASE=$($info.base)" | Out-File -FilePath $env:GITHUB_ENV -Append -Encoding utf8
        "VERSION_SEMVER=$($info.semver)" | Out-File -FilePath $env:GITHUB_ENV -Append -Encoding utf8
        "VERSION_TAG=$($info.tag)" | Out-File -FilePath $env:GITHUB_ENV -Append -Encoding utf8
        Write-Host "Building release for $($info.semver)"

    - name: Build release
      shell: cmd
      run: build.cmd %VERSION_SEMVER%

    - name: Package release
      shell: cmd
      run: publish.cmd %VERSION_SEMVER%

    - name: Generate checksums
      shell: cmd
      run: |
        cd dist
        certutil -hashfile fontlift-%VERSION_TAG%.zip SHA256 > checksums.txt

    - name: Create GitHub Release
      uses: softprops/action-gh-release@v1
      with:
        files: |
          dist/fontlift-${{ env.VERSION_TAG }}.zip
          dist/checksums.txt
        generate_release_notes: true
        draft: false
        prerelease: false
```

**Tasks:**

1. **Create version infrastructure**
   - [ ] Create `scripts/get-version.cmd` - extract version from git tags
   - [ ] Create `scripts/generate-version-rc.cmd` - generate version.rc from template
   - [ ] Create `templates/version.rc.template` - Windows version resource template
   - [ ] Test version extraction locally with git tag

2. **Update build.cmd**
   - [ ] Add version parameter support
   - [ ] Integrate get-version.cmd call
   - [ ] Integrate generate-version-rc.cmd call
   - [ ] Add version.rc to compilation
   - [ ] Test local build with explicit version
   - [ ] Test local build with git tag version
   - [ ] Verify version appears in executable properties

3. **Update publish.cmd**
   - [ ] Add version parameter support
   - [ ] Replace hardcoded v0.1.0 with dynamic version
   - [ ] Update zip filename to use version variable
   - [ ] Update README.txt to use version variable
   - [ ] Test local publish with version

4. **Create GitHub Actions workflows**
   - [ ] Create `.github/workflows/` directory
   - [ ] Create `build.yml` - CI build workflow
   - [ ] Create `release.yml` - release workflow
   - [ ] Test CI build workflow (push to main)
   - [ ] Create initial git tag: `v0.1.0`
   - [ ] Test release workflow (push tag)
   - [ ] Verify GitHub Release created
   - [ ] Verify artifacts uploaded correctly

5. **Testing & validation**
   - [ ] Test version resource in executable (right-click > Properties)
   - [ ] Test `fontlift.exe` shows version (future: add --version flag)
   - [ ] Test CI build on pull request
   - [ ] Test CI build on push to main
   - [ ] Test release creation with tag `v0.1.0`
   - [ ] Download release artifact and verify
   - [ ] Test version extraction in CI environment
   - [ ] Test version extraction in local environment

6. **Documentation**
   - [ ] Update README.md with build instructions
   - [ ] Document version tagging process
   - [ ] Document release process
   - [ ] Update CONTRIBUTING.md with CI/CD info
   - [ ] Update CHANGELOG.md with v0.1.0 release

**Success Criteria:**
- Git tag `v0.1.0` created
- `build.cmd` extracts version from git tags
- `publish.cmd` creates correctly-named zip file
- Windows executable has embedded version resource
- GitHub Actions CI runs on every push/PR
- GitHub Actions creates release on tag push
- Release includes `fontlift-v0.1.0.zip` and checksums
- Build scripts work identically locally and in CI

**Edge Cases:**
- No git tags exist (use v0.0.0-dev)
- Invalid version format (validate and error)
- CI environment without git history (fetch-depth: 0)
- Existing release for tag (workflow should fail or update)
- Version mismatch between tag and build (use tag as source of truth)

**Post-Implementation Bug Fix (2025-11-01):**
- **Issue:** GitHub Actions workflows were failing with "not was unexpected at this time" error
- **Root Cause:** In `scripts/generate-version-rc.cmd`, incorrect use of `%VAR%` instead of `!VAR!` within `setlocal enabledelayedexpansion` block
- **Solution:** Changed all variable references from `%INPUT%`, `%OUTPUT%`, etc. to `!INPUT!`, `!OUTPUT!`, etc. (lines 38, 39, 44, 51, 53)
- **Impact:** All GitHub Actions workflows (build.yml and release.yml) now pass successfully
- **Verification:** Commits and tag pushes now trigger successful builds and releases

### Phase 1: Foundation (MVP)

**Goal:** Basic project structure and build system.

**Tasks:**
1. Create `src/main.cpp` with command parsing
   - Parse: `fontlift.exe <command> [options]`
   - Commands: list, l, install, i, uninstall, u, remove, rm
   - Options: `-p <path>`, `-n <name>`
   - Show usage on error

2. Create `build.cmd` script
   - Use `cl.exe` (MSVC compiler)
   - Compile with: `/std:c++17 /EHsc /W4 /O2`
   - Link with: `Advapi32.lib Shlwapi.lib User32.lib Gdi32.lib`
   - Output: `fontlift.exe`

3. Create `publish.cmd` script
   - Copy `fontlift.exe` to `dist/` folder
   - Create README.txt with usage
   - Create zip archive: `fontlift-v0.1.0.zip`

**Success Criteria:**
- `build.cmd` produces working `fontlift.exe`
- Shows usage message when run without args
- Basic command parsing works

### Phase 2: List Fonts

**Goal:** Read and display installed fonts from Registry.

**Tasks:**
1. Implement `ListFonts(bool showPaths, bool showNames)`
   - Open Registry key: `HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts`
   - Enumerate all values with `RegEnumValueA()`
   - Parse value names (e.g., "Arial (TrueType)") and data (e.g., "arial.ttf")
   - Output format:
     - `-p` only: full path per line
     - `-n` only: font name per line
     - `-n -p`: "path;name" per line

2. Handle edge cases:
   - Font files outside %windir%\fonts (absolute paths in registry)
   - Missing font files (registry entry but file gone)
   - Permission errors reading registry

**Success Criteria:**
- `fontlift list` displays all installed fonts
- Output matches system fonts visible in Control Panel
- Handles fonts in custom locations

### Phase 3: Install Fonts

**Goal:** Persistently install font files system-wide.

**Tasks:**
1. Implement `InstallFont(const char* filepath)`
   - Check admin privileges with `IsAdmin()` - error if not admin
   - Validate file exists and is readable
   - Parse font file to get internal name (Phase 4 dependency - use filename for now)
   - Copy to `%windir%\fonts\` with `CopyFileA()`
   - Add Registry entry: `"Font Name (TrueType)"="filename.ttf"`
   - Call `AddFontResourceExA()` to load immediately
   - Send `WM_FONTCHANGE` to notify system

2. Handle edge cases:
   - Font already installed (check Registry first)
   - Invalid font file
   - Insufficient permissions
   - Disk space issues

**Success Criteria:**
- Fonts installed with `fontlift install` appear in system immediately
- Fonts persist after reboot
- Clear error messages for common failures

### Phase 4: Font Name Parsing

**Goal:** Extract internal font names from font files.

**Tasks:**
1. Implement minimal TTF/OTF parser in `font_parser.cpp`
   - Read file header and validate format
   - Locate "name" table in table directory
   - Parse name table structure
   - Extract Font Family (nameID=1) or Full Name (nameID=4)
   - Prefer Windows platform, Unicode encoding, English language
   - Convert UTF-16BE strings to ASCII/UTF-8

2. Handle TTC/OTC collections
   - Detect "ttcf" header
   - Parse collection header for font count
   - Extract names for all fonts in collection
   - Return vector of font names

3. Fallback: If parsing fails, use filename without extension

**Success Criteria:**
- Correctly extracts "Arial" from arial.ttf
- Handles multi-font TTC files
- Graceful fallback for parsing errors

### Phase 5: Uninstall Fonts

**Goal:** Remove fonts from system without deleting files.

**Tasks:**
1. Implement `UninstallFontByPath(const char* filepath)`
   - Parse font file to get name
   - Find matching Registry entry
   - Remove Registry entry with `RegDeleteValueA()`
   - Call `RemoveFontResourceExA()` to unload
   - Send `WM_FONTCHANGE` to notify

2. Implement `UninstallFontByName(const char* fontname)`
   - Search Registry for matching name (exact or partial match)
   - If multiple matches, list them and ask user to be more specific
   - Remove Registry entry
   - Unload font and notify

**Success Criteria:**
- `fontlift uninstall -p arial.ttf` removes Arial
- `fontlift uninstall -n Arial` removes Arial
- Font files remain in %windir%\fonts
- Fonts are not visible after uninstall

### Phase 6: Remove Fonts

**Goal:** Uninstall fonts and delete font files.

**Tasks:**
1. Implement `RemoveFontByPath(const char* filepath)`
   - Call `UninstallFontByPath()` first
   - Get full path to font file in %windir%\fonts
   - Delete file with `DeleteFileA()`

2. Implement `RemoveFontByName(const char* fontname)`
   - Find font in Registry
   - Get filename from Registry value
   - Uninstall font
   - Delete file

3. Safety checks:
   - Confirm font is uninstalled before deleting
   - Don't delete fonts outside %windir%\fonts
   - Handle locked files (font in use)

**Success Criteria:**
- `fontlift remove -p arial.ttf` removes font and deletes file
- `fontlift remove -n Arial` works correctly
- Safe error handling for locked files

### Phase 7: Polish & Testing

**Goal:** Robust error handling, testing, and documentation.

**Tasks:**
1. Error handling improvements
   - Detect non-admin execution early
   - Provide helpful error messages
   - Return appropriate exit codes (0=success, 1=error, 2=permission)

2. Testing checklist:
   - Test with .ttf files (TrueType)
   - Test with .otf files (OpenType)
   - Test with .ttc files (TrueType Collection)
   - Test with .otc files (OpenType Collection)
   - Test with filenames containing spaces
   - Test with Unicode paths
   - Test without admin rights
   - Test with already-installed fonts
   - Test with invalid/corrupt font files
   - Test uninstall/remove of system fonts
   - Verify persistence after reboot

3. Documentation:
   - Update README.md with installation instructions
   - Add usage examples
   - Document exit codes
   - Create CHANGELOG.md

**Success Criteria:**
- All test scenarios pass
- Clear, actionable error messages
- Complete documentation

## Technical Decisions

### Why Not Use External Libraries?

**Decision:** Implement minimal font parsing instead of using FreeType.

**Reasoning:**
- FreeType is large (2+ MB) and complex
- We only need font family name, not rendering
- Minimal parser: ~100 lines of code
- Keeps executable small (<100 KB)
- No external dependencies to distribute

**Risk:** May miss edge cases in font parsing
**Mitigation:** Fallback to filename if parsing fails

### Why Single File vs Multiple Modules?

**Decision:** Start with multiple files (main.cpp, font_ops.cpp, font_parser.cpp, sys_utils.cpp).

**Reasoning:**
- Easier to test individual modules
- Clear separation of concerns
- Can merge later if needed

**Risk:** Over-engineering
**Mitigation:** Keep each file under 200 lines; merge if total <400 lines

### Build System: CMake vs batch script?

**Decision:** Start with simple `build.cmd` batch script.

**Reasoning:**
- Single target (fontlift.exe)
- No cross-platform needs
- Simple dependency chain
- Can add CMake later if needed

**Risk:** Hard to maintain for complex builds
**Mitigation:** Add CMake in Phase 8 if project grows

## Dependencies & Constraints

### Windows SDK Requirements
- Windows 7 or later (target OS)
- Windows SDK 10.0 or later (for compilation)
- Visual Studio 2017 or later (for MSVC compiler)

### Required Windows APIs
- `Advapi32.lib` - Registry functions
- `Shlwapi.lib` - Path functions
- `User32.lib` - Window messaging
- `Gdi32.lib` - Font functions

### Code Constraints
- C++17 standard
- Functions: max 20 lines
- Files: max 200 lines
- Indentation: max 3 levels

## Testing Strategy

### Unit Testing
- Manual testing (no test framework for simplicity)
- Create `tests/test_fonts/` with sample fonts
- Test script: `tests/run_tests.cmd`

### Test Cases
1. **List operations:**
   - List on clean system
   - List with many fonts
   - List with custom font locations

2. **Install operations:**
   - Install single TTF
   - Install OTF
   - Install TTC with multiple fonts
   - Install to system without admin (should fail)
   - Install already-installed font

3. **Uninstall operations:**
   - Uninstall by exact path
   - Uninstall by exact name
   - Uninstall by partial name match
   - Uninstall non-existent font

4. **Remove operations:**
   - Remove by path
   - Remove by name
   - Remove system font (should warn)

### Integration Testing
- Install font, reboot, verify persistence
- Install multiple fonts, list, uninstall all
- Test with Windows Font Viewer to verify

## Future Enhancements (Out of Scope for v1.0)

- Batch operations (install multiple fonts from directory)
- Backup/restore font configurations
- Font preview before install
- Cross-platform support (Linux, macOS)
- GUI wrapper

These are explicitly excluded to maintain simplicity and focus.

## Success Metrics

Version 1.0 is complete when:
1. All Phase 1-7 tasks are implemented
2. All test cases pass
3. Documentation is complete
4. Single executable <200 KB
5. No external runtime dependencies
6. Fonts persist after reboot
7. Clean, maintainable code (<1000 total lines)
