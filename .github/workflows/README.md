# GitHub Actions Workflows
<!-- this_file: .github/workflows/README.md -->

This directory contains automated CI/CD workflows for fontlift-win-cli.

## Workflows

### build.yml - CI Build

**Trigger:** Push to `main` branch or pull requests

**Purpose:** Validate that the code compiles and runs correctly

**Steps:**
1. Checkout code with full git history
2. Setup MSVC compiler environment
3. Extract version from git tags (or use `v0.0.0-dev` if no tags exist)
4. Build fontlift.exe with embedded version resource
5. Test that executable was created
6. Test that executable runs without errors
7. Upload executable as build artifact (7-day retention)

**Artifacts:** `fontlift-{version}` containing `fontlift.exe`

### release.yml - Release Creation

**Trigger:** Push tag matching `v*.*.*` (e.g., `v0.1.0`, `v1.2.3`)

**Purpose:** Create GitHub releases with distribution packages

**Steps:**
1. Checkout code with full git history
2. Setup MSVC compiler environment
3. Extract version from tag name
4. Build fontlift.exe with version embedded
5. Package distribution zip: `fontlift-v{X.Y.Z}.zip`
6. Generate SHA256 checksums
7. Create GitHub Release with auto-generated release notes
8. Upload zip and checksums as release assets

**Release Assets:**
- `fontlift-v{X.Y.Z}.zip` - Distribution package
- `checksums.txt` - SHA256 checksums for verification

## Version Management

Version is managed through git tags:

- **No tags:** Uses `v0.0.0-dev` as version
- **Tagged release:** Uses tag name (e.g., `v1.2.3` → version `1.2.3`)

Version is embedded in:
1. Windows executable resource (visible in file properties)
2. Distribution zip filename
3. Distribution README.txt
4. GitHub Release metadata

## Creating a Release

To create a new release:

1. Ensure all changes are committed and tests pass
2. Update CHANGELOG.md with release notes
3. Create and push an annotated tag:
   ```bash
   git tag -a v0.1.0 -m "Release v0.1.0"
   git push origin v0.1.0
   ```
4. GitHub Actions will automatically:
   - Build the executable with version v0.1.0
   - Package fontlift-v0.1.0.zip
   - Create GitHub Release
   - Upload release artifacts

## Testing Workflows Locally

Workflows use Windows-specific commands and cannot be tested on macOS/Linux.

To test changes:
1. Push to a feature branch
2. Create a pull request
3. CI build workflow will run automatically
4. Review build logs for any errors

For release workflow testing:
1. Create a test tag: `git tag v0.0.1-test`
2. Push tag: `git push origin v0.0.1-test`
3. Workflow will run and create a release
4. Delete test release and tag if successful

## Troubleshooting

**Build fails with "cl.exe not found":**
- MSVC setup step failed
- Check that `ilammy/msvc-dev-cmd@v1` action is working

**Version shows as "0.0.0-dev":**
- No git tags exist, or fetch-depth: 0 is missing
- Ensure checkout step includes `fetch-depth: 0`

**Release workflow doesn't trigger:**
- Tag doesn't match `v*.*.*` pattern
- Ensure tag has format: `v1.2.3` (semantic versioning)

**Checksum generation fails:**
- Zip file wasn't created
- Check publish.cmd output for errors
