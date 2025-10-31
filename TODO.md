# TODO.md
<!-- this_file: TODO.md -->

## Phase 0: Build Infrastructure & CI/CD
### Version Infrastructure
- [ ] Create `scripts/` directory
- [ ] Create `scripts/get-version.cmd` - extract version from git tags
- [ ] Create `scripts/generate-version-rc.cmd` - generate version.rc from template
- [ ] Create `templates/version.rc.template` - Windows version resource template
- [ ] Test version extraction locally (no git tags - should return v0.0.0-dev)
- [ ] Create initial git tag: `v0.1.0`
- [ ] Test version extraction with git tag (should return 0.1.0)

### Build Script Enhancement
- [ ] Backup current build.cmd
- [ ] Update build.cmd to accept optional version parameter
- [ ] Integrate get-version.cmd call in build.cmd
- [ ] Integrate generate-version-rc.cmd call in build.cmd
- [ ] Add version.rc to MSVC compilation command
- [ ] Add version.rc to linker inputs
- [ ] Test local build with explicit version: `build.cmd 0.1.0`
- [ ] Test local build without version (should use git tag)
- [ ] Verify version appears in executable properties (Windows: right-click > Properties > Details)
- [ ] Verify build/ directory structure unchanged

### Publish Script Enhancement
- [ ] Backup current publish.cmd
- [ ] Update publish.cmd to accept optional version parameter
- [ ] Integrate get-version.cmd call in publish.cmd
- [ ] Replace hardcoded "0.1.0" with version variable
- [ ] Update zip filename to use version: `fontlift-v%VERSION_TAG%.zip`
- [ ] Update README.txt generation to use version
- [ ] Test local publish with explicit version: `publish.cmd 0.1.0`
- [ ] Test local publish without version (should use git tag)
- [ ] Verify zip file has correct name
- [ ] Verify README.txt has correct version

### GitHub Actions Workflows
- [ ] Create `.github/workflows/` directory
- [ ] Create `.github/workflows/build.yml` - CI build workflow
  - [ ] Add triggers: push to main, pull requests
  - [ ] Configure windows-latest runner
  - [ ] Add checkout step with fetch-depth: 0
  - [ ] Add MSVC setup step (ilammy/msvc-dev-cmd@v1)
  - [ ] Add version extraction step
  - [ ] Add build step
  - [ ] Add test step (executable exists)
  - [ ] Add smoke test (executable runs)
  - [ ] Add artifact upload step
- [ ] Create `.github/workflows/release.yml` - release workflow
  - [ ] Add trigger: push tags matching v*.*.*
  - [ ] Configure windows-latest runner with write permissions
  - [ ] Add checkout step with fetch-depth: 0
  - [ ] Add MSVC setup step
  - [ ] Add version extraction from tag
  - [ ] Add build step with version
  - [ ] Add publish step with version
  - [ ] Add checksum generation step
  - [ ] Add GitHub Release creation step
  - [ ] Add asset upload step

### CI/CD Testing
- [ ] Commit version infrastructure files
- [ ] Push to main branch
- [ ] Verify CI build workflow triggers
- [ ] Verify CI build workflow passes
- [ ] Verify artifact uploaded
- [ ] Download CI artifact and test locally
- [ ] Create and push git tag: `v0.1.0`
- [ ] Verify release workflow triggers
- [ ] Verify release workflow passes
- [ ] Verify GitHub Release created
- [ ] Verify release assets uploaded (zip + checksums)
- [ ] Download release zip and verify contents
- [ ] Verify executable has correct version in properties
- [ ] Test release executable runs correctly

### Documentation Updates
- [ ] Update README.md - add "Building from Source" section
- [ ] Update README.md - add "Release Process" section
- [ ] Update README.md - document version tagging process
- [ ] Update CONTRIBUTING.md - add CI/CD information
- [ ] Update CONTRIBUTING.md - document how to test workflows locally
- [ ] Update CHANGELOG.md - add v0.1.0 release notes for CI/CD
- [ ] Create `.github/workflows/README.md` - explain workflows

### Cleanup & Validation
- [ ] Add `src/version.rc` to .gitignore (generated file)
- [ ] Verify `scripts/` directory has proper `this_file` tracking
- [ ] Run full end-to-end test: tag -> build -> publish -> release
- [ ] Document any issues or edge cases discovered
- [ ] Update WORK.md with Phase 0 completion status

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
- [ ] Add GitHub pull request template for quality PRs
- [ ] Add CODE_OF_CONDUCT.md for open source community standards
- [ ] Document test font requirements and sources

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
