# TODO.md
<!-- this_file: TODO.md -->

See docs/STREAMLINING_PLAN.md for complete execution plan.

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

- ✅ Core implementation (1,355 lines, zero bloat)
- ✅ CI/CD passing on Windows runners
- ✅ Initial documentation cleanup (removed 4 LLM files)
- ✅ Created streamlining plan (docs/STREAMLINING_PLAN.md)
- ✅ Created package distribution plan (docs/PACKAGE_DISTRIBUTION.md)
