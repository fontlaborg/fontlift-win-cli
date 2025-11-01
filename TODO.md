# TODO.md
<!-- this_file: TODO.md -->

See docs/STREAMLINING_PLAN.md for complete execution plan.

## Immediate: Quality & Robustness Improvements ✅ COMPLETE

All critical fixes implemented and tested (2025-11-02):

- ✅ **Fixed argv bounds checking in main.cpp** (MEDIUM severity)
  - Added `i + 1 < argc` checks at lines 66, 88-90, 114-116
  - Prevents crash with malformed CLI arguments

- ✅ **Added input validation for font file paths**
  - New IsValidFontPath() function in sys_utils.cpp/h
  - Validates paths don't contain `..\\` sequences
  - Applied to RemoveFontByName, UninstallFontByName

- ✅ **Improved error handling in InstallFont**
  - AddFontResourceExA failure now returns error
  - Automatic rollback: deletes registry entry + copied file
  - Ensures atomic operations

## Immediate: Additional Quality Improvements (Round 2) ✅ COMPLETE

Small refinements implemented (2025-11-02):

- ✅ **Added --version flag to CLI**
  - Shows version from embedded resource data
  - Supports both `--version` and `-v` flags
  - +31 lines in main.cpp

- ✅ **Improved error messages with actionable guidance**
  - Admin errors suggest "Run as administrator"
  - File errors suggest checking path
  - +7 "Solution:" lines in font_ops.cpp

- ✅ **Added build output validation to build.cmd**
  - Verifies fontlift.exe exists
  - Warns on suspicious file sizes
  - +17 lines in build.cmd

## Immediate: Robustness Improvements (Round 3) ✅ COMPLETE

Additional quality improvements implemented (2025-11-02):

- ✅ **Add file size validation to font_parser.cpp**
  - Rejects files <100 bytes or >50MB (clearly corrupt/invalid)
  - Early validation before attempting to parse
  - Prevents crashes from malformed font files
  - +14 lines in font_parser.cpp (2 validation blocks)

- ✅ **Add Windows API error codes to error messages**
  - Includes GetLastError() codes with FormatMessage
  - Helps diagnose CopyFile, Registry, AddFontResource failures
  - New GetLastErrorMessage() helper in sys_utils
  - +18 lines in sys_utils.cpp, +3 lines in font_ops.cpp

- ✅ **Add duplicate installation detection**
  - Checks if font already registered before installing
  - Warns user with existing location
  - Proceeds with overwrite (avoids accidental duplicates)
  - +6 lines in font_ops.cpp InstallFont()

## Week 1: Documentation Cleanup

- [ ] Delete unnecessary MD files (9 files: CLAUDE.md, CODE_OF_CONDUCT.md, PRINCIPLES.md, WORK.md, TODO.md, PLAN.md, etc.)
- [ ] Delete templates/ directory
- [ ] Consolidate README to <150 lines
- [ ] Archive CHANGELOG (keep last 5 releases)
- [ ] Commit: "Aggressive documentation cleanup"

## Week 2: Chocolatey Package

- [ ] Create fontlift.nuspec (repository root)
- [ ] Create tools/chocolateyinstall.ps1
- [ ] Test locally: `choco pack && choco install fontlift -s . -y`
- [ ] Update .github/workflows/release.yml (add Chocolatey publish)
- [ ] Submit to https://community.chocolatey.org/
- [ ] Wait for approval (3-7 days)

## Week 3: WinGet Package

- [ ] Fork microsoft/winget-pkgs
- [ ] Create manifests/f/Fontlaborg/fontlift/VERSION/*.yaml (3 files)
- [ ] Validate: `winget validate manifests/f/Fontlaborg/fontlift/VERSION/`
- [ ] Test: `winget install --manifest manifests/f/Fontlaborg/fontlift/VERSION/`
- [ ] Submit PR to microsoft/winget-pkgs
- [ ] Wait for merge (1-3 days)

## Week 4: Verification

- [ ] Test: `choco install fontlift`
- [ ] Test: `winget install Fontlaborg.fontlift`
- [ ] Update README with 3 installation methods
- [ ] Create release tag v1.2.0
- [ ] Verify automated publishing works

---

## Completed ✅

- ✅ Core implementation (991 lines source, zero bloat)
- ✅ CI/CD passing on Windows runners (5/5 builds, 100% success rate)
- ✅ Initial documentation cleanup (removed 4 LLM files)
- ✅ Created streamlining plan (docs/STREAMLINING_PLAN.md)
- ✅ Created package distribution plan (docs/PACKAGE_DISTRIBUTION.md)
- ✅ /test - Comprehensive code verification (3 rounds)
- ✅ /report - Updated WORK.md and CHANGELOG.md with test results
- ✅ Quality improvements Round 1 - Fixed 1 MEDIUM + 2 LOW severity issues
- ✅ Quality improvements Round 2 - Added 3 UX improvements
- ✅ Quality improvements Round 3 - Added 3 robustness enhancements
