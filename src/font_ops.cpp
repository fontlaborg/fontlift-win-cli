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

namespace FontOps {

// Helper: global context for registry enumeration callback
static struct {
    bool showPaths;
    bool showNames;
    bool sorted;
    std::string fontsDir;
    std::string userFontsDir;
    std::set<std::string>* outputSet;  // For sorted/unique output
} g_listContext;

static std::string ResolveFullPath(const char* file) {
    size_t fileLen = strlen(file);
    bool isAbsolute = (fileLen > 0 && file[0] == '\\') || (fileLen > 1 && file[1] == ':');
    // Absolute paths are used for per-user fonts, relative paths for system fonts
    return isAbsolute ? file : g_listContext.fontsDir + "\\" + file;
}

static std::string FormatOutput(const std::string& path, const char* name) {
    if (g_listContext.showPaths && g_listContext.showNames) return path + ";" + name;
    if (g_listContext.showNames) return name;
    return path;
}

static void ListCallback(const char* name, const char* file) {
    std::string fullPath = ResolveFullPath(file);
    std::string output = FormatOutput(fullPath, name);

    if (g_listContext.sorted) {
        g_listContext.outputSet->insert(output);
    } else {
        std::cout << output << "\n";
    }
}

static void OutputSorted(const std::set<std::string>& outputSet) {
    for (const auto& line : outputSet) {
        std::cout << line << "\n";
    }
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

// Helper: Validate prerequisites for font installation
static bool HasValidFontExtension(const char* path) {
    const char* validExts[] = {".ttf", ".otf", ".ttc", ".otc"};
    std::string pathStr(path);
    for (auto& c : pathStr) c = tolower(c);
    for (const char* ext : validExts) {
        if (pathStr.length() >= strlen(ext) &&
            pathStr.compare(pathStr.length() - strlen(ext), strlen(ext), ext) == 0) {
            return true;
        }
    }
    return false;
}

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

// Helper: Find font entry in registry with name variant matching
static bool FindFontInRegistry(const char* fontName, std::string& outFile, std::string& outRegName, bool& outPerUser, bool forceSystemOnly = false) {
    std::vector<std::string> variants = {
        std::string(fontName) + " (TrueType)",
        std::string(fontName) + " (OpenType)",
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

// Helper: Register font and load into system
static int RegisterAndLoadFont(const std::string& destPath, const std::string& fontName, bool perUser) {
    std::string regValue = perUser ? destPath : SysUtils::GetFileName(destPath.c_str());
    std::string regName = fontName + " (TrueType)";
    std::string existingFile;
    if (SysUtils::RegReadFontEntry(regName.c_str(), existingFile, perUser)) {
        std::cerr << "Warning: Font '" << fontName << "' already installed, overwriting...\n";
    }
    if (!SysUtils::RegWriteFontEntry(regName.c_str(), regValue.c_str(), perUser)) {
        std::cerr << "Error: Failed to register font in registry" << SysUtils::GetLastErrorMessage() << "\n";
        SysUtils::DeleteFromFontsFolder(SysUtils::GetFileName(destPath.c_str()).c_str());
        return EXIT_ERROR;
    }
    if (AddFontResourceExA(destPath.c_str(), FR_PRIVATE, 0) == 0) {
        std::cerr << "Error: Failed to load font resource" << SysUtils::GetLastErrorMessage() << "\n";
        SysUtils::RegDeleteFontEntry(regName.c_str(), perUser);
        SysUtils::DeleteFromFontsFolder(SysUtils::GetFileName(destPath.c_str()).c_str());
        return EXIT_ERROR;
    }
    return EXIT_SUCCESS_CODE;
}

// Helper: Unload font and cleanup registry (shared by Uninstall and Remove)
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
    std::string destPath;
    if (!SysUtils::CopyToFontsFolder(fontPath, destPath, perUser)) {
        std::cerr << "Error: Failed to copy font file" << SysUtils::GetLastErrorMessage() << "\n";
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

static bool IsEmptyOrWhitespace(const char* str) {
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

} // namespace FontOps
