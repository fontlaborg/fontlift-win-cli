# TODO.md
<!-- this_file: TODO.md -->

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
