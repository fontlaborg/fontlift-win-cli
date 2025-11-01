// this_file: src/main.cpp
// fontlift-win-cli - Windows font management CLI tool
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0

#include "exit_codes.h"
#include "font_ops.h"
#include <iostream>
#include <cstring>

void ShowUsage(const char* programName) {
    std::cout << "fontlift - Windows Font Management CLI\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << programName << " <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  list, l              List installed fonts\n";
    std::cout << "    -p                 Show paths (default)\n";
    std::cout << "    -n                 Show internal font names\n";
    std::cout << "    -n -p              Show both (path;name format)\n";
    std::cout << "    -s                 Sort output and remove duplicates\n\n";
    std::cout << "  install, i <path>    Install font from filepath\n";
    std::cout << "    -p <filepath>      Specify font file path\n\n";
    std::cout << "  uninstall, u         Uninstall font (keep file)\n";
    std::cout << "    -p <filepath>      Uninstall by path\n";
    std::cout << "    -n <fontname>      Uninstall by internal name\n\n";
    std::cout << "  remove, rm           Uninstall font (delete file)\n";
    std::cout << "    -p <filepath>      Remove by path\n";
    std::cout << "    -n <fontname>      Remove by internal name\n\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        ShowUsage(argv[0]);
        return EXIT_ERROR;
    }

    const char* command = argv[1];

    // List command
    if (strcmp(command, "list") == 0 || strcmp(command, "l") == 0) {
        bool hasPathFlag = false;
        bool hasNameFlag = false;
        bool hasSortFlag = false;

        // Collect all flags first
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-p") == 0) hasPathFlag = true;
            if (strcmp(argv[i], "-n") == 0) hasNameFlag = true;
            if (strcmp(argv[i], "-s") == 0) hasSortFlag = true;
        }

        // Determine behavior: both flags → both, -n only → names, default/path-only → paths
        bool showPaths = hasPathFlag || !hasNameFlag;  // default to paths
        bool showNames = hasNameFlag;

        return FontOps::ListFonts(showPaths, showNames, hasSortFlag);
    }

    // Install command
    if (strcmp(command, "install") == 0 || strcmp(command, "i") == 0) {
        const char* filepath = nullptr;

        if (argc == 3 && argv[2][0] != '-') {
            filepath = argv[2];
        } else {
            for (int i = 2; i < argc - 1; i++) {
                if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
                    filepath = argv[i + 1];
                    break;
                }
            }
        }

        if (!filepath) {
            std::cerr << "Error: No font file specified\n";
            ShowUsage(argv[0]);
            return EXIT_ERROR;
        }

        return FontOps::InstallFont(filepath);
    }

    // Uninstall command
    if (strcmp(command, "uninstall") == 0 || strcmp(command, "u") == 0) {
        const char* filepath = nullptr;
        const char* fontname = nullptr;

        for (int i = 2; i < argc - 1; i++) {
            if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
                filepath = argv[i + 1];
            } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
                fontname = argv[i + 1];
            }
        }

        if (!filepath && !fontname) {
            std::cerr << "Error: Must specify -p <path> or -n <name>\n";
            ShowUsage(argv[0]);
            return EXIT_ERROR;
        }

        if (filepath) {
            return FontOps::UninstallFontByPath(filepath);
        } else {
            return FontOps::UninstallFontByName(fontname);
        }
    }

    // Remove command
    if (strcmp(command, "remove") == 0 || strcmp(command, "rm") == 0) {
        const char* filepath = nullptr;
        const char* fontname = nullptr;

        for (int i = 2; i < argc - 1; i++) {
            if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
                filepath = argv[i + 1];
            } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
                fontname = argv[i + 1];
            }
        }

        if (!filepath && !fontname) {
            std::cerr << "Error: Must specify -p <path> or -n <name>\n";
            ShowUsage(argv[0]);
            return EXIT_ERROR;
        }

        if (filepath) {
            return FontOps::RemoveFontByPath(filepath);
        } else {
            return FontOps::RemoveFontByName(fontname);
        }
    }

    // Unknown command
    std::cerr << "Error: Unknown command '" << command << "'\n";
    ShowUsage(argv[0]);
    return EXIT_ERROR;
}
