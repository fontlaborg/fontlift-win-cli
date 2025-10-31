// this_file: src/font_ops.h
// Font operations for fontlift-win-cli
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0

#ifndef FONT_OPS_H
#define FONT_OPS_H

namespace FontOps {
    // List installed fonts
    // showPaths: display file paths
    // showNames: display font names
    int ListFonts(bool showPaths, bool showNames);

    // Install font from file path
    // Returns: 0=success, 1=error, 2=permission denied
    int InstallFont(const char* fontPath);

    // Uninstall font by path (keeps file)
    int UninstallFontByPath(const char* fontPath);

    // Uninstall font by name (keeps file)
    int UninstallFontByName(const char* fontName);

    // Remove font by path (deletes file)
    int RemoveFontByPath(const char* fontPath);

    // Remove font by name (deletes file)
    int RemoveFontByName(const char* fontName);
}

#endif // FONT_OPS_H
