// this_file: src/main.cpp
// fontlift-win-cli - Windows font management CLI tool
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0

#include <iostream>
#include <cstring>

// Exit codes
constexpr int EXIT_SUCCESS_CODE = 0;
constexpr int EXIT_ERROR = 1;
constexpr int EXIT_PERMISSION_DENIED = 2;

void ShowUsage(const char* programName) {
    std::cout << "fontlift - Windows Font Management CLI\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << programName << " <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  list, l              List installed fonts\n";
    std::cout << "    -p                 Show paths (default)\n";
    std::cout << "    -n                 Show internal font names\n";
    std::cout << "    -n -p              Show both (path;name format)\n\n";
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
        bool showPaths = true;  // default
        bool showNames = false;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-n") == 0) {
                showNames = true;
                showPaths = false;  // -n alone means names only
            }
            if (strcmp(argv[i], "-p") == 0 && showNames) {
                showPaths = true;  // -n -p means both
            }
        }

        std::cout << "List fonts: paths=" << showPaths
                  << " names=" << showNames << "\n";
        std::cout << "[NOT IMPLEMENTED YET]\n";
        return EXIT_SUCCESS_CODE;
    }

    // Install command
    if (strcmp(command, "install") == 0 || strcmp(command, "i") == 0) {
        const char* filepath = nullptr;

        if (argc == 3 && argv[2][0] != '-') {
            filepath = argv[2];
        } else {
            for (int i = 2; i < argc - 1; i++) {
                if (strcmp(argv[i], "-p") == 0) {
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

        std::cout << "Install font: " << filepath << "\n";
        std::cout << "[NOT IMPLEMENTED YET]\n";
        return EXIT_SUCCESS_CODE;
    }

    // Uninstall command
    if (strcmp(command, "uninstall") == 0 || strcmp(command, "u") == 0) {
        const char* filepath = nullptr;
        const char* fontname = nullptr;

        for (int i = 2; i < argc - 1; i++) {
            if (strcmp(argv[i], "-p") == 0) {
                filepath = argv[i + 1];
            } else if (strcmp(argv[i], "-n") == 0) {
                fontname = argv[i + 1];
            }
        }

        if (!filepath && !fontname) {
            std::cerr << "Error: Must specify -p <path> or -n <name>\n";
            ShowUsage(argv[0]);
            return EXIT_ERROR;
        }

        if (filepath) {
            std::cout << "Uninstall font by path: " << filepath << "\n";
        } else {
            std::cout << "Uninstall font by name: " << fontname << "\n";
        }
        std::cout << "[NOT IMPLEMENTED YET]\n";
        return EXIT_SUCCESS_CODE;
    }

    // Remove command
    if (strcmp(command, "remove") == 0 || strcmp(command, "rm") == 0) {
        const char* filepath = nullptr;
        const char* fontname = nullptr;

        for (int i = 2; i < argc - 1; i++) {
            if (strcmp(argv[i], "-p") == 0) {
                filepath = argv[i + 1];
            } else if (strcmp(argv[i], "-n") == 0) {
                fontname = argv[i + 1];
            }
        }

        if (!filepath && !fontname) {
            std::cerr << "Error: Must specify -p <path> or -n <name>\n";
            ShowUsage(argv[0]);
            return EXIT_ERROR;
        }

        if (filepath) {
            std::cout << "Remove font by path: " << filepath << "\n";
        } else {
            std::cout << "Remove font by name: " << fontname << "\n";
        }
        std::cout << "[NOT IMPLEMENTED YET]\n";
        return EXIT_SUCCESS_CODE;
    }

    // Unknown command
    std::cerr << "Error: Unknown command '" << command << "'\n";
    ShowUsage(argv[0]);
    return EXIT_ERROR;
}
