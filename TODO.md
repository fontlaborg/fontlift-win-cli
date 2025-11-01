# TODO.md
<!-- this_file: TODO.md -->

## Next Actions

### CI/CD Validation

- [ ] Trigger GitHub Actions build workflow via `gh run`
- [ ] Analyze build logs for compilation and workflow issues
- [ ] Create tag v1.1.24 once build passes
- [ ] Verify GitHub Release created with correct version metadata
- [ ] Download and verify release artifacts (zip + checksums)

### Phase 7: Manual Testing (Windows Required)

- [ ] Test with filenames containing spaces
- [ ] Test with Unicode file paths
- [ ] Test with .otc files (OpenType Collection)
- [ ] Test with corrupt/invalid font files
- [ ] Test uninstall/remove of system fonts
- [ ] Verify final executable size <200 KB
- [ ] Integration test: install font, reboot, verify persistence, uninstall

### Documentation

- [ ] Update CHANGELOG.md after successful CI/CD test
- [ ] Add release notes for v1.1.24 when tagged

---

## Completed ✅

- ✅ CI/CD infrastructure (semantic versioning, GitHub Actions workflows, fallback mechanisms)
- ✅ Core font management (install, uninstall, remove, list, parse TTF/OTF/TTC/OTC)
- ✅ Code quality verification (838 lines, memory safety, security hardening)
- ✅ All documentation files (README, CONTRIBUTING, DEPENDENCIES, etc.)
- ✅ Batch file delayed expansion fixes (CI/CD Fix #9)

---
