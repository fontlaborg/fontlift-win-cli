// this_file: src/font_ops.h
// Font operations for fontlift-win-cli
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0
// Public API for font installation, uninstallation, removal, and registry enumeration

#ifndef FONT_OPS_H
#define FONT_OPS_H

namespace FontOps {
    // List installed fonts
    // showPaths: display file paths
    // showNames: display font names
    // Output is always sorted; path-only mode removes duplicate paths
    int ListFonts(bool showPaths, bool showNames);

    // Install font from file path
    // forceAdmin: if true, force system-level installation (requires admin)
    // Returns: 0=success, 1=error, 2=permission denied
    int InstallFont(const char* fontPath, bool forceAdmin = false);

    // Uninstall font by path (keeps file)
    // forceAdmin: request system-scope removal; user fonts are still removed when found
    int UninstallFontByPath(const char* fontPath, bool forceAdmin = false);

    // Uninstall font by name (keeps file)
    // forceAdmin: request system-scope removal; user fonts are still removed when found
    int UninstallFontByName(const char* fontName, bool forceAdmin = false);

    // Remove font by path (deletes file)
    // forceAdmin: request system-scope removal; user fonts are still removed when found
    int RemoveFontByPath(const char* fontPath, bool forceAdmin = false);

    // Remove font by name (deletes file)
    // forceAdmin: request system-scope removal; user fonts are still removed when found
    int RemoveFontByName(const char* fontName, bool forceAdmin = false);

    // Cleanup font registry and caches. includeSystem toggles system-wide scope (requires admin when true)
    int Cleanup(bool includeSystem);
}

#endif // FONT_OPS_H
