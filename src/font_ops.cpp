// this_file: src/font_ops.cpp
// Font operations implementation
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0

#include "exit_codes.h"
#include "font_ops.h"
#include "sys_utils.h"
#include "font_parser.h"
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <set>
#include <algorithm>

// Font registry suffix constants (per Windows font registry naming convention)
constexpr const char* FONT_SUFFIX_TRUETYPE = " (TrueType)";
constexpr const char* FONT_SUFFIX_OPENTYPE = " (OpenType)";

namespace FontOps {
// Font installation, uninstallation, and registry management operations

// Global context for registry enumeration callback
// Windows RegEnumValueA requires a C-style callback function that cannot capture state.
// This global struct holds the configuration and output destination for the ListCallback function,
// enabling stateful enumeration of registry font entries across both system and user registries.
static struct {
    bool showPaths;
    bool showNames;
    bool sorted;
    std::string fontsDir;
    std::string userFontsDir;
    std::set<std::string>* outputSet;  // For sorted/unique output
} g_listContext;

static struct {
    int removedCount;
    bool changed;
    std::string fontsDir;
    std::string userFontsDir;
} g_cleanupContext;

// Helper: Resolve relative or absolute font file path
static std::string ResolveFullPath(const char* file, bool perUser) {
    size_t fileLen = strlen(file);
    bool isAbsolute = (fileLen > 0 && (file[0] == '\\' || file[0] == '/')) || (fileLen > 1 && file[1] == ':');
    // Absolute paths are used for per-user fonts, relative paths for system fonts
    if (isAbsolute) return file;
    const std::string& baseDir = perUser ? g_listContext.userFontsDir : g_listContext.fontsDir;
    if (baseDir.empty()) return file;
    return baseDir + "\\" + file;
}

// Helper: Format font output based on display flags (path/name/both)
static std::string FormatOutput(const std::string& path, const char* name) {
    if (g_listContext.showPaths && g_listContext.showNames) return path + "::" + name;
    if (g_listContext.showNames) return name;
    return path;
}

// Windows registry enumeration callback (C-style function pointer required by RegEnumValueA API)
// Called once for each font entry found in the registry (system or user fonts)
// Processes each entry according to g_listContext configuration (sorted, paths, names)
static void ListCallback(const char* name, const char* file, bool perUser) {
    std::string fullPath = ResolveFullPath(file, perUser);
    std::string output = FormatOutput(fullPath, name);

    if (g_listContext.sorted) {
        g_listContext.outputSet->insert(output);
    } else {
        std::cout << output << "\n";
    }
}

// Helper: Output sorted and deduplicated font list
static void OutputSorted(const std::set<std::string>& outputSet) {
    for (const auto& line : outputSet) {
        std::cout << line << "\n";
    }
}

// Registry cleanup callback: remove entries pointing to missing files
static void CleanupCallback(const char* name, const char* file, bool perUser) {
    if (!file || file[0] == '\0') return;
    size_t len = strlen(file);
    bool isAbsolute = (len > 1 && file[1] == ':') || (len > 0 && (file[0] == '\\' || file[0] == '/'));
    const std::string& baseDir = perUser ? g_cleanupContext.userFontsDir : g_cleanupContext.fontsDir;
    if (!isAbsolute && baseDir.empty()) {
        std::cerr << "    Warning: Skipping registry entry '" << name << "' (unknown base directory).\n";
        return;
    }
    std::string fullPath = isAbsolute ? std::string(file) : baseDir + "\\" + file;
    if (SysUtils::FileExists(fullPath.c_str())) return;

    std::cout << "  - Removing broken entry: " << name << "\n";
    std::cout << "    File not found: " << fullPath << "\n";
    if (SysUtils::RegDeleteFontEntry(name, perUser)) {
        g_cleanupContext.removedCount++;
        g_cleanupContext.changed = true;
    } else {
        std::cerr << "    Warning: Failed to remove registry entry.\n";
    }
}

// Registry cleanup orchestrator: enumerate system and/or user fonts
static int CleanupRegistry(bool includeSystem, bool includeUser) {
    g_cleanupContext.removedCount = 0;
    g_cleanupContext.changed = false;
    g_cleanupContext.fontsDir.clear();
    g_cleanupContext.userFontsDir.clear();

    bool success = true;

    if (includeSystem) {
        g_cleanupContext.fontsDir = SysUtils::GetFontsDirectory();
        if (g_cleanupContext.fontsDir.empty()) {
            std::cerr << "Error: Could not determine system fonts directory.\n";
            success = false;
        } else if (!SysUtils::RegEnumerateFonts(CleanupCallback, false)) {
            std::cerr << "Error: Failed to enumerate system fonts.\n";
            success = false;
        }
    }

    if (includeUser) {
        g_cleanupContext.userFontsDir = SysUtils::GetUserFontsDirectory();
        if (g_cleanupContext.userFontsDir.empty()) {
            std::cerr << "    Warning: Could not determine user fonts directory.\n";
            success = false;
        } else if (!SysUtils::RegEnumerateFonts(CleanupCallback, true)) {
            std::cerr << "    Warning: Failed to enumerate user fonts.\n";
            success = false;
        }
    }

    if (g_cleanupContext.changed) {
        SysUtils::NotifyFontChange();
    }

    return success ? g_cleanupContext.removedCount : -1;
}

int ListFonts(bool showPaths, bool showNames, bool sorted) {
    g_listContext.showPaths = showPaths;
    g_listContext.showNames = showNames;
    g_listContext.sorted = sorted;
    g_listContext.fontsDir = SysUtils::GetFontsDirectory();
    g_listContext.userFontsDir = SysUtils::GetUserFontsDirectory();
    if (g_listContext.fontsDir.empty()) {
        std::cerr << "Error: Cannot determine fonts directory\n";
        return EXIT_ERROR;
    }
    std::set<std::string> outputSet;
    if (sorted) g_listContext.outputSet = &outputSet;

    // Enumerate system fonts (HKEY_LOCAL_MACHINE)
    if (!SysUtils::RegEnumerateFonts(ListCallback, false)) {
        std::cerr << "Error: Failed to enumerate system fonts\n";
        return EXIT_ERROR;
    }

    // Enumerate user fonts (HKEY_CURRENT_USER)
    SysUtils::RegEnumerateFonts(ListCallback, true);  // Don't fail if user fonts missing

    if (sorted) OutputSorted(outputSet);
    return EXIT_SUCCESS_CODE;
}

// Helper: Check if file has valid font extension (.ttf, .otf, .ttc, .otc)
static bool HasValidFontExtension(const char* path) noexcept {
    constexpr const char* validExts[] = {".ttf", ".otf", ".ttc", ".otc"};
    std::string pathStr(path);
    // Locale-independent ASCII lowercase conversion
    for (auto& c : pathStr) {
        if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
    }
    for (const char* ext : validExts) {
        if (pathStr.length() >= strlen(ext) &&
            pathStr.compare(pathStr.length() - strlen(ext), strlen(ext), ext) == 0) {
            return true;
        }
    }
    return false;
}

// Helper: Validate font file exists and has valid extension before installation
static int ValidateInstallPrerequisites(const char* fontPath) {
    if (!HasValidFontExtension(fontPath)) {
        std::cerr << "Error: Invalid font file extension\n";
        std::cerr << "Solution: Use a valid font file (.ttf, .otf, .ttc, .otc)\n";
        return EXIT_ERROR;
    }
    if (!SysUtils::FileExists(fontPath)) {
        std::cerr << "Error: Font file not found: " << fontPath << "\n";
        std::cerr << "Solution: Check the file path and ensure the font file exists\n";
        return EXIT_ERROR;
    }
    return EXIT_SUCCESS_CODE;
}

// Helper: Extract font name from font file
static int ExtractFontName(const char* fontPath, std::string& outName) {
    if (FontParser::IsCollection(fontPath)) {
        std::vector<std::string> names = FontParser::GetFontsInCollection(fontPath);
        if (names.empty()) {
            std::cerr << "Error: Failed to parse font collection\n";
            return EXIT_ERROR;
        }
        outName = names[0];
        if (names.size() > 1) std::cout << "Note: Collection contains " << names.size() << " fonts\n";
    } else {
        outName = FontParser::GetFontName(fontPath);
        if (outName.empty()) {
            std::cerr << "Error: Failed to parse font name\n";
            return EXIT_ERROR;
        }
    }
    return EXIT_SUCCESS_CODE;
}

// Helper: Search registry for font by name with automatic suffix matching (TrueType/OpenType)
static bool FindFontInRegistry(const char* fontName, std::string& outFile, std::string& outRegName, bool& outPerUser, bool forceSystemOnly = false) {
    std::vector<std::string> variants = {
        std::string(fontName) + FONT_SUFFIX_TRUETYPE,
        std::string(fontName) + FONT_SUFFIX_OPENTYPE,
        std::string(fontName)
    };

    // If not forcing system-only, first check user registry (HKEY_CURRENT_USER)
    if (!forceSystemOnly) {
        for (const auto& variant : variants) {
            if (SysUtils::RegReadFontEntry(variant.c_str(), outFile, true)) {
                outRegName = variant;
                outPerUser = true;
                return true;
            }
        }
    }

    // Then check system registry (HKEY_LOCAL_MACHINE)
    for (const auto& variant : variants) {
        if (SysUtils::RegReadFontEntry(variant.c_str(), outFile, false)) {
            outRegName = variant;
            outPerUser = false;
            return true;
        }
    }
    return false;
}

// Helper: Write font to registry and load into system via AddFontResourceEx
static int RegisterAndLoadFont(const std::string& destPath, const std::string& fontName, bool perUser) {
    std::string regValue = perUser ? destPath : SysUtils::GetFileName(destPath.c_str());
    std::string regName = fontName + FONT_SUFFIX_TRUETYPE;
    std::string existingFile;
    if (SysUtils::RegReadFontEntry(regName.c_str(), existingFile, perUser)) {
        std::cerr << "Warning: Font '" << fontName << "' already installed, overwriting...\n";
    }
    if (!SysUtils::RegWriteFontEntry(regName.c_str(), regValue.c_str(), perUser)) {
        std::cerr << "Error: Failed to register font in registry: " << SysUtils::GetLastErrorMessage() << "\n";
        SysUtils::DeleteFromFontsFolder(SysUtils::GetFileName(destPath.c_str()).c_str());
        return EXIT_ERROR;
    }
    if (AddFontResourceExA(destPath.c_str(), FR_PRIVATE, 0) == 0) {
        std::cerr << "Error: Failed to load font resource: " << SysUtils::GetLastErrorMessage() << "\n";
        SysUtils::RegDeleteFontEntry(regName.c_str(), perUser);
        SysUtils::DeleteFromFontsFolder(SysUtils::GetFileName(destPath.c_str()).c_str());
        return EXIT_ERROR;
    }
    return EXIT_SUCCESS_CODE;
}

// Helper: Best-effort removal of existing font with matching family name before installation
static void TryUninstallExistingFont(const std::string& fontName, bool forceAdmin) {
    for (unsigned int attempt = 0; attempt < 4; ++attempt) {
        std::string fontFile;
        std::string matchedName;
        bool perUser = false;
        if (!FindFontInRegistry(fontName.c_str(), fontFile, matchedName, perUser, forceAdmin)) return;
        if (!perUser && !SysUtils::IsAdmin()) {
            std::cerr << "Warning: Found older font '" << fontName << "' but cannot remove it without admin privileges.\n";
            return;
        }
        if (!SysUtils::IsValidFontPath(fontFile.c_str())) {
            std::cerr << "Warning: Found invalid registry path for '" << fontName << "', skipping automatic uninstall.\n";
            return;
        }
        if (UnloadAndCleanupFont(fontFile, matchedName, fontName, false, perUser) != EXIT_SUCCESS_CODE) {
            std::cerr << "Warning: Failed to remove existing font '" << fontName << "' before installation.\n";
            return;
        }
        std::cout << "Note: Automatically uninstalled older version of: " << fontName << "\n";
    }
    std::cerr << "Warning: Stopping automatic uninstall for '" << fontName << "' after multiple attempts.\n";
}

// Helper: Remove font from system memory and registry, optionally delete file
static int UnloadAndCleanupFont(const std::string& fontFile, const std::string& matchedName, const std::string& fontName, bool deleteFile, bool perUser) {
    // For per-user fonts, fontFile is already an absolute path
    // For system fonts, fontFile is relative and needs to be combined with system fonts dir
    std::string fullPath;
    bool isAbsolute = (fontFile.length() > 1 && fontFile[1] == ':');
    if (isAbsolute) {
        fullPath = fontFile;
    } else {
        std::string fontsDir = SysUtils::GetFontsDirectory();
        fullPath = fontsDir + "\\" + fontFile;
    }

    // RemoveFontResourceExA failure is non-fatal: font may not be loaded in current process
    // Warning message informs user, but we proceed with registry/file cleanup
    if (RemoveFontResourceExA(fullPath.c_str(), FR_PRIVATE, 0) == 0) {
        std::cerr << "Warning: Failed to unload font resource\n";
    }
    if (!SysUtils::RegDeleteFontEntry(matchedName.c_str(), perUser)) {
        std::cerr << "Error: Failed to remove font from registry\n";
        return EXIT_ERROR;
    }
    if (deleteFile) {
        if (isAbsolute) {
            if (DeleteFileA(fullPath.c_str()) == 0) {
                std::cerr << "Error: Failed to delete font file: " << fullPath << "\n";
                std::cerr << "Font has been uninstalled but file remains\n";
                SysUtils::NotifyFontChange();
                return EXIT_ERROR;
            }
        } else {
            if (!SysUtils::DeleteFromFontsFolder(fontFile.c_str())) {
                std::cerr << "Error: Failed to delete font file: " << fullPath << "\n";
                std::cerr << "Font has been uninstalled but file remains\n";
                SysUtils::NotifyFontChange();
                return EXIT_ERROR;
            }
        }
    }
    SysUtils::NotifyFontChange();
    std::cout << "Successfully " << (deleteFile ? "removed" : "uninstalled") << ": " << fontName << "\n";
    if (!deleteFile) std::cout << "Font file remains at: " << fullPath << "\n";
    else std::cout << "File deleted: " << fullPath << "\n";
    return EXIT_SUCCESS_CODE;
}

int InstallFont(const char* fontPath, bool forceAdmin) {
    int result = ValidateInstallPrerequisites(fontPath);
    if (result != EXIT_SUCCESS_CODE) return result;

    // Determine installation type
    bool isAdmin = SysUtils::IsAdmin();
    bool perUser;

    if (forceAdmin) {
        // User explicitly requested system-level installation
        if (!isAdmin) {
            std::cerr << "Error: Administrator privileges required for system-level installation\n";
            std::cerr << "Solution: Right-click Command Prompt and select 'Run as administrator'\n";
            return EXIT_PERMISSION_DENIED;
        }
        perUser = false;
    } else {
        // Auto-detect based on admin privileges (current behavior)
        perUser = !isAdmin;
        if (perUser) {
            std::cout << "Installing font for current user only (no admin privileges)...\n";
        }
    }

    std::string fontName;
    result = ExtractFontName(fontPath, fontName);
    if (result != EXIT_SUCCESS_CODE) return result;
    TryUninstallExistingFont(fontName, forceAdmin);
    std::string destPath;
    if (!SysUtils::CopyToFontsFolder(fontPath, destPath, perUser)) {
        std::cerr << "Error: Failed to copy font file: " << SysUtils::GetLastErrorMessage() << "\n";
        return EXIT_ERROR;
    }
    result = RegisterAndLoadFont(destPath, fontName, perUser);
    if (result != EXIT_SUCCESS_CODE) return result;
    SysUtils::NotifyFontChange();
    std::cout << "Successfully installed: " << fontName << "\n";
    std::cout << "Location: " << destPath << "\n";
    if (perUser) {
        std::cout << "Note: Font installed for current user only\n";
    }
    return EXIT_SUCCESS_CODE;
}

int UninstallFontByPath(const char* fontPath, bool forceAdmin) {
    // Parse font name
    std::string fontName = FontParser::GetFontName(fontPath);
    if (fontName.empty()) {
        std::cerr << "Error: Failed to parse font name\n";
        return EXIT_ERROR;
    }

    return UninstallFontByName(fontName.c_str(), forceAdmin);
}

// Helper: Check if string is empty or contains only whitespace characters
static bool IsEmptyOrWhitespace(const char* str) noexcept {
    if (!str || *str == '\0') return true;
    for (const char* p = str; *p; ++p) {
        if (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') return false;
    }
    return true;
}

int UninstallFontByName(const char* fontName, bool forceAdmin) {
    if (IsEmptyOrWhitespace(fontName)) {
        std::cerr << "Error: Font name cannot be empty\n";
        return EXIT_ERROR;
    }

    // If forceAdmin is set, check admin privileges upfront
    if (forceAdmin && !SysUtils::IsAdmin()) {
        std::cerr << "Error: Administrator privileges required for system-level operation\n";
        std::cerr << "Solution: Right-click Command Prompt and select 'Run as administrator'\n";
        return EXIT_PERMISSION_DENIED;
    }

    std::string fontFile, matchedName;
    bool perUser;
    if (!FindFontInRegistry(fontName, fontFile, matchedName, perUser, forceAdmin)) {
        std::cerr << "Error: Font not found in registry: " << fontName << "\n";
        if (forceAdmin) {
            std::cerr << "Note: Search limited to system fonts (--admin flag used)\n";
        }
        return EXIT_ERROR;
    }

    // For system fonts, check admin privileges
    if (!perUser && !SysUtils::IsAdmin()) {
        std::cerr << "Error: Administrator privileges required for system fonts\n";
        std::cerr << "Solution: Right-click Command Prompt and select 'Run as administrator'\n";
        return EXIT_PERMISSION_DENIED;
    }

    if (!SysUtils::IsValidFontPath(fontFile.c_str())) {
        std::cerr << "Error: Invalid font path in registry: " << fontFile << "\n";
        return EXIT_ERROR;
    }
    return UnloadAndCleanupFont(fontFile, matchedName, fontName, false, perUser);
}

int RemoveFontByPath(const char* fontPath, bool forceAdmin) {
    // Parse font name
    std::string fontName = FontParser::GetFontName(fontPath);
    if (fontName.empty()) {
        std::cerr << "Error: Failed to parse font name\n";
        return EXIT_ERROR;
    }

    return RemoveFontByName(fontName.c_str(), forceAdmin);
}

int RemoveFontByName(const char* fontName, bool forceAdmin) {
    if (IsEmptyOrWhitespace(fontName)) {
        std::cerr << "Error: Font name cannot be empty\n";
        return EXIT_ERROR;
    }

    // If forceAdmin is set, check admin privileges upfront
    if (forceAdmin && !SysUtils::IsAdmin()) {
        std::cerr << "Error: Administrator privileges required for system-level operation\n";
        std::cerr << "Solution: Right-click Command Prompt and select 'Run as administrator'\n";
        return EXIT_PERMISSION_DENIED;
    }

    std::string fontFile, matchedName;
    bool perUser;
    if (!FindFontInRegistry(fontName, fontFile, matchedName, perUser, forceAdmin)) {
        std::cerr << "Error: Font not found in registry: " << fontName << "\n";
        if (forceAdmin) {
            std::cerr << "Note: Search limited to system fonts (--admin flag used)\n";
        }
        return EXIT_ERROR;
    }

    // For system fonts, check admin privileges
    if (!perUser && !SysUtils::IsAdmin()) {
        std::cerr << "Error: Administrator privileges required for system fonts\n";
        std::cerr << "Solution: Right-click Command Prompt and select 'Run as administrator'\n";
        return EXIT_PERMISSION_DENIED;
    }

    if (!SysUtils::IsValidFontPath(fontFile.c_str())) {
        std::cerr << "Error: Invalid font path in registry: " << fontFile << "\n";
        return EXIT_ERROR;
    }
    return UnloadAndCleanupFont(fontFile, matchedName, fontName, true, perUser);
}

int Cleanup(bool includeSystem) {
    std::cout << "Scanning font registry for broken entries...\n";
    int brokenEntries = CleanupRegistry(includeSystem, true);
    bool registryOk = brokenEntries >= 0;
    if (registryOk) {
        std::cout << "Found and removed " << brokenEntries << " broken font entries.\n";
    } else {
        std::cerr << "Error: Failed to scan font registry.\n";
    }

    std::cout << "Clearing font caches...\n";
    bool cachesOk = true;
    if (!SysUtils::ClearUserFontCaches()) {
        std::cerr << "Error: Failed to clear one or more user/third-party caches.\n";
        cachesOk = false;
    }

    if (includeSystem) {
        if (!SysUtils::ClearSystemFontCaches()) {
            std::cerr << "Error: Failed to clear one or more system caches.\n";
            cachesOk = false;
        }
    }

    if (!registryOk || !cachesOk) {
        return EXIT_ERROR;
    }

    std::cout << "Font caches cleared successfully.\n";
    return EXIT_SUCCESS_CODE;
}

} // namespace FontOps
