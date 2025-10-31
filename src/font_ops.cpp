// this_file: src/font_ops.cpp
// Font operations implementation
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0

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

constexpr int EXIT_SUCCESS_CODE = 0;
constexpr int EXIT_ERROR = 1;
constexpr int EXIT_PERMISSION_DENIED = 2;

// Helper: global context for registry enumeration callback
static struct {
    bool showPaths;
    bool showNames;
    bool sorted;
    std::string fontsDir;
    std::set<std::string>* outputSet;  // For sorted/unique output
} g_listContext;

static std::string ResolveFullPath(const char* file) {
    size_t fileLen = strlen(file);
    bool isAbsolute = (fileLen > 0 && file[0] == '\\') || (fileLen > 1 && file[1] == ':');
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
    if (g_listContext.fontsDir.empty()) {
        std::cerr << "Error: Cannot determine fonts directory\n";
        return EXIT_ERROR;
    }
    std::set<std::string> outputSet;
    if (sorted) g_listContext.outputSet = &outputSet;
    if (!SysUtils::RegEnumerateFonts(ListCallback)) {
        std::cerr << "Error: Failed to enumerate fonts\n";
        return EXIT_ERROR;
    }
    if (sorted) OutputSorted(outputSet);
    return EXIT_SUCCESS_CODE;
}

int InstallFont(const char* fontPath) {
    // Check admin privileges
    if (!SysUtils::IsAdmin()) {
        std::cerr << "Error: Administrator privileges required\n";
        std::cerr << "Please run from an elevated command prompt\n";
        return EXIT_PERMISSION_DENIED;
    }

    // Check if file exists
    if (!SysUtils::FileExists(fontPath)) {
        std::cerr << "Error: Font file not found: " << fontPath << "\n";
        return EXIT_ERROR;
    }

    // Get font name
    std::string fontName;
    if (FontParser::IsCollection(fontPath)) {
        std::vector<std::string> names = FontParser::GetFontsInCollection(fontPath);
        if (names.empty()) {
            std::cerr << "Error: Failed to parse font collection\n";
            return EXIT_ERROR;
        }
        fontName = names[0];  // Use first font in collection
        if (names.size() > 1) {
            std::cout << "Note: Collection contains " << names.size() << " fonts\n";
        }
    } else {
        fontName = FontParser::GetFontName(fontPath);
        if (fontName.empty()) {
            std::cerr << "Error: Failed to parse font name\n";
            return EXIT_ERROR;
        }
    }

    // Copy to fonts folder
    std::string destPath;
    if (!SysUtils::CopyToFontsFolder(fontPath, destPath)) {
        std::cerr << "Error: Failed to copy font file\n";
        return EXIT_ERROR;
    }

    std::string filename = SysUtils::GetFileName(destPath.c_str());

    // Determine font type and registry value name
    std::string regName = fontName + " (TrueType)";

    // Add to registry
    if (!SysUtils::RegWriteFontEntry(regName.c_str(), filename.c_str())) {
        std::cerr << "Error: Failed to register font in registry\n";
        SysUtils::DeleteFromFontsFolder(filename.c_str());  // Cleanup
        return EXIT_ERROR;
    }

    // Load font into system
    if (AddFontResourceExA(destPath.c_str(), FR_PRIVATE, 0) == 0) {
        std::cerr << "Warning: Failed to load font resource\n";
    }

    // Notify system
    SysUtils::NotifyFontChange();

    std::cout << "Successfully installed: " << fontName << "\n";
    std::cout << "Location: " << destPath << "\n";

    return EXIT_SUCCESS_CODE;
}

int UninstallFontByPath(const char* fontPath) {
    if (!SysUtils::IsAdmin()) {
        std::cerr << "Error: Administrator privileges required\n";
        return EXIT_PERMISSION_DENIED;
    }

    // Parse font name
    std::string fontName = FontParser::GetFontName(fontPath);
    if (fontName.empty()) {
        std::cerr << "Error: Failed to parse font name\n";
        return EXIT_ERROR;
    }

    return UninstallFontByName(fontName.c_str());
}

int UninstallFontByName(const char* fontName) {
    if (!SysUtils::IsAdmin()) {
        std::cerr << "Error: Administrator privileges required\n";
        return EXIT_PERMISSION_DENIED;
    }

    // Try different registry name formats
    std::vector<std::string> nameVariants = {
        std::string(fontName) + " (TrueType)",
        std::string(fontName) + " (OpenType)",
        std::string(fontName)
    };

    bool found = false;
    std::string fontFile;
    std::string matchedName;

    for (const auto& variant : nameVariants) {
        if (SysUtils::RegReadFontEntry(variant.c_str(), fontFile)) {
            found = true;
            matchedName = variant;
            break;
        }
    }

    if (!found) {
        std::cerr << "Error: Font not found in registry: " << fontName << "\n";
        return EXIT_ERROR;
    }

    // Remove font resource from system
    std::string fontsDir = SysUtils::GetFontsDirectory();
    std::string fullPath = fontsDir + "\\" + fontFile;

    if (RemoveFontResourceExA(fullPath.c_str(), FR_PRIVATE, 0) == 0) {
        std::cerr << "Warning: Failed to unload font resource\n";
    }

    // Remove from registry
    if (!SysUtils::RegDeleteFontEntry(matchedName.c_str())) {
        std::cerr << "Error: Failed to remove font from registry\n";
        return EXIT_ERROR;
    }

    // Notify system
    SysUtils::NotifyFontChange();

    std::cout << "Successfully uninstalled: " << fontName << "\n";
    std::cout << "Font file remains at: " << fullPath << "\n";

    return EXIT_SUCCESS_CODE;
}

int RemoveFontByPath(const char* fontPath) {
    if (!SysUtils::IsAdmin()) {
        std::cerr << "Error: Administrator privileges required\n";
        return EXIT_PERMISSION_DENIED;
    }

    // Parse font name
    std::string fontName = FontParser::GetFontName(fontPath);
    if (fontName.empty()) {
        std::cerr << "Error: Failed to parse font name\n";
        return EXIT_ERROR;
    }

    return RemoveFontByName(fontName.c_str());
}

int RemoveFontByName(const char* fontName) {
    if (!SysUtils::IsAdmin()) {
        std::cerr << "Error: Administrator privileges required\n";
        return EXIT_PERMISSION_DENIED;
    }

    // Try different registry name formats
    std::vector<std::string> nameVariants = {
        std::string(fontName) + " (TrueType)",
        std::string(fontName) + " (OpenType)",
        std::string(fontName)
    };

    bool found = false;
    std::string fontFile;
    std::string matchedName;

    for (const auto& variant : nameVariants) {
        if (SysUtils::RegReadFontEntry(variant.c_str(), fontFile)) {
            found = true;
            matchedName = variant;
            break;
        }
    }

    if (!found) {
        std::cerr << "Error: Font not found in registry: " << fontName << "\n";
        return EXIT_ERROR;
    }

    // Remove font resource from system
    std::string fontsDir = SysUtils::GetFontsDirectory();
    std::string fullPath = fontsDir + "\\" + fontFile;

    if (RemoveFontResourceExA(fullPath.c_str(), FR_PRIVATE, 0) == 0) {
        std::cerr << "Warning: Failed to unload font resource\n";
    }

    // Remove from registry
    if (!SysUtils::RegDeleteFontEntry(matchedName.c_str())) {
        std::cerr << "Error: Failed to remove font from registry\n";
        return EXIT_ERROR;
    }

    // Delete file
    if (!SysUtils::DeleteFromFontsFolder(fontFile.c_str())) {
        std::cerr << "Error: Failed to delete font file: " << fullPath << "\n";
        std::cerr << "Font has been uninstalled but file remains\n";
        SysUtils::NotifyFontChange();
        return EXIT_ERROR;
    }

    // Notify system
    SysUtils::NotifyFontChange();

    std::cout << "Successfully removed: " << fontName << "\n";
    std::cout << "File deleted: " << fullPath << "\n";

    return EXIT_SUCCESS_CODE;
}

} // namespace FontOps
