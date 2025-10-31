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
