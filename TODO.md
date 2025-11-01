# TODO.md
<!-- this_file: TODO.md -->

## CRITICAL - Current Blockers

- [ ] Trigger GitHub Actions build to validate new semver/version-resource pipeline on Windows runners
- [ ] Trigger a tag-based release once CI is green to confirm assets, checksums, and version metadata
- [x] Verify `fontlift list` has no prolog/epilog output (requirement: pure list only, pipe-friendly)
- [ ] Monitor CI/logs post-release to ensure build + publish scripts handle pre-release semver suffixes correctly

## Completed Issues

- [x] @issues/101.md - CI/CD infrastructure (98% complete, needs 1-line fix above)
- [x] @issues/103.md - Core font management functionality (100% complete)

## Phase 0: Build Infrastructure & CI/CD ✅ COMPLETED
### Version Infrastructure
- [x] Create `scripts/` directory
- [x] Create `scripts/get-version.cmd` - extract version from git tags
- [x] Create `scripts/generate-version-rc.cmd` - generate version.rc from template
- [x] Create `templates/version.rc.template` - Windows version resource template
- [x] Test version extraction locally (no git tags - should return v0.0.0-dev)
- [x] Create initial git tag: `v0.1.0`
- [x] Test version extraction with git tag (should return 0.1.0)
- [x] **BUG FIX:** Fixed delayed expansion syntax in generate-version-rc.cmd (2025-11-01)

### Build Script Enhancement
- [x] Backup current build.cmd
- [x] Update build.cmd to accept optional version parameter
- [x] Integrate get-version.cmd call in build.cmd
- [x] Integrate generate-version-rc.cmd call in build.cmd
- [x] Add version.rc to MSVC compilation command
- [x] Add version.rc to linker inputs
- [x] Test local build with explicit version: `build.cmd 0.1.0`
- [x] Test local build without version (should use git tag)
- [x] Verify version appears in executable properties (Windows: right-click > Properties > Details)
- [x] Verify build/ directory structure unchanged

### Publish Script Enhancement
- [x] Backup current publish.cmd
- [x] Update publish.cmd to accept optional version parameter
- [x] Integrate get-version.cmd call in publish.cmd
- [x] Replace hardcoded "0.1.0" with version variable
- [x] Update zip filename to use version: `fontlift-v%VERSION_TAG%.zip`
- [x] Update README.txt generation to use version
- [x] Test local publish with explicit version: `publish.cmd 0.1.0`
- [x] Test local publish without version (should use git tag)
- [x] Verify zip file has correct name
- [x] Verify README.txt has correct version

### GitHub Actions Workflows
- [x] Create `.github/workflows/` directory
- [x] Create `.github/workflows/build.yml` - CI build workflow
  - [x] Add triggers: push to main, pull requests
  - [x] Configure windows-latest runner
  - [x] Add checkout step with fetch-depth: 0
  - [x] Add MSVC setup step (ilammy/msvc-dev-cmd@v1)
  - [x] Add version extraction step
  - [x] Add build step
  - [x] Add test step (executable exists)
  - [x] Add smoke test (executable runs)
  - [x] Add artifact upload step
- [x] Create `.github/workflows/release.yml` - release workflow
  - [x] Add trigger: push tags matching v*.*.*
  - [x] Configure windows-latest runner with write permissions
  - [x] Add checkout step with fetch-depth: 0
  - [x] Add MSVC setup step
  - [x] Add version extraction from tag
  - [x] Add build step with version
  - [x] Add publish step with version
  - [x] Add checksum generation step
  - [x] Add GitHub Release creation step
  - [x] Add asset upload step

### CI/CD Testing
- [x] Commit version infrastructure files
- [x] Push to main branch
- [x] Verify CI build workflow triggers
- [x] Fix batch script bug #1 (delayed expansion syntax in generate-version-rc.cmd)
- [x] Replace fragile batch parsing with PowerShell helpers for version detection and version.rc generation
- [x] Rename helper parameters to `-TargetVersion` to avoid PowerShell host switch conflicts when passing explicit versions
- [ ] Verify CI build workflow passes (pending Windows runner execution)
- [ ] Verify artifact uploaded (pending CI rerun)
- [ ] Download CI artifact and test locally (pending CI rerun)
- [x] Create and push git tags: `v0.1.0`, `v1.0.0`, `v1.1.0`, `v1.1.1`, `v1.1.2`, `v1.1.3`
- [x] Verify release workflow triggers
- [ ] Verify release workflow passes (pending new tag after CI validation)
- [ ] Verify GitHub Release created with assets (pending next release tag)
- [ ] Verify release assets uploaded (pending release rerun)
- [ ] Download release zip and verify contents (pending release rerun)
- [ ] Verify executable has correct version in properties (pending release rerun)
- [ ] Test release executable runs correctly (pending release rerun)

### Documentation Updates
- [x] Update README.md - add "Building from Source" section
- [x] Update README.md - add "Release Process" section
- [x] Update README.md - document version tagging process
- [x] Update CONTRIBUTING.md - add CI/CD information
- [x] Update CONTRIBUTING.md - document how to test workflows locally
- [x] Update CHANGELOG.md - add v0.1.0 release notes for CI/CD
- [x] Create `.github/workflows/README.md` - explain workflows

### Cleanup & Validation
- [x] Add `src/version.rc` to .gitignore (generated file)
- [x] Verify `scripts/` directory has proper `this_file` tracking
- [ ] Run full end-to-end test: tag -> build -> publish -> release (pending after bug fix)
- [x] Document issues discovered (batch script delayed expansion bug)
- [x] Update WORK.md with Phase 0 completion status

## Phase 1: Foundation
- [x] Create src/ directory structure
- [x] Create src/main.cpp with command-line argument parsing
- [x] Implement command routing (list, install, uninstall, remove)
- [x] Add usage/help message display
- [x] Create build.cmd to compile with MSVC
- [x] Create publish.cmd to package distribution
- [ ] Test build.cmd produces fontlift.exe (requires Windows + MSVC)
- [ ] Test basic command parsing (requires Windows + MSVC)

## Phase 2: List Fonts
- [ ] Create src/font_ops.h header file
- [ ] Create src/font_ops.cpp implementation file
- [ ] Implement Registry enumeration for fonts
- [ ] Implement ListFonts() with -p flag (show paths)
- [ ] Implement ListFonts() with -n flag (show names)
- [ ] Implement ListFonts() with -n -p flags (show both)
- [ ] Handle fonts in custom locations (non-%windir%\fonts)
- [ ] Handle permission errors gracefully
- [ ] Test list command with various flags

## Phase 3: Install Fonts (Basic)
- [ ] Create src/sys_utils.h header file
- [ ] Create src/sys_utils.cpp implementation file
- [ ] Implement IsAdmin() privilege check
- [ ] Implement CopyToFontsFolder() function
- [ ] Implement Registry write operations
- [ ] Implement NotifyFontChange() with WM_FONTCHANGE
- [ ] Implement InstallFont() basic version (use filename as font name)
- [ ] Add file validation (exists, readable)
- [ ] Add duplicate check (font already installed)
- [ ] Test install with .ttf file
- [ ] Test install without admin rights (should fail gracefully)
- [ ] Verify font persists after reboot

## Phase 4: Font Name Parsing
- [ ] Create src/font_parser.h header file
- [ ] Create src/font_parser.cpp implementation file
- [ ] Implement TTF/OTF file header parsing
- [ ] Implement table directory parsing
- [ ] Implement name table location
- [ ] Implement name table parsing (nameID 1 and 4)
- [ ] Implement UTF-16BE to ASCII/UTF-8 conversion
- [ ] Implement TTC/OTC collection header parsing
- [ ] Implement GetFontName() for single fonts
- [ ] Implement GetFontsInCollection() for TTC/OTC
- [ ] Add fallback to filename if parsing fails
- [ ] Test with various .ttf files
- [ ] Test with .otf files
- [ ] Test with .ttc collection files
- [ ] Update InstallFont() to use parsed font names

## Phase 5: Uninstall Fonts
- [ ] Implement UninstallFontByPath() function
- [ ] Implement UninstallFontByName() function
- [ ] Add Registry search for font name matching
- [ ] Handle multiple matches (list and prompt)
- [ ] Implement RemoveFontResourceExA() call
- [ ] Ensure WM_FONTCHANGE notification sent
- [ ] Verify font file remains in folder
- [ ] Test uninstall by path
- [ ] Test uninstall by exact name
- [ ] Test uninstall by partial name
- [ ] Test uninstall of non-existent font

## Phase 6: Remove Fonts
- [ ] Implement RemoveFontByPath() function
- [ ] Implement RemoveFontByName() function
- [ ] Add file deletion after uninstall
- [ ] Add safety check (don't delete outside fonts folder)
- [ ] Handle locked file errors (font in use)
- [ ] Test remove by path
- [ ] Test remove by name
- [ ] Test remove of font in use (should error)
- [ ] Verify file is deleted

## Quality Improvements (Pre-Phase 2)
- [x] Fix BUG-001: List command flag order dependency (main.cpp:45-53)
- [x] Add .gitignore to exclude build artifacts and IDE files
- [x] Add this_file tracking to build.cmd and publish.cmd (already present)

## Quality Improvements (Iteration 2)
- [x] Create DEPENDENCIES.md documenting Windows SDK libraries
- [x] Enhance README.md with installation, usage examples, and exit codes
- [x] Add C++ source template with license header for future files

## Quality Improvements (Iteration 3)
- [x] Add .editorconfig for code style consistency across editors
- [x] Create CONTRIBUTING.md with guidelines for contributors
- [x] Add GitHub issue templates for bug reports and feature requests

## Quality Improvements (Iteration 4)
- [x] Add GitHub pull request template for quality PRs
- [x] Add CODE_OF_CONDUCT.md for open source community standards
- [x] Document test font requirements and sources

## Phase 7: Polish & Testing
- [ ] Add comprehensive error messages
- [ ] Implement exit codes (0=success, 1=error, 2=permission)
- [ ] Add early admin privilege detection
- [ ] Test with filenames containing spaces
- [ ] Test with Unicode file paths
- [ ] Test with .otc files
- [ ] Test with corrupt/invalid font files
- [ ] Test uninstall/remove of system fonts
- [ ] Create tests/test_fonts/ directory with sample fonts
- [ ] Create tests/run_tests.cmd script
- [x] Update README.md with installation instructions
- [x] Update README.md with usage examples
- [x] Add exit code documentation to README.md
- [x] Create CHANGELOG.md with v1.0 release notes
- [x] Create WORK.md to track testing results
- [ ] Verify final executable size <200 KB
- [ ] Verify total code <1000 lines
- [ ] Final integration test: install, reboot, verify, uninstall
