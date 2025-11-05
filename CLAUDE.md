# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Scope

Windows CLI tool for system font management: install fonts persistently, uninstall by path or name, list installed fonts, remove fonts with file deletion.

## Build Commands

**Primary development commands:**
```cmd
build.cmd [version]     # Build executable (optional semver: "1.2.3" or "1.2.3-dev.1")
publish.cmd             # Create distribution package
```

**Version resolution:**
- Build system uses multi-level fallback: provided argument → git tag resolution → hardcoded fallback
- Version info extracted via `scripts/get-version.ps1` from git tags
- Fallback version: `0.0.0-fallback` ensures build never fails on version resolution

**Build requirements:**
- Visual Studio 2017+ with MSVC compiler
- Run from Visual Studio Developer Command Prompt, or initialize with:
  ```cmd
  C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat
  ```

## Architecture

### Module Structure (4 core modules, ~1,245 lines total)

**`src/main.cpp`** - CLI interface and command routing
- Command parsing: `list|l`, `install|i`, `uninstall|u`, `remove|rm`
- Flag handling: `-p` (path), `-n` (name), `-s` (sort), `--admin/-a` (force system-level)
- Version extraction from embedded resources using `VS_FIXEDFILEINFO`
- Exit codes: 0 (success), 1 (error), 2 (permission denied)

**`src/font_ops.cpp/.h`** - Public API for font operations
- `ListFonts(showPaths, showNames, sorted)` - Registry enumeration with sorting/dedup
- `InstallFont(fontPath, forceAdmin)` - Install with system/user detection
- `UninstallFontBy{Path|Name}(target, forceAdmin)` - Remove from registry, keep file
- `RemoveFontBy{Path|Name}(target, forceAdmin)` - Uninstall + delete file
- Uses static `g_listContext` struct for Windows registry callback state management

**`src/font_parser.cpp/.h`** - Font file parsing (TTF/OTF/TTC/OTC)
- `GetFontName(fontPath)` - Extract font family name from name table
- `GetFontsInCollection(fontPath)` - Handle TTC/OTC multi-font files
- `IsCollection(fontPath)` - Detect collection format by magic number
- Implements minimal OpenType/TrueType parsing (no external dependencies)

**`src/sys_utils.cpp/.h`** - Windows API wrappers
- Admin detection: `IsAdmin()` via `CheckTokenMembership`
- Directory paths: `GetFontsDirectory()` (system), `GetUserFontsDirectory()` (user)
- File operations: `CopyToFontsFolder()`, `DeleteFromFontsFolder()`, validation
- Registry: `RegReadFontEntry()`, `RegWriteFontEntry()`, `RegDeleteFontEntry()`, `RegEnumerateFonts()`
  - System registry: `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts`
  - User registry: `HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts`
- System notification: `NotifyFontChange()` via `SendMessage(HWND_BROADCAST, WM_FONTCHANGE)`

**`src/exit_codes.h`** - Exit code constants
- `EXIT_SUCCESS_CODE = 0`, `EXIT_ERROR = 1`, `EXIT_PERMISSION_DENIED = 2`

### Key Design Patterns

**Dual-registry support:**
- System fonts (admin): Install to `C:\Windows\Fonts`, write to `HKLM` registry
- User fonts (no admin): Install to `%LOCALAPPDATA%\Microsoft\Windows\Fonts`, write to `HKCU` registry
- `forceAdmin` flag: Force system-level operations (requires admin privileges)

**Permission handling:**
- `IsAdmin()` check determines default installation scope
- Exit code 2 signals permission denied, prompting user to elevate
- Clear error messages guide users: "Run Command Prompt as Administrator"

**Font file parsing:**
- Reads OpenType/TrueType name table (table tag 'name', name ID 1 = font family)
- Handles Big Endian 16-bit encoding for font metadata
- TTC/OTC collections: Parses offset table to extract individual fonts

**Registry enumeration:**
- Uses Windows `RegEnumValue` with callback pattern
- Static context struct (`g_listContext`) bridges C-style callbacks with C++ state
- Supports sorting and deduplication via `std::set<std::string>`

## CI/CD

**GitHub Actions workflows:**
- `.github/workflows/build.yml` - CI build on push/PR
  - Runs `build.cmd` with git-resolved version
  - Tests: executable exists, runs (shows usage), artifact upload
- `.github/workflows/release.yml` - Release build on tag push
  - Creates distribution package via `publish.cmd`
  - Attaches zip to GitHub release

**Version resource generation:**
- `scripts/generate-version-rc.ps1` - Creates `src/version.rc` with embedded version metadata
- Compiled into executable via `rc.exe`, extracted at runtime by `main.cpp`

## Code Quality Standards (Per CHANGELOG.md Round 26)

**Current metrics:**
- 1,245 lines total (69 improvements across 16 quality rounds)
- Functions: <20 lines each
- Files: ~200-350 lines each (within acceptable range for module complexity)
- Indentation: Max 3 levels
- Constants: All magic numbers extracted (e.g., `REGISTRY_BUFFER_SIZE = 512`)

**Documentation standards:**
- All static helpers have `// Helper:` comments explaining purpose
- Complex patterns (e.g., global callback state) have architectural explanations
- Error messages use consistent prefixes: `Error:`, `Warning:`, `Solution:`, `Note:`

## Testing Strategy

**Manual testing approach:**
- No automated unit tests (manual validation with real fonts)
- Test formats: TTF, OTF, TTC, OTC
- Test scenarios: system vs user install, with/without admin, font collections
- Edge cases: spaces in filenames, Unicode paths, invalid files, post-reboot persistence

**Validation checklist:**
- CI tests: executable builds, runs, shows usage
- File size: 50-500 KB (CI warns if outside range)
- Real-world usage: Install/uninstall/list with actual font files

## Common Development Tasks

**Adding new command:**
1. Add handler function in `main.cpp` (follow `HandleXxxCommand` pattern)
2. Update `ShowUsage()` with command documentation
3. Route command in `main()` switch statement
4. Implement operation in `font_ops.cpp` if Windows API interaction needed
5. Test with real fonts on Windows

**Modifying registry operations:**
- All registry functions in `sys_utils.cpp`: `Reg{Read|Write|Delete|Enumerate}FontEntry`
- Always support both `perUser` (HKCU) and system (HKLM) paths
- Use `REGISTRY_BUFFER_SIZE` constant for buffer allocation
- Call `NotifyFontChange()` after registry modifications

**Extending font format support:**
- Add parsing logic to `font_parser.cpp`
- Update `GetFontName()` or `GetFontsInCollection()` for new format detection
- Follow existing OpenType/TrueType parsing patterns (Big Endian 16-bit)

## Windows API Specifics

**Font installation:**
- `AddFontResourceEx(path, FR_PRIVATE, 0)` - Temporary installation (process lifetime)
- Registry write + `NotifyFontChange()` - Persistent installation (survives reboot)

**Admin privilege detection:**
- `OpenProcessToken()` + `CheckTokenMembership()` against `SECURITY_NT_AUTHORITY`
- Required for system font operations (C:\Windows\Fonts, HKLM registry)

**Font directories:**
- System: `SHGetFolderPathA(CSIDL_FONTS)` → `C:\Windows\Fonts`
- User: `SHGetFolderPathA(CSIDL_LOCAL_APPDATA)` + `\Microsoft\Windows\Fonts`

**Resource compilation:**
- `version.rc.template` → `generate-version-rc.ps1` → `version.rc` → `rc.exe` → `version.res`
- Linked into executable via `/link build\version.res`

## Dependencies

**Windows SDK only:**
- Advapi32.lib - Registry and security APIs
- Shlwapi.lib - Shell path utilities
- User32.lib - Window messaging (WM_FONTCHANGE)
- Gdi32.lib - Font resource management

**No external libraries:**
- Font parsing: Custom OpenType/TrueType implementation
- No FreeType, no Boost, no STL extensions beyond standard C++17

## Anti-Bloat Principles

This is a simple utility tool. The following are explicitly excluded:
- No configuration files (command-line only)
- No logging framework (stderr for errors, stdout for output)
- No auto-update, analytics, metrics, telemetry
- No performance monitoring or benchmarking
- No retry logic or sophisticated error recovery (fail fast with clear messages)
- No abstractions for "future flexibility"

## Exit Codes Reference

- **0** - Success
- **1** - General error (file not found, invalid font, operation failed)
- **2** - Permission denied (need Administrator for system fonts)

## File Path Tracking

All source files include `// this_file: path/to/file.ext` near the top (after copyright header).

## Notes for Future Instances

- **Version system:** Never break version resolution—fallback ensures builds always succeed
- **Registry operations:** Always test both system (HKLM) and user (HKCU) paths
- **Error messages:** Maintain consistency with existing format (prefix + guidance)
- **Code size:** Keep functions <20 lines, files <200 lines (current state exceeds due to complexity, but new code must follow)
- **Testing:** Always test with real fonts on Windows before considering changes complete
- **Documentation:** Update WORK.md with test results, CHANGELOG.md with changes
