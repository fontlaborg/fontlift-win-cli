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
    UINT result = GetWindowsDirectoryA(winDir, MAX_PATH);
    if (result == 0 || result >= MAX_PATH) {
        return "";  // Failed or path truncated
    }
    std::string fontsDir = winDir;
    fontsDir += "\\Fonts";
    return fontsDir;
}

std::string GetUserFontsDirectory() {
    char localAppData[MAX_PATH];
    DWORD result = GetEnvironmentVariableA("LOCALAPPDATA", localAppData, MAX_PATH);
    if (result == 0 || result >= MAX_PATH) {
        return "";  // Failed or path truncated
    }
    std::string fontsDir = localAppData;
    fontsDir += "\\Microsoft\\Windows\\Fonts";
    return fontsDir;
}

bool CopyToFontsFolder(const char* sourcePath, std::string& destPath, bool perUser) {
    std::string fontsDir = perUser ? GetUserFontsDirectory() : GetFontsDirectory();
    if (fontsDir.empty()) return false;

    // Ensure the user fonts directory exists
    if (perUser) {
        if (!PathFileExistsA(fontsDir.c_str())) {
            if (CreateDirectoryA(fontsDir.c_str(), NULL) == 0) {
                // Failed to create directory (could be permissions issue)
                return false;
            }
        }
    }

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
    if (!path || path[0] == '\0') return "";
    const char* filename = PathFindFileNameA(path);
    return filename ? std::string(filename) : "";
}

static bool HasPathTraversal(const std::string& path) {
    return path.find("..\\") != std::string::npos || path.find("../") != std::string::npos;
}

static bool IsAbsolutePathInFontsDir(const std::string& pathStr) {
    if (pathStr.length() <= 1 || pathStr[1] != ':') return true;

    std::string fontsDir = GetFontsDirectory();
    if (fontsDir.empty()) return false;

    std::string lowerPath = pathStr;
    std::string lowerFonts = fontsDir;
    for (auto& c : lowerPath) c = tolower(c);
    for (auto& c : lowerFonts) c = tolower(c);

    return lowerPath.find(lowerFonts) == 0;
}

bool IsValidFontPath(const char* path) {
    if (!path || strlen(path) == 0) return false;
    std::string pathStr(path);
    if (HasPathTraversal(pathStr)) return false;
    return IsAbsolutePathInFontsDir(pathStr);
}

bool RegReadFontEntry(const char* valueName, std::string& fontFile, bool perUser) {
    HKEY hKey;
    const char* regPath = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
    HKEY rootKey = perUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;

    if (RegOpenKeyExA(rootKey, regPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
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

bool RegWriteFontEntry(const char* valueName, const char* fontFile, bool perUser) {
    HKEY hKey;
    const char* regPath = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
    HKEY rootKey = perUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;

    // For per-user installation, create the registry key if it doesn't exist
    DWORD disposition;
    LONG openResult = perUser
        ? RegCreateKeyExA(rootKey, regPath, 0, NULL, 0, KEY_WRITE, NULL, &hKey, &disposition)
        : RegOpenKeyExA(rootKey, regPath, 0, KEY_WRITE, &hKey);

    if (openResult != ERROR_SUCCESS) {
        return false;
    }

    LONG result = RegSetValueExA(hKey, valueName, 0, REG_SZ,
        (const BYTE*)fontFile, (DWORD)strlen(fontFile) + 1);

    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool RegDeleteFontEntry(const char* valueName, bool perUser) {
    HKEY hKey;
    const char* regPath = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
    HKEY rootKey = perUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;

    if (RegOpenKeyExA(rootKey, regPath, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    LONG result = RegDeleteValueA(hKey, valueName);
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool RegEnumerateFonts(void (*callback)(const char* name, const char* file), bool perUser) {
    HKEY hKey;
    const char* regPath = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
    HKEY rootKey = perUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;

    if (RegOpenKeyExA(rootKey, regPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
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
