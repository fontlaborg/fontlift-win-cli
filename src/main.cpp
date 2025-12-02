// this_file: src/main.cpp
// fontlift-win-cli - Windows font management CLI tool
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0

#include "exit_codes.h"
#include "font_ops.h"
#include "sys_utils.h"
#include <windows.h>
#include <cstring>
#include <iostream>
#include <vector>

#pragma comment(lib, "version.lib")

void ShowUsage(const char* programName) {
    std::cout << "fontlift-win - Windows Font Management CLI\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << programName << " <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  list, l              List installed fonts\n";
    std::cout << "    -p                 Show paths (default, sorted)\n";
    std::cout << "    -n                 Show internal font names (sorted)\n";
    std::cout << "    -n -p              Show both (path::name format, sorted)\n";
    std::cout << "                       Path-only output removes duplicate paths\n";
    std::cout << "    -s                 (Optional) Kept for compatibility; output is always sorted\n\n";
    std::cout << "  install, i <path>    Install font from filepath\n";
    std::cout << "    -p <filepath>      Specify font file path\n";
    std::cout << "    --admin, -a        Force system-level installation (requires admin)\n\n";
    std::cout << "  uninstall, u         Uninstall font (keep file)\n";
    std::cout << "    -p <filepath>      Uninstall by path\n";
    std::cout << "    -n <fontname>      Uninstall by internal name\n";
    std::cout << "    --admin, -a        Include system-level uninstallation (requires admin)\n\n";
    std::cout << "  remove, rm           Uninstall font (delete file)\n";
    std::cout << "    -p <filepath>      Remove by path\n";
    std::cout << "    -n <fontname>      Remove by internal name\n";
    std::cout << "    --admin, -a        Include system-level removal (requires admin)\n\n";
    std::cout << "  cleanup, c           Cleanup registry entries and font caches\n";
    std::cout << "    --admin, -a        Include system-wide cleanup (requires admin)\n";
    std::cout << "                      - Removes registry entries pointing to missing files\n";
    std::cout << "                      - Clears user and third-party font caches\n";
    std::cout << "                      - With --admin: clears system font caches\n\n";
    std::cout << "made by FontLab https://www.fontlab.com/\n";
}

static bool ExtractVersionInfo(WORD& major, WORD& minor, WORD& patch) noexcept {
    char filename[MAX_PATH];
    DWORD result = GetModuleFileNameA(NULL, filename, MAX_PATH);
    if (result == 0 || result >= MAX_PATH) return false;  // Failed or path truncated

    DWORD handle;
    DWORD size = GetFileVersionInfoSizeA(filename, &handle);
    if (size == 0) return false;

    std::vector<BYTE> buffer(size);
    if (!GetFileVersionInfoA(filename, 0, size, buffer.data())) return false;

    VS_FIXEDFILEINFO* fileInfo;
    UINT len;
    if (!VerQueryValueA(buffer.data(), "\\", reinterpret_cast<LPVOID*>(&fileInfo), &len)) return false;

    major = HIWORD(fileInfo->dwFileVersionMS);
    minor = LOWORD(fileInfo->dwFileVersionMS);
    patch = HIWORD(fileInfo->dwFileVersionLS);
    return true;
}

static int HandleVersionCommand() {
    WORD major, minor, patch;
    if (ExtractVersionInfo(major, minor, patch)) {
        std::cout << "fontlift-win version " << major << "." << minor << "." << patch << "\n";
    } else {
        std::cout << "fontlift-win version unknown\n";
    }
    return EXIT_SUCCESS_CODE;
}

static int HandleListCommand(int argc, char* argv[]) {
    bool hasPathFlag = false, hasNameFlag = false, sawSortFlag = false;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) hasPathFlag = true;
        if (strcmp(argv[i], "-n") == 0) hasNameFlag = true;
        if (strcmp(argv[i], "-s") == 0) sawSortFlag = true;  // Backward compatibility; always sorted
    }
    bool showPaths = hasPathFlag || !hasNameFlag;
    bool showNames = hasNameFlag;
    (void)sawSortFlag;  // Suppress unused warning; sorting is now default
    return FontOps::ListFonts(showPaths, showNames);
}

static int HandleInstallCommand(int argc, char* argv[], const char* progName) {
    const char* filepath = nullptr;
    bool forceAdmin = false;

    // Parse flags
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--admin") == 0 || strcmp(argv[i], "-a") == 0) {
            forceAdmin = true;
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            filepath = argv[i + 1];
            i++; // Skip the next argument as it's the filepath
        } else if (argv[i][0] != '-') {
            filepath = argv[i];
        }
    }

    if (!filepath || filepath[0] == '\0') {
        std::cerr << "Error: No font file specified\n";
        ShowUsage(progName);
        return EXIT_ERROR;
    }
    return FontOps::InstallFont(filepath, forceAdmin);
}

static int HandleUninstallOrRemove(int argc, char* argv[], const char* progName, bool deleteFile) {
    const char* filepath = nullptr;
    const char* fontname = nullptr;
    bool forceAdmin = false;

    // Parse flags
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--admin") == 0 || strcmp(argv[i], "-a") == 0) {
            forceAdmin = true;
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            filepath = argv[i + 1];
            i++; // Skip the next argument
        } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            fontname = argv[i + 1];
            i++; // Skip the next argument
        }
    }

    // Validate arguments are non-empty
    if (filepath && filepath[0] == '\0') filepath = nullptr;
    if (fontname && fontname[0] == '\0') fontname = nullptr;

    if (!filepath && !fontname) {
        std::cerr << "Error: Must specify -p <path> or -n <name>\n";
        ShowUsage(progName);
        return EXIT_ERROR;
    }
    if (filepath) {
        return deleteFile ? FontOps::RemoveFontByPath(filepath, forceAdmin) : FontOps::UninstallFontByPath(filepath, forceAdmin);
    } else {
        return deleteFile ? FontOps::RemoveFontByName(fontname, forceAdmin) : FontOps::UninstallFontByName(fontname, forceAdmin);
    }
}

static int HandleCleanupCommand(int argc, char* argv[]) {
    bool includeSystem = false;
    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "--admin") == 0 || strcmp(argv[i], "-a") == 0) {
            includeSystem = true;
        } else if (argv[i][0] == '-') {
            std::cerr << "Warning: Unknown option for cleanup command: " << argv[i] << "\n";
        }
    }

    if (includeSystem && !SysUtils::IsAdmin()) {
        std::cerr << "Error: Administrator privileges are required for --admin cleanup.\n";
        std::cerr << "Solution: Right-click Command Prompt and select 'Run as administrator'.\n";
        return EXIT_PERMISSION_DENIED;
    }

    std::cout << "Starting " << (includeSystem ? "system" : "user") << " cleanup...\n";
    int result = FontOps::Cleanup(includeSystem);
    if (result == EXIT_SUCCESS_CODE) {
        std::cout << (includeSystem ? "System" : "User") << " cleanup completed successfully.\n";
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        ShowUsage(argv[0]);
        return EXIT_ERROR;
    }

    const char* command = argv[1];

    // Version command: Failure to extract version returns "unknown" gracefully (no error handling needed)
    if (strcmp(command, "--version") == 0 || strcmp(command, "-v") == 0) {
        return HandleVersionCommand();
    }

    if (strcmp(command, "list") == 0 || strcmp(command, "l") == 0) {
        return HandleListCommand(argc, argv);
    }

    if (strcmp(command, "install") == 0 || strcmp(command, "i") == 0) {
        return HandleInstallCommand(argc, argv, argv[0]);
    }

    if (strcmp(command, "uninstall") == 0 || strcmp(command, "u") == 0) {
        return HandleUninstallOrRemove(argc, argv, argv[0], false);
    }

    if (strcmp(command, "remove") == 0 || strcmp(command, "rm") == 0) {
        return HandleUninstallOrRemove(argc, argv, argv[0], true);
    }

    if (strcmp(command, "cleanup") == 0 || strcmp(command, "c") == 0) {
        return HandleCleanupCommand(argc, argv);
    }

    std::cerr << "Error: Unknown command '" << command << "'\n";
    ShowUsage(argv[0]);
    return EXIT_ERROR;
}
