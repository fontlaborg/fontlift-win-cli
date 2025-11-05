# fontlift-win-cli

Windows CLI tool for system font installation and management. Install fonts persistently, uninstall by path or name, list installed fonts.

## Features

- **List installed fonts** - View all system fonts with paths, names, or both
- **Install fonts** - Persistent installation with Windows Registry integration
- **Uninstall fonts** - Remove from system while keeping files
- **Remove fonts** - Uninstall and delete font files
- **Supports all formats** - TTF, OTF, TTC, OTC font files
- **No dependencies** - Single executable, Windows SDK only

## Installation

### Download Binary
1. Download `fontlift-v*.zip` from [Releases](https://github.com/fontlaborg/fontlift-win-cli/releases)
2. Extract and add to PATH

### Build from Source
```cmd
git clone https://github.com/fontlaborg/fontlift-win-cli.git
cd fontlift-win-cli
build.cmd
```

## Usage

### List Fonts
```cmd
fontlift list          # Show paths (default)
fontlift list -n       # Show names
fontlift list -n -p    # Show both
fontlift list -s       # Sorted output
```

### Install Fonts
```cmd
fontlift install myfont.ttf
fontlift i -p C:\Downloads\font.otf
fontlift i myfont.ttf --admin      # Force system-level (requires admin)
fontlift i myfont.ttf -a           # Same as --admin
```

**Note:** By default, fonts are installed system-wide with admin privileges, or per-user without admin. Use `--admin` / `-a` to force system-level installation.
Existing installations with the same font family name are removed automatically.

### Uninstall Fonts (Keep Files)
```cmd
fontlift uninstall myfont.ttf
fontlift u -n "Font Name"
fontlift u -n "Font Name" --admin  # Force system-level (requires admin)
```

### Remove Fonts (Delete Files)
```cmd
fontlift remove myfont.ttf
fontlift rm -n "Font Name"
fontlift rm -n "Font Name" --admin  # Force system-level (requires admin)
```

**Warning:** Files permanently deleted

### Cleanup Caches
```cmd
fontlift cleanup              # Clean registry + user/third-party caches
fontlift cleanup --admin      # Include system font caches (requires admin)
fontlift c                    # Alias for user cleanup
```
Removes registry entries pointing to missing font files, clears user-level font caches (including Adobe `.lst` caches), and optionally purges system font caches when `--admin` is supplied.

## Commands

| Command | Alias | Description |
|---------|-------|-------------|
| `list` | `l` | List installed fonts |
| `install` | `i` | Install font from file |
| `uninstall` | `u` | Uninstall, keep file |
| `remove` | `rm` | Uninstall, delete file |
| `cleanup` | `c` | Cleans registry + user/third-party caches; with `--admin` also clears system caches |

**Options:**
- `-p <path>` - Font file path
- `-n <name>` - Font internal name
- `-s` - Sort output (list only)
- `--admin`, `-a` - Force system-level operation (requires admin)

## Exit Codes

- `0` - Success
- `1` - Error
- `2` - Permission denied (need Administrator for system fonts)

## Troubleshooting

**Access Denied for System Fonts (Exit 2)**
When uninstalling/removing system fonts, run Command Prompt as Administrator. Per-user fonts don't require admin.

**Font already installed**
Uninstall existing font first: `fontlift u -n "Name"`

**Invalid font file**
Verify format (.ttf, .otf, .ttc, .otc) and file integrity

**Font doesn't appear**
Check exit code, verify with `fontlift list -n`, restart application

**Font cache issues or rendering glitches**
Run `fontlift cleanup` as administrator to purge font caches and broken registry entries.

## Technical Details

- C++17, Windows 7+
- Dependencies: Windows SDK only (Advapi32, Shlwapi, User32, Gdi32)
- Size: ~100 KB
- Code: <1000 lines

See [DEPENDENCIES.md](DEPENDENCIES.md) for details.

## Building

```cmd
build.cmd      # Build executable
publish.cmd    # Create distribution
```

## License

Copyright 2025 by Fontlab Ltd.
Licensed under Apache License 2.0 - see [LICENSE](LICENSE)

## Links

- Repository: https://github.com/fontlaborg/fontlift-win-cli
- Changelog: [CHANGELOG.md](CHANGELOG.md)
- Test Fonts: [docs/test-fonts.md](docs/test-fonts.md)
