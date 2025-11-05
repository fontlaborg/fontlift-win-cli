// this_file: src/sys_utils.h
// System utilities for fontlift-win-cli
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0
// Windows API wrappers for file operations, registry management, and system utilities

#ifndef SYS_UTILS_H
#define SYS_UTILS_H

#include <string>

namespace SysUtils {
    // Get Windows error message from GetLastError()
    [[nodiscard]] std::string GetLastErrorMessage();

    // Check if running with administrator privileges
    [[nodiscard]] bool IsAdmin();

    // Get Windows fonts directory path (system)
    [[nodiscard]] std::string GetFontsDirectory();

    // Get user fonts directory path
    [[nodiscard]] std::string GetUserFontsDirectory();

    // Copy file to fonts directory (system or user)
    bool CopyToFontsFolder(const char* sourcePath, std::string& destPath, bool perUser = false);

    // Delete file from fonts directory
    bool DeleteFromFontsFolder(const char* filename);

    // Check if file exists
    [[nodiscard]] bool FileExists(const char* path);

    // Get filename from full path
    [[nodiscard]] std::string GetFileName(const char* path);

    // Validate font file path (no path traversal, must be in fonts dir if absolute)
    bool IsValidFontPath(const char* path);

    // Registry operations (perUser: false = HKEY_LOCAL_MACHINE, true = HKEY_CURRENT_USER)
    bool RegReadFontEntry(const char* valueName, std::string& fontFile, bool perUser = false);
    bool RegWriteFontEntry(const char* valueName, const char* fontFile, bool perUser = false);
    bool RegDeleteFontEntry(const char* valueName, bool perUser = false);
    bool RegEnumerateFonts(void (*callback)(const char* name, const char* file, bool perUser), bool perUser = false);

    // Notify system of font changes
    void NotifyFontChange();

    // Clear Windows font caches scoped to the current user
    bool ClearUserFontCaches();

    // Clear system-wide font caches by stopping the FontCache service and deleting cache files
    bool ClearSystemFontCaches();
}

#endif // SYS_UTILS_H
