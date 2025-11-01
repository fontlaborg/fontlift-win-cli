// this_file: src/sys_utils.h
// System utilities for fontlift-win-cli
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0

#ifndef SYS_UTILS_H
#define SYS_UTILS_H

#include <string>

namespace SysUtils {
    // Check if running with administrator privileges
    bool IsAdmin();

    // Get Windows fonts directory path
    std::string GetFontsDirectory();

    // Copy file to fonts directory
    bool CopyToFontsFolder(const char* sourcePath, std::string& destPath);

    // Delete file from fonts directory
    bool DeleteFromFontsFolder(const char* filename);

    // Check if file exists
    bool FileExists(const char* path);

    // Get filename from full path
    std::string GetFileName(const char* path);

    // Validate font file path (no path traversal, must be in fonts dir if absolute)
    bool IsValidFontPath(const char* path);

    // Registry operations
    bool RegReadFontEntry(const char* valueName, std::string& fontFile);
    bool RegWriteFontEntry(const char* valueName, const char* fontFile);
    bool RegDeleteFontEntry(const char* valueName);
    bool RegEnumerateFonts(void (*callback)(const char* name, const char* file));

    // Notify system of font changes
    void NotifyFontChange();
}

#endif // SYS_UTILS_H
