<!-- this_file: docs/PACKAGE_DISTRIBUTION.md -->

# Package Distribution Plan

## Overview

Make fontlift-win-cli available via Chocolatey and WinGet for easy installation on Windows.

## Chocolatey Distribution

### Package Structure

Create `fontlift.nuspec` in repository root:

```xml
<?xml version="1.0" encoding="utf-8"?>
<package xmlns="http://schemas.microsoft.com/packaging/2015/06/nuspec.xsd">
  <metadata>
    <id>fontlift</id>
    <version>1.1.24</version>
    <title>fontlift - Windows Font Management CLI</title>
    <authors>Fontlab Ltd</authors>
    <projectUrl>https://github.com/fontlaborg/fontlift-win-cli</projectUrl>
    <license type="expression">Apache-2.0</license>
    <requireLicenseAcceptance>false</requireLicenseAcceptance>
    <description>Minimal Windows CLI tool for font management. Install, uninstall, remove, and list fonts.</description>
    <summary>Windows font management from the command line</summary>
    <tags>font fonts cli windows admin</tags>
    <releaseNotes>https://github.com/fontlaborg/fontlift-win-cli/releases</releaseNotes>
  </metadata>
  <files>
    <file src="build\fontlift.exe" target="tools\fontlift.exe" />
  </files>
</package>
```

### Installation Script

Create `tools/chocolateyinstall.ps1`:

```powershell
$ErrorActionPreference = 'Stop'
$packageName = 'fontlift'
$toolsDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$exePath = Join-Path $toolsDir 'fontlift.exe'

# Add to PATH
Install-ChocolateyPath $toolsDir 'Machine'

Write-Host "fontlift installed to $exePath"
Write-Host "Run 'fontlift' to get started"
```

### Uninstallation Script

Create `tools/chocolateyuninstall.ps1`:

```powershell
$ErrorActionPreference = 'Stop'
$packageName = 'fontlift'
$toolsDir = Split-Path -Parent $MyInvocation.MyCommand.Definition

# Remove from PATH
Uninstall-ChocolateyPath $toolsDir 'Machine'
```

### Testing Locally

```powershell
# Build package
choco pack fontlift.nuspec

# Test installation
choco install fontlift -s . -y

# Test executable
fontlift --help

# Uninstall
choco uninstall fontlift -y
```

### Submission Process

1. Create account at https://community.chocolatey.org/
2. Generate API key from account settings
3. Set API key: `choco apikey -k [API_KEY] -s https://push.chocolatey.org/`
4. Push package: `choco push fontlift.1.1.24.nupkg -s https://push.chocolatey.org/`
5. Package enters moderation queue (~2-7 days for first package)
6. Once approved, updates are automatic

### Automated Publishing

Add to `.github/workflows/release.yml` after creating GitHub Release:

```yaml
- name: Build Chocolatey package
  shell: pwsh
  run: |
    choco pack fontlift.nuspec --version ${{ env.VERSION_SEMVER }}

- name: Push to Chocolatey
  if: github.event_name == 'push' && startsWith(github.ref, 'refs/tags/')
  shell: pwsh
  env:
    CHOCO_API_KEY: ${{ secrets.CHOCO_API_KEY }}
  run: |
    choco apikey -k $env:CHOCO_API_KEY -s https://push.chocolatey.org/
    choco push fontlift.${{ env.VERSION_SEMVER }}.nupkg -s https://push.chocolatey.org/
```

---

## WinGet Distribution

### Package Structure

WinGet uses a separate repository: https://github.com/microsoft/winget-pkgs

Create manifest directory structure:
```
manifests/f/Fontlaborg/fontlift/1.1.24/
  ├── Fontlaborg.fontlift.installer.yaml
  ├── Fontlaborg.fontlift.locale.en-US.yaml
  └── Fontlaborg.fontlift.yaml
```

### Manifest Files

**`Fontlaborg.fontlift.yaml`** (version manifest):
```yaml
PackageIdentifier: Fontlaborg.fontlift
PackageVersion: 1.1.24
DefaultLocale: en-US
ManifestType: version
ManifestVersion: 1.6.0
```

**`Fontlaborg.fontlift.installer.yaml`**:
```yaml
PackageIdentifier: Fontlaborg.fontlift
PackageVersion: 1.1.24
InstallerType: zip
InstallerSwitches:
  Silent: ""
  SilentWithProgress: ""
Installers:
  - Architecture: x64
    InstallerUrl: https://github.com/fontlaborg/fontlift-win-cli/releases/download/v1.1.24/fontlift-v1.1.24.zip
    InstallerSha256: <SHA256_HASH>
    NestedInstallerFiles:
      - RelativeFilePath: fontlift.exe
        PortableCommandAlias: fontlift
ManifestType: installer
ManifestVersion: 1.6.0
```

**`Fontlaborg.fontlift.locale.en-US.yaml`**:
```yaml
PackageIdentifier: Fontlaborg.fontlift
PackageVersion: 1.1.24
PackageLocale: en-US
Publisher: Fontlab Ltd
PublisherUrl: https://github.com/fontlaborg
PackageName: fontlift
PackageUrl: https://github.com/fontlaborg/fontlift-win-cli
License: Apache-2.0
LicenseUrl: https://github.com/fontlaborg/fontlift-win-cli/blob/main/LICENSE
ShortDescription: Windows CLI tool for font management
Description: Minimal Windows CLI tool for system font management. Install, uninstall, remove, and list fonts from the command line.
Moniker: fontlift
Tags:
  - font
  - fonts
  - cli
  - windows
  - admin
ManifestType: defaultLocale
ManifestVersion: 1.6.0
```

### Submission Process

1. Fork https://github.com/microsoft/winget-pkgs
2. Create manifests using `wingetcreate` tool:
   ```powershell
   winget install wingetcreate
   wingetcreate new --urls https://github.com/fontlaborg/fontlift-win-cli/releases/download/v1.1.24/fontlift-v1.1.24.zip
   ```
3. Validate manifests:
   ```powershell
   winget validate manifests/f/Fontlaborg/fontlift/1.1.24/
   ```
4. Test installation locally:
   ```powershell
   winget install --manifest manifests/f/Fontlaborg/fontlift/1.1.24/
   ```
5. Commit and push to fork
6. Create pull request to microsoft/winget-pkgs
7. Automated tests run (signature check, hash validation, installer test)
8. Microsoft reviews and merges (~1-3 days)

### Automated Publishing

Add to `.github/workflows/release.yml`:

```yaml
- name: Generate WinGet manifests
  shell: pwsh
  run: |
    # Install wingetcreate
    Invoke-WebRequest -Uri https://aka.ms/wingetcreate/latest -OutFile wingetcreate.exe

    # Generate manifests
    .\wingetcreate.exe update Fontlaborg.fontlift `
      --version ${{ env.VERSION_SEMVER }} `
      --urls https://github.com/fontlaborg/fontlift-win-cli/releases/download/v${{ env.VERSION_TAG }}/fontlift-v${{ env.VERSION_TAG }}.zip `
      --token ${{ secrets.GITHUB_TOKEN }}
```

Note: Fully automated WinGet publishing requires submitting a PR from a bot account, which requires Microsoft approval. Most maintainers submit PRs manually or semi-automatically.

---

## Installation Commands

Once published:

**Chocolatey:**
```powershell
choco install fontlift
```

**WinGet:**
```powershell
winget install Fontlaborg.fontlift
```

---

## Maintenance

### Version Updates

Both package managers pull from GitHub Releases. To update:

1. Create new git tag: `git tag v1.2.0 && git push origin v1.2.0`
2. GitHub Actions creates release and artifacts
3. **Chocolatey:** Automated push via workflow (if configured)
4. **WinGet:** Submit PR to winget-pkgs with new version manifest

### Verification

After each release:
- [ ] Verify Chocolatey package installs correctly
- [ ] Verify WinGet package installs correctly
- [ ] Test fontlift.exe runs and shows correct version
- [ ] Verify uninstall removes executable and PATH entry

---

## Timeline

1. **Week 1:** Set up Chocolatey package, test locally, submit to community repo
2. **Week 2:** Create WinGet manifests, test locally, submit PR
3. **Week 3:** Wait for approvals, address any feedback
4. **Week 4:** Verify both packages are live, document installation in README
