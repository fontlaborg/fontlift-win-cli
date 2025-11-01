# fontlift-win-cli

Windows CLI tool for system font installation and management. Install fonts persistently, uninstall by path or name, list installed fonts.

## Features

- **List installed fonts** - View all system fonts with paths, names, or both
- **Install fonts** - Persistent installation with Windows Registry integration
- **Uninstall fonts** - Remove from system while keeping files
- **Remove fonts** - Uninstall and delete font files
- **Supports all formats** - TTF, OTF, TTC, OTC font files
- **No dependencies** - Single executable, Windows SDK only
- **Administrator aware** - Clear guidance when admin rights needed

## Installation

### Download Pre-built Binary

1. Download `fontlift-v0.1.0.zip` from [Releases](https://github.com/fontlaborg/fontlift-win-cli/releases)
2. Extract to a folder (e.g., `C:\Tools\fontlift\`)
3. Add folder to PATH or use full path to executable

### Build from Source

**Requirements:**
- Windows 7 or later
- Visual Studio 2017 or later (for MSVC compiler)
- Git (optional)

**Steps:**
```cmd
git clone https://github.com/fontlaborg/fontlift-win-cli.git
cd fontlift-win-cli
build.cmd
```

Executable will be created in `build\fontlift.exe`

## Usage

### List Installed Fonts

```cmd
# List font paths (default)
fontlift list
fontlift l

# List font names only
fontlift list -n

# List both paths and names (semicolon-separated)
fontlift list -n -p
fontlift list -p -n  # order doesn't matter
```

**Example output:**
```
# fontlift list
C:\Windows\Fonts\arial.ttf
C:\Windows\Fonts\times.ttf

# fontlift list -n
Arial
Times New Roman

# fontlift list -n -p
C:\Windows\Fonts\arial.ttf;Arial
C:\Windows\Fonts\times.ttf;Times New Roman
```

### Install Fonts

```cmd
# Install a single font
fontlift install C:\Downloads\myfont.ttf
fontlift i myfont.ttf

# Install with explicit path flag
fontlift install -p C:\Downloads\myfont.otf

# Install font collection (installs all fonts in .ttc/.otc)
fontlift install fonts.ttc
```

**Requirements:**
- Administrator privileges (run as Administrator or from elevated prompt)
- Valid font file (.ttf, .otf, .ttc, .otc)
- Fonts persist after reboot

### Uninstall Fonts (Keep Files)

```cmd
# Uninstall by file path
fontlift uninstall C:\Windows\Fonts\myfont.ttf
fontlift u -p myfont.ttf

# Uninstall by font name
fontlift uninstall -n "My Custom Font"
fontlift u -n Arial
```

Font files remain in `C:\Windows\Fonts\` after uninstallation.

### Remove Fonts (Delete Files)

```cmd
# Remove by file path (uninstall + delete)
fontlift remove C:\Windows\Fonts\myfont.ttf
fontlift rm -p myfont.ttf

# Remove by font name
fontlift remove -n "My Custom Font"
fontlift rm -n Arial
```

**Warning:** Font files are permanently deleted. Cannot be undone.

## Exit Codes

fontlift uses standard exit codes for scripting and automation:

- `0` - Success
- `1` - Error (invalid arguments, file not found, operation failed)
- `2` - Permission denied (administrator privileges required)

**Example usage in batch scripts:**
```cmd
fontlift install myfont.ttf
if %ERRORLEVEL% EQU 2 (
    echo Please run as Administrator
    exit /b 1
)
if %ERRORLEVEL% EQU 1 (
    echo Font installation failed
    exit /b 1
)
echo Font installed successfully
```

## Command Reference

| Command | Aliases | Description |
|---------|---------|-------------|
| `list` | `l` | List installed fonts |
| `install` | `i` | Install font(s) from file |
| `uninstall` | `u` | Uninstall font, keep file |
| `remove` | `rm` | Uninstall font, delete file |

### Options

| Option | Description |
|--------|-------------|
| `-p <path>` | Specify font file path |
| `-n <name>` | Specify font by internal name |

## Test Fonts

See [docs/test-fonts.md](docs/test-fonts.md) for the authoritative list of freely redistributable fonts used in regression tests and manual verification. The document covers download locations, storage layout, and licensing reminders.

## Troubleshooting

### "Access Denied" or Exit Code 2
**Solution:** Run Command Prompt as Administrator
- Right-click Command Prompt → "Run as administrator"
- Or use elevated PowerShell

### "Font already installed"
**Solution:** Uninstall existing font first
```cmd
fontlift uninstall -n "Font Name"
fontlift install newfont.ttf
```

### "Invalid font file"
**Solution:** Verify file format is .ttf, .otf, .ttc, or .otc
- Check file is not corrupted
- Try opening in Windows Font Viewer

### Font doesn't appear after installation
**Solution:**
1. Check exit code was 0 (success)
2. Verify with `fontlift list -n`
3. Restart application using the font
4. Reboot if necessary (rare)

## Technical Details

- **Language:** C++17
- **Platform:** Windows 7 or later
- **Dependencies:** Windows SDK only (Advapi32, Shlwapi, User32, Gdi32)
- **Size:** ~100 KB executable
- **Code:** <1000 lines

See [DEPENDENCIES.md](DEPENDENCIES.md) for detailed dependency information.

## Development

See project documentation:
- [PLAN.md](PLAN.md) - Implementation roadmap
- [TODO.md](TODO.md) - Task list
- [CHANGELOG.md](CHANGELOG.md) - Version history
- [PRINCIPLES.md](PRINCIPLES.md) - Design principles

### Building

```cmd
# Build executable
build.cmd

# Create distribution package
publish.cmd
```

## License

Copyright 2025 by Fontlab Ltd.

Licensed under the Apache License, Version 2.0. See [LICENSE](LICENSE) for details.

## Repository

https://github.com/fontlaborg/fontlift-win-cli

## Version

Current: v0.1.0 (in development)
