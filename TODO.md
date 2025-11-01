# TODO.md
<!-- this_file: TODO.md -->

## Next Actions

### CI/CD Validation

- [ ] Trigger GitHub Actions build workflow: `gh workflow run build.yml`
- [ ] Analyze build logs, fix any issues
- [ ] Create release tag: `git tag v1.1.24 && git push origin v1.1.24`
- [ ] Verify release artifacts (zip + checksums)

### Package Distribution

- [ ] Create Chocolatey package manifest
- [ ] Test Chocolatey installation flow
- [ ] Submit to Chocolatey community repository
- [ ] Create WinGet package manifest
- [ ] Test WinGet installation flow
- [ ] Submit to winget-pkgs repository

### Manual Testing (Windows Required)

- [ ] Test edge cases: spaces in filenames, Unicode paths, .otc files
- [ ] Test error handling: corrupt files, non-admin execution
- [ ] Verify executable size <200 KB
- [ ] Integration test: install → reboot → verify → uninstall

### Documentation

- [ ] Update CHANGELOG.md after CI/CD test
- [ ] Add release notes for v1.1.24

---

## Completed ✅

- ✅ Core functionality (install, uninstall, remove, list, parse)
- ✅ CI/CD infrastructure (semantic versioning, GitHub Actions)
- ✅ Code quality verification (857 lines, memory safe)
- ✅ Documentation cleanup and streamlining
