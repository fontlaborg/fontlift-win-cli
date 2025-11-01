<!-- this_file: docs/STREAMLINING_PLAN.md -->

# Aggressive Streamlining Plan

## Executive Summary

**Goal:** Strip fontlift-win-cli to absolute essentials. Remove all non-core files, consolidate documentation, eliminate redundancy.

**Current State:**
- 1,355 lines of code ✅ (lean, no enterprise bloat)
- 22 markdown files ⚠️ (too many)
- 0 enterprise features ✅ (clean)
- CI/CD passing ✅

**Target State:**
- Keep all 1,355 lines of code (already optimal)
- Reduce to 6-8 essential markdown files
- Package for Chocolatey + WinGet distribution
- Single-purpose focus: font management only

---

## Phase 1: Documentation Consolidation

### Files to KEEP (Essential)

1. **README.md** - User-facing documentation
   - Installation (binary + source + Chocolatey + WinGet)
   - Usage examples
   - Quick reference

2. **CHANGELOG.md** - Release history
   - Keep recent releases only (last 5-10)
   - Archive older releases to separate file

3. **LICENSE** - Legal requirement

4. **CONTRIBUTING.md** - Contributor guidelines (optional, can remove if no external contributors)

5. **DEPENDENCIES.md** - Technical reference

6. **.github/** - CI/CD workflows (essential for automation)

### Files to REMOVE (Bloat)

1. **CLAUDE.md** (22K) - LLM-specific instructions
   - Move essential info to README or CONTRIBUTING
   - Delete file

2. **CODE_OF_CONDUCT.md** (1.7K) - Overkill for single-purpose tool
   - Not needed for utility projects
   - Delete file

3. **PRINCIPLES.md** (1.1K) - Redundant with README
   - Merge into CONTRIBUTING or delete
   - Delete file

4. **WORK.md** (1.1K) - Development scratch pad
   - Not needed in repository
   - Delete file (use local notes instead)

5. **TODO.md** (1.2K) - Development tracking
   - Use GitHub Issues instead
   - Delete file

6. **PLAN.md** (2.7K) - Project planning
   - Archive or delete (implementation complete)
   - Delete file

7. **templates/** directory - Code templates
   - Not needed (code complete, no new files planned)
   - Delete directory

8. **docs/test-fonts.md** - Testing documentation
   - Merge into CONTRIBUTING if needed
   - Delete standalone file

9. **docs/PACKAGE_DISTRIBUTION.md** - Internal planning
   - Execute plan, then delete file
   - Keep implementation artifacts only

### Files to CONSOLIDATE

**Option A: Minimal (Recommended)**
- README.md (all user info)
- CHANGELOG.md (release history)
- LICENSE (required)
- .github/workflows/ (CI/CD)

**Option B: Standard**
- Add CONTRIBUTING.md (contributor guidelines)
- Add DEPENDENCIES.md (technical reference)

**Reduction:** 22 files → 4-6 files (73-82% reduction)

---

## Phase 2: Codebase Audit (Already Clean ✅)

**Analysis:** No changes needed. Code is already lean and focused.

**Evidence:**
```
0 results for: log|metric|analytics|monitor|telemetry|health
1,355 total lines of code
All functions <20 lines
All files <300 lines
```

**Core modules (all essential):**
- main.cpp (138 lines) - CLI parsing
- font_ops.cpp (289 lines) - Core operations
- font_parser.cpp (174 lines) - Font file parsing
- sys_utils.cpp (148 lines) - System utilities
- Headers (118 lines total) - Interfaces

**Build infrastructure (all essential):**
- build.cmd (122 lines) - Build script
- publish.cmd (124 lines) - Distribution packaging
- scripts/get-version.* (179 lines) - Version automation
- scripts/generate-version-rc.* (56 lines) - Resource generation

**No bloat found. Keep all code as-is.**

---

## Phase 3: Package Distribution (Implementation)

### Chocolatey Package

**Files to CREATE:**

1. **fontlift.nuspec** (repository root)
```xml
<?xml version="1.0"?>
<package xmlns="http://schemas.microsoft.com/packaging/2015/06/nuspec.xsd">
  <metadata>
    <id>fontlift</id>
    <version>$VERSION$</version>
    <title>fontlift</title>
    <authors>Fontlab Ltd</authors>
    <projectUrl>https://github.com/fontlaborg/fontlift-win-cli</projectUrl>
    <license type="expression">Apache-2.0</license>
    <requireLicenseAcceptance>false</requireLicenseAcceptance>
    <description>Minimal Windows CLI for font management. Install, uninstall, remove, list fonts.</description>
    <summary>Windows font management from command line</summary>
    <tags>font fonts cli windows admin</tags>
  </metadata>
  <files>
    <file src="build\fontlift.exe" target="tools\" />
    <file src="tools\chocolateyinstall.ps1" target="tools\" />
    <file src="LICENSE" target="tools\" />
  </files>
</package>
```

2. **tools/chocolateyinstall.ps1**
```powershell
$ErrorActionPreference = 'Stop'
$toolsDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
Install-ChocolateyPath $toolsDir 'Machine'
Write-Host "fontlift installed. Run 'fontlift' to get started."
```

3. **Update .github/workflows/release.yml** - Add Chocolatey publish step

**Testing:**
```powershell
choco pack
choco install fontlift -s . -y
fontlift list
choco uninstall fontlift -y
```

**Submission:** Push to https://community.chocolatey.org/

---

### WinGet Package

**Files to CREATE in microsoft/winget-pkgs fork:**

1. **manifests/f/Fontlaborg/fontlift/VERSION/Fontlaborg.fontlift.yaml**
```yaml
PackageIdentifier: Fontlaborg.fontlift
PackageVersion: 1.1.25
DefaultLocale: en-US
ManifestType: version
ManifestVersion: 1.6.0
```

2. **Fontlaborg.fontlift.installer.yaml**
```yaml
PackageIdentifier: Fontlaborg.fontlift
PackageVersion: 1.1.25
InstallerType: portable
NestedInstallerType: exe
Commands:
  - fontlift
Installers:
  - Architecture: x64
    InstallerUrl: https://github.com/fontlaborg/fontlift-win-cli/releases/download/v1.1.25/fontlift-v1.1.25.zip
    InstallerSha256: <HASH>
ManifestType: installer
ManifestVersion: 1.6.0
```

3. **Fontlaborg.fontlift.locale.en-US.yaml**
```yaml
PackageIdentifier: Fontlaborg.fontlift
PackageVersion: 1.1.25
PackageLocale: en-US
Publisher: Fontlab Ltd
PackageName: fontlift
License: Apache-2.0
ShortDescription: Windows font management CLI
Tags:
  - font
  - cli
  - windows
ManifestType: defaultLocale
ManifestVersion: 1.6.0
```

**Testing:**
```powershell
winget validate manifests/f/Fontlaborg/fontlift/1.1.25/
winget install --manifest manifests/f/Fontlaborg/fontlift/1.1.25/
fontlift list
```

**Submission:** PR to microsoft/winget-pkgs

---

## Phase 4: README Consolidation

**Sections to KEEP:**
- One-line description
- Installation (4 methods: binary, Chocolatey, WinGet, source)
- Usage examples (all 4 commands)
- Exit codes
- License

**Sections to REMOVE:**
- Verbose explanations
- Troubleshooting (move to GitHub Wiki or Issues)
- Development details (move to CONTRIBUTING)
- Technical architecture (move to DEPENDENCIES)

**Target:** <150 lines (currently ~200 lines)

---

## Phase 5: Execution Timeline

### Week 1: Documentation Cleanup
- [ ] Delete unnecessary MD files (CLAUDE.md, CODE_OF_CONDUCT.md, etc.)
- [ ] Delete templates/ directory
- [ ] Consolidate README to <150 lines
- [ ] Archive old CHANGELOG entries (keep last 5 releases)
- [ ] Commit: "Aggressive documentation cleanup"

### Week 2: Chocolatey Package
- [ ] Create fontlift.nuspec
- [ ] Create tools/chocolateyinstall.ps1
- [ ] Test locally: `choco pack && choco install fontlift -s . -y`
- [ ] Update release.yml to publish to Chocolatey
- [ ] Submit to Chocolatey community repo
- [ ] Wait for approval (3-7 days)

### Week 3: WinGet Package
- [ ] Fork microsoft/winget-pkgs
- [ ] Create manifest files (3 YAML files)
- [ ] Validate: `winget validate`
- [ ] Test locally: `winget install --manifest`
- [ ] Submit PR to winget-pkgs
- [ ] Wait for merge (1-3 days)

### Week 4: Verification
- [ ] Test Chocolatey installation: `choco install fontlift`
- [ ] Test WinGet installation: `winget install Fontlaborg.fontlift`
- [ ] Update README with installation methods
- [ ] Create release tag v1.2.0
- [ ] Verify automated publishing works

---

## Success Metrics

**Before:**
- 22 markdown files
- 1,355 lines of code
- Manual binary distribution only
- Complex documentation structure

**After:**
- 4-6 markdown files (73-82% reduction)
- 1,355 lines of code (unchanged - already optimal)
- 3 installation methods: binary, Chocolatey, WinGet
- Simple, focused documentation

**Installation commands:**
```powershell
# Chocolatey
choco install fontlift

# WinGet
winget install Fontlaborg.fontlift

# Binary (manual)
Expand-Archive fontlift-v1.1.25.zip
```

---

## Anti-Bloat Checklist

Before adding ANY file, ask:

1. **Is this required for core font management?** → No? Don't add it.
2. **Does this help users install/use fontlift?** → No? Don't add it.
3. **Is this information already in another file?** → Yes? Don't duplicate it.
4. **Can we use GitHub features instead?** (Issues, Wiki, Releases) → Yes? Use those.
5. **Is this developer-facing only?** → Yes? Keep it local, not in repo.

**Remember:** Every file is a maintenance burden. Ruthlessly minimize.
