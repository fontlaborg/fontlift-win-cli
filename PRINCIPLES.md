# PRINCIPLES.md
<!-- this_file: PRINCIPLES.md -->

## General principles behind fontlift-win-cli

1. **Single purpose**: Install, uninstall, and list fonts. Nothing else.

2. **Windows-native**: Use Windows APIs directly (GDI, Registry). No cross-platform abstractions.

3. **Minimal dependencies**: Prefer Windows SDK over third-party libraries. Single executable with no runtime dependencies beyond system DLLs.

4. **Simple build process**:
   - Must have `./build.cmd` - builds the project
   - Must have `./publish.cmd` - creates distributable package

5. **Small and fast**: Keep code under 200 lines per file, functions under 20 lines. No bloat.

6. **Clear error messages**: Users should know exactly what went wrong and how to fix it.

7. **Administrator-aware**: Detect when admin rights are needed, guide user accordingly.

8. **Test with real fonts**: Validate with .ttf, .otf, .ttc, .otc files in actual Windows environments.

9. **No configuration files**: Command-line arguments only. No INI, no JSON, no TOML.

10. **Reliable persistence**: Fonts must survive reboots. Registry entries must be correct.
