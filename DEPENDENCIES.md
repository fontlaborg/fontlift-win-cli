# DEPENDENCIES.md
<!-- this_file: DEPENDENCIES.md -->

## External Dependencies

fontlift-win-cli is designed to have **zero external runtime dependencies** beyond the Windows operating system itself. All functionality is provided by Windows SDK libraries that are included with the operating system.

## Windows SDK Libraries

The following Windows SDK libraries are linked at compile time:

### Advapi32.lib
**Purpose:** Windows Registry operations

**Functions Used:**
- `RegOpenKeyExA()` - Open registry keys for reading font list
- `RegEnumValueA()` - Enumerate installed fonts from registry
- `RegSetValueExA()` - Add font entries to registry during installation
- `RegDeleteValueA()` - Remove font entries during uninstallation
- `RegCreateKeyExA()` - Create registry keys if needed
- `RegCloseKey()` - Close registry handles

**Why:** Persistent font installation on Windows requires modifying the registry at:
`HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts`

### Shlwapi.lib
**Purpose:** Shell path and file operations

**Functions Used:**
- `PathFileExistsA()` - Validate font file existence before operations
- `PathCombineA()` - Construct full paths to %windir%\fonts directory
- `PathIsRelativeA()` - Determine if paths are relative or absolute

**Why:** Provides safe, tested path manipulation functions that handle edge cases like long paths, spaces, and Unicode characters.

### User32.lib
**Purpose:** Windows messaging and user interface

**Functions Used:**
- `SendMessageA()` - Broadcast WM_FONTCHANGE to all windows
- `HWND_BROADCAST` - Constant for system-wide message broadcasting

**Why:** After installing or removing fonts, Windows applications must be notified to refresh their font lists. The `WM_FONTCHANGE` message ensures all running applications see the new font state without requiring a reboot.

### Gdi32.lib
**Purpose:** Graphics Device Interface for font operations

**Functions Used:**
- `AddFontResourceExA()` - Load font into current session (temporary)
- `RemoveFontResourceExA()` - Unload font from current session
- `FR_PRIVATE` - Flag for private font installation
- `FR_NOT_ENUM` - Flag for non-enumerable fonts

**Why:** These functions provide immediate font availability without a reboot. Combined with registry entries, they enable both immediate and persistent font installation.

## C++ Standard Library

**Compiler:** MSVC (Visual Studio 2017 or later)
**Standard:** C++17

**STL Components Used:**
- `<iostream>` - Console output (std::cout, std::cerr)
- `<cstring>` - String comparison (strcmp)

**Why minimal STL usage:**
- Reduces executable size
- Minimizes runtime dependencies
- Simpler to debug and maintain
- Most operations use Windows API directly

## Build-Time Requirements

### Required Software
- **Operating System:** Windows 7 or later (for development)
- **Compiler:** Microsoft Visual C++ (MSVC) from Visual Studio 2017 or later
- **Windows SDK:** Version 10.0 or later (included with Visual Studio)

### Optional Tools
- **Git:** For version control
- **PowerShell:** For publish.cmd zip creation (included with Windows 7+)

## Why No External Libraries?

**Decision rationale:**

1. **FreeType NOT used** - While FreeType is the standard font rendering library, we only need to extract font names, not render glyphs. A minimal parser (~100 lines) is sufficient and keeps the executable under 100 KB vs 2+ MB with FreeType.

2. **No CLI parsing library** - Simple command-line parsing with strcmp is sufficient for our limited command set. Libraries like Boost.Program_Options or CLI11 would add unnecessary complexity.

3. **No logging framework** - Console output (stdout/stderr) is sufficient for a CLI tool. No need for loguru, spdlog, or similar.

4. **No JSON/TOML/Config parsers** - The tool uses command-line arguments only, no configuration files.

## Future Considerations

If the project grows beyond v1.0, we might consider:

- **CMake** - Currently using batch scripts, might add CMake for better cross-IDE support
- **GoogleTest** - For automated unit testing (currently manual testing only)
- **Minimal TTF parser library** - If edge cases in font parsing become problematic

However, all future dependencies must be:
- Header-only OR statically linked (no DLL dependencies)
- Well-maintained (active development, >200 stars if OSS)
- Documented reasoning in this file

## Verification

To verify dependencies of the compiled executable:
```cmd
dumpbin /DEPENDENTS fontlift.exe
```

Should show only:
- KERNEL32.dll (implicit, always present)
- ADVAPI32.dll
- SHLWAPI.dll
- USER32.dll
- GDI32.dll

All of these are guaranteed to be present on any Windows 7+ system.
