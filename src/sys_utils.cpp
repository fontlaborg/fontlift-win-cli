// this_file: src/sys_utils.cpp
// System utilities implementation
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0

#include "sys_utils.h"
#include <windows.h>
#include <shlwapi.h>
#include <iostream>

namespace SysUtils {

std::string GetLastErrorMessage() {
    DWORD error = GetLastError();
    if (error == 0) return "";

    char* msgBuf = nullptr;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL, error, 0, (LPSTR)&msgBuf, 0, NULL);

    std::string message = msgBuf ? msgBuf : "";
    if (msgBuf) LocalFree(msgBuf);

    // Remove trailing newlines
    while (!message.empty() && (message.back() == '\n' || message.back() == '\r')) {
        message.pop_back();
    }

    return " (Error " + std::to_string(error) + ": " + message + ")";
}

bool IsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuthority, 2,
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin != FALSE;
}

std::string GetFontsDirectory() {
    char winDir[MAX_PATH];
    if (GetWindowsDirectoryA(winDir, MAX_PATH) == 0) {
        return "";
    }
    std::string fontsDir = winDir;
    fontsDir += "\\Fonts";
    return fontsDir;
}

bool CopyToFontsFolder(const char* sourcePath, std::string& destPath) {
    std::string fontsDir = GetFontsDirectory();
    if (fontsDir.empty()) return false;

    std::string filename = GetFileName(sourcePath);
    destPath = fontsDir + "\\" + filename;

    return CopyFileA(sourcePath, destPath.c_str(), FALSE) != 0;
}

bool DeleteFromFontsFolder(const char* filename) {
    std::string fontsDir = GetFontsDirectory();
    if (fontsDir.empty()) return false;

    std::string fullPath = fontsDir + "\\" + filename;
    return DeleteFileA(fullPath.c_str()) != 0;
}

bool FileExists(const char* path) {
    return PathFileExistsA(path) != FALSE;
}

std::string GetFileName(const char* path) {
    const char* filename = PathFindFileNameA(path);
    return filename ? std::string(filename) : "";
}

bool IsValidFontPath(const char* path) {
    if (!path || strlen(path) == 0) return false;

    // Check for path traversal attempts
    std::string pathStr(path);
    if (pathStr.find("..\\") != std::string::npos ||
        pathStr.find("../") != std::string::npos) {
        return false;
    }

    // Reject absolute paths outside fonts directory
    if (pathStr.length() > 1 && pathStr[1] == ':') {
        // Absolute path - ensure it's in fonts directory
        std::string fontsDir = GetFontsDirectory();
        if (fontsDir.empty()) return false;

        // Convert to lowercase for comparison
        std::string lowerPath = pathStr;
        std::string lowerFonts = fontsDir;
        for (auto& c : lowerPath) c = tolower(c);
        for (auto& c : lowerFonts) c = tolower(c);

        // Path must start with fonts directory
        if (lowerPath.find(lowerFonts) != 0) {
            return false;
        }
    }

    return true;
}

bool RegReadFontEntry(const char* valueName, std::string& fontFile) {
    HKEY hKey;
    const char* regPath = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    char buffer[512];
    DWORD bufferSize = sizeof(buffer);
    DWORD type;

    LONG result = RegQueryValueExA(hKey, valueName, NULL, &type, (LPBYTE)buffer, &bufferSize);
    RegCloseKey(hKey);

    if (result == ERROR_SUCCESS && type == REG_SZ) {
        fontFile = buffer;
        return true;
    }
    return false;
}

bool RegWriteFontEntry(const char* valueName, const char* fontFile) {
    HKEY hKey;
    const char* regPath = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPath, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    LONG result = RegSetValueExA(hKey, valueName, 0, REG_SZ,
        (const BYTE*)fontFile, (DWORD)strlen(fontFile) + 1);

    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool RegDeleteFontEntry(const char* valueName) {
    HKEY hKey;
    const char* regPath = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPath, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    LONG result = RegDeleteValueA(hKey, valueName);
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool RegEnumerateFonts(void (*callback)(const char* name, const char* file)) {
    HKEY hKey;
    const char* regPath = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    char valueName[512];
    BYTE valueData[512];
    DWORD index = 0;

    while (true) {
        DWORD nameSize = sizeof(valueName);
        DWORD dataSize = sizeof(valueData);
        DWORD type;

        LONG result = RegEnumValueA(hKey, index++, valueName, &nameSize,
            NULL, &type, valueData, &dataSize);

        if (result == ERROR_NO_MORE_ITEMS) break;
        if (result != ERROR_SUCCESS) continue;
        if (type != REG_SZ) continue;

        callback(valueName, (const char*)valueData);
    }

    RegCloseKey(hKey);
    return true;
}

void NotifyFontChange() {
    SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
}

} // namespace SysUtils
