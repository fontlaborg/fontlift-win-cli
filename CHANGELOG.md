# CHANGELOG.md
<!-- this_file: CHANGELOG.md -->

All notable changes to fontlift-win-cli will be documented in this file.

## [Unreleased]

### Planning & Research (2025-10-31)
- Created comprehensive PLAN.md with 7-phase implementation strategy
- Created TODO.md with 75+ itemized tasks across all phases
- Researched Windows font installation APIs (AddFontResourceEx, Registry, WM_FONTCHANGE)
- Analyzed existing implementations: fontctl (Go) and FontRegister (C#)
- Decided on minimal font parser approach to avoid FreeType dependency
- Established project structure and documentation files

### Architecture Decisions
- Target: Windows 7+ with Visual Studio 2017+ (MSVC)
- Language: C++17
- Build: Simple build.cmd batch script (may add CMake later)
- Dependencies: Windows SDK only (Advapi32, Shlwapi, User32, Gdi32)
- Code constraints: <20 lines per function, <200 lines per file
- Testing: Manual testing with sample fonts, no test framework

### Phase 1 Implementation (2025-10-31)
- Implemented src/main.cpp (155 lines)
  - Complete command-line parsing for all commands (list, install, uninstall, remove)
  - Command synonym support (l, i, u, rm)
  - Option parsing for -p (path) and -n (name) flags
  - Comprehensive usage/help message
  - Exit code constants defined (0=success, 1=error, 2=permission)
- Created build.cmd script
  - MSVC compiler detection and validation
  - C++17 compilation with /W4 warnings and /O2 optimization
  - Links Windows libraries: Advapi32.lib, Shlwapi.lib, User32.lib, Gdi32.lib
- Created publish.cmd script
  - Distribution packaging with PowerShell zip creation
  - Auto-generated README.txt for distribution
- All Phase 1 code ready for testing (requires Windows + Visual Studio)

## [0.1.0] - Planned
Initial release with core functionality:
- List installed fonts (paths, names, or both)
- Install fonts with persistent registration
- Uninstall fonts (keep files)
- Remove fonts (delete files)
- Parse font names from TTF/OTF/TTC/OTC files
- Admin privilege detection and handling
- Clear error messages and exit codes

Target: Single executable <200 KB, <1000 total lines of code
