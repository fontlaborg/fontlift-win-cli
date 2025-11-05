// this_file: src/sys_utils.cpp
// System utilities implementation
// Copyright 2025 by Fontlab Ltd. Licensed under Apache 2.0

#include "sys_utils.h"
#include <windows.h>
#include <winsvc.h>
#include <shlwapi.h>
#include <filesystem>
#include <system_error>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace {
constexpr DWORD FONT_CACHE_TIMEOUT_MS = 30000;
constexpr DWORD SERVICE_POLL_INTERVAL_MS = 500;
constexpr const wchar_t* FONT_CACHE_SERVICE_NAME = L"FontCache";
constexpr const char* SYSTEM_CACHE_FILE = "C:\\Windows\\System32\\FNTCACHE.DAT";
constexpr const char* SERVICE_CACHE_DIR = "C:\\Windows\\ServiceProfiles\\LocalService\\AppData\\Local\\FontCache";
constexpr const char* ADOBE_CACHE_PREFIX = "AdobeFnt";
constexpr const char* ADOBE_CACHE_EXTENSION = ".lst";

struct ScopedServiceHandles {
    SC_HANDLE manager{nullptr};
    SC_HANDLE service{nullptr};
    ~ScopedServiceHandles() {
        if (service) CloseServiceHandle(service);
        if (manager) CloseServiceHandle(manager);
    }
};
} // namespace

namespace {
std::string GetEnvVariable(const char* name) {
    DWORD required = GetEnvironmentVariableA(name, nullptr, 0);
    if (required == 0) return "";
    std::string value(required, '\0');
    DWORD written = GetEnvironmentVariableA(name, value.data(), required);
    if (written == 0 || written >= required) return "";
    value.resize(written);
    return value;
}

bool OpenFontCacheService(DWORD desiredAccess, ScopedServiceHandles& handles) {
    handles.manager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!handles.manager) {
        std::cerr << "    Warning: Unable to open service manager" << SysUtils::GetLastErrorMessage() << "\n";
        return false;
    }
    handles.service = OpenServiceW(handles.manager, FONT_CACHE_SERVICE_NAME, desiredAccess | SERVICE_QUERY_STATUS);
    if (!handles.service) {
        std::cerr << "    Warning: Unable to open FontCache service" << SysUtils::GetLastErrorMessage() << "\n";
        return false;
    }
    return true;
}

bool WaitForServiceState(SC_HANDLE service, DWORD desiredState, DWORD timeoutMs) {
    DWORD waited = 0;
    SERVICE_STATUS_PROCESS status{};
    DWORD bytesNeeded = 0;
    while (waited <= timeoutMs) {
        if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
                reinterpret_cast<LPBYTE>(&status), sizeof(status), &bytesNeeded)) {
            std::cerr << "    Warning: QueryServiceStatusEx failed" << SysUtils::GetLastErrorMessage() << "\n";
            return false;
        }
        if (status.dwCurrentState == desiredState) return true;
        Sleep(SERVICE_POLL_INTERVAL_MS);
        waited += SERVICE_POLL_INTERVAL_MS;
    }
    std::cerr << "    Warning: Service did not reach desired state within timeout.\n";
    return false;
}

bool StopFontCacheService() {
    ScopedServiceHandles handles;
    if (!OpenFontCacheService(SERVICE_STOP, handles)) return false;
    SERVICE_STATUS_PROCESS status{};
    DWORD bytesNeeded = 0;
    if (!QueryServiceStatusEx(handles.service, SC_STATUS_PROCESS_INFO,
            reinterpret_cast<LPBYTE>(&status), sizeof(status), &bytesNeeded)) {
        std::cerr << "    Warning: QueryServiceStatusEx failed" << SysUtils::GetLastErrorMessage() << "\n";
        return false;
    }
    if (status.dwCurrentState == SERVICE_STOPPED) return true;
    if (!ControlService(handles.service, SERVICE_CONTROL_STOP, reinterpret_cast<LPSERVICE_STATUS>(&status))) {
        DWORD error = GetLastError();
        if (error != ERROR_SERVICE_NOT_ACTIVE) {
            std::cerr << "    Warning: Failed to stop FontCache service" << SysUtils::GetLastErrorMessage() << "\n";
            return false;
        }
    }
    return WaitForServiceState(handles.service, SERVICE_STOPPED, FONT_CACHE_TIMEOUT_MS);
}

bool StartFontCacheService() {
    ScopedServiceHandles handles;
    if (!OpenFontCacheService(SERVICE_START, handles)) return false;
    SERVICE_STATUS_PROCESS status{};
    DWORD bytesNeeded = 0;
    if (!QueryServiceStatusEx(handles.service, SC_STATUS_PROCESS_INFO,
            reinterpret_cast<LPBYTE>(&status), sizeof(status), &bytesNeeded)) {
        std::cerr << "    Warning: QueryServiceStatusEx failed" << SysUtils::GetLastErrorMessage() << "\n";
        return false;
    }
    if (status.dwCurrentState == SERVICE_RUNNING) return true;
    if (!StartServiceW(handles.service, 0, nullptr)) {
        DWORD error = GetLastError();
        if (error != ERROR_SERVICE_ALREADY_RUNNING) {
            std::cerr << "    Warning: Failed to start FontCache service" << SysUtils::GetLastErrorMessage() << "\n";
            return false;
        }
    }
    return WaitForServiceState(handles.service, SERVICE_RUNNING, FONT_CACHE_TIMEOUT_MS);
}

bool DeleteSystemCacheFiles() {
    bool success = true;
    if (DeleteFileA(SYSTEM_CACHE_FILE) == 0) {
        DWORD error = GetLastError();
        if (error != ERROR_FILE_NOT_FOUND) {
            std::cerr << "    Warning: Failed to delete " << SYSTEM_CACHE_FILE << SysUtils::GetLastErrorMessage() << "\n";
            success = false;
        }
    }
    std::error_code ec;
    fs::remove_all(fs::path(SERVICE_CACHE_DIR), ec);
    if (ec) {
        std::cerr << "    Warning: Failed to clean cache directory: " << SERVICE_CACHE_DIR << " (" << ec.message() << ")\n";
        success = false;
    }
    return success;
}

bool DeleteDirectoryTree(const fs::path& path, const char* description) {
    std::error_code ec;
    if (!fs::exists(path, ec)) {
        if (ec) {
            std::cerr << "    Warning: Unable to access " << description << ": " << path.string() << " (" << ec.message() << ")\n";
            return false;
        }
        return true;
    }

    fs::remove_all(path, ec);
    if (ec) {
        std::cerr << "    Warning: Failed to delete " << description << ": " << path.string() << " (" << ec.message() << ")\n";
        return false;
    }
    return true;
}

bool ClearAdobeCachesIn(const fs::path& root) {
    if (root.empty()) return true;
    std::error_code ec;
    if (!fs::exists(root, ec)) {
        if (ec) {
            std::cerr << "    Warning: Unable to access Adobe cache directory: " << root.string() << " (" << ec.message() << ")\n";
            return false;
        }
        return true;
    }

    bool success = true;
    const auto options = fs::directory_options::skip_permission_denied;
    fs::recursive_directory_iterator it(root, options, ec);
    if (ec) {
        std::cerr << "    Warning: Failed to enumerate Adobe cache directory: " << root.string() << " (" << ec.message() << ")\n";
        return false;
    }

    while (it != fs::recursive_directory_iterator()) {
        const fs::path current = it->path();
        std::error_code statusEc;
        bool isFile = it->is_regular_file(statusEc);
        if (statusEc) {
            std::cerr << "    Warning: Failed to query entry in Adobe cache directory: " << current.string() << " (" << statusEc.message() << ")\n";
            success = false;
        } else if (isFile) {
            std::string filename = current.filename().string();
            if (filename.rfind(ADOBE_CACHE_PREFIX, 0) == 0 && current.extension() == ADOBE_CACHE_EXTENSION) {
                std::error_code removeEc;
                if (!fs::remove(current, removeEc)) {
                    if (removeEc) {
                        std::cerr << "    Warning: Failed to delete Adobe cache file: " << current.string() << " (" << removeEc.message() << ")\n";
                        success = false;
                    }
                }
            }
        }

        it.increment(ec);
        if (ec) {
            std::cerr << "    Warning: Failed to continue Adobe cache traversal: " << root.string() << " (" << ec.message() << ")\n";
            success = false;
            break;
        }
    }
    return success;
}
} // namespace

// Registry buffer size constant (Windows registry value limit)
constexpr size_t REGISTRY_BUFFER_SIZE = 512;

namespace SysUtils {
// System utilities for Windows API, registry, file operations, and error handling

std::string GetLastErrorMessage() {
    DWORD error = GetLastError();
    if (error == 0) return "";

    char* msgBuf = nullptr;
    DWORD result = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                    NULL, error, 0, reinterpret_cast<LPSTR>(&msgBuf), 0, NULL);

    std::string message;
    if (result != 0 && msgBuf) {
        message = msgBuf;
        LocalFree(msgBuf);
    } else {
        // FormatMessageA failed, return error code only
        return " (Error " + std::to_string(error) + ")";
    }

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

// Helper: Check for path traversal attempts (../ or ..\)
static bool HasPathTraversal(const std::string& const path) noexcept {
    return path.find("..\\") != std::string::npos || path.find("../") != std::string::npos;
}

// Helper: Validate absolute paths are within fonts directory
static bool IsAbsolutePathInFontsDir(const std::string& const pathStr) noexcept {
    if (pathStr.length() <= 1 || pathStr[1] != ':') return true;

    std::string fontsDir = GetFontsDirectory();
    if (fontsDir.empty()) return false;

    std::string lowerPath = pathStr;
    std::string lowerFonts = fontsDir;
    // Locale-independent ASCII lowercase conversion
    for (auto& c : lowerPath) {
        if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
    }
    for (auto& c : lowerFonts) {
        if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
    }

    return lowerPath.find(lowerFonts) == 0;
}

bool IsValidFontPath(const char* path) {
    if (!path || strlen(path) == 0) return false;
    std::string pathStr(path);
    if (HasPathTraversal(pathStr)) return false;
    return IsAbsolutePathInFontsDir(pathStr);
}

bool RegReadFontEntry(const char* valueName, std::string& fontFile, bool perUser) {
    // Validate value name length (Windows limit: 16,383 characters)
    if (!valueName || strlen(valueName) > 16383) {
        return false;
    }

    HKEY hKey;
    const char* regPath = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
    HKEY rootKey = perUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;

    if (RegOpenKeyExA(rootKey, regPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    char buffer[REGISTRY_BUFFER_SIZE];
    DWORD bufferSize = sizeof(buffer);
    DWORD type;

    LONG result = RegQueryValueExA(hKey, valueName, NULL, &type, reinterpret_cast<LPBYTE>(buffer), &bufferSize);
    RegCloseKey(hKey);

    if (result == ERROR_SUCCESS && type == REG_SZ) {
        // Ensure buffer is null-terminated to prevent overflow
        if (bufferSize > 0 && bufferSize < sizeof(buffer)) {
            buffer[bufferSize] = '\0';
        } else {
            buffer[sizeof(buffer) - 1] = '\0';
        }
        fontFile = buffer;
        return true;
    }
    return false;
}

bool RegWriteFontEntry(const char* valueName, const char* fontFile, bool perUser) {
    // Validate value name length (Windows limit: 16,383 characters)
    if (!valueName || strlen(valueName) > 16383) {
        return false;
    }

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

    size_t pathLen = strlen(fontFile);
    // Validate length doesn't overflow DWORD (extremely unlikely but defensive)
    if (pathLen >= MAXDWORD) {
        RegCloseKey(hKey);
        return false;
    }

    LONG result = RegSetValueExA(hKey, valueName, 0, REG_SZ,
        reinterpret_cast<const BYTE*>(fontFile), static_cast<DWORD>(pathLen + 1));

    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool RegDeleteFontEntry(const char* valueName, bool perUser) {
    // Validate value name length (Windows limit: 16,383 characters)
    if (!valueName || strlen(valueName) > 16383) {
        return false;
    }

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

bool RegEnumerateFonts(void (*callback)(const char* name, const char* file, bool perUser), bool perUser) {
    HKEY hKey;
    const char* regPath = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
    HKEY rootKey = perUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;

    if (RegOpenKeyExA(rootKey, regPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    char valueName[REGISTRY_BUFFER_SIZE];
    BYTE valueData[REGISTRY_BUFFER_SIZE];
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

        // Ensure null-termination for safety
        if (dataSize > 0 && dataSize < sizeof(valueData)) {
            valueData[dataSize] = '\0';
        } else {
            valueData[sizeof(valueData) - 1] = '\0';
        }

        callback(valueName, reinterpret_cast<const char*>(valueData), perUser);
    }

    RegCloseKey(hKey);
    return true;
}

void NotifyFontChange() {
    // SendMessage return value not checked: broadcast notification is best-effort
    // Applications refresh fonts asynchronously; failure doesn't affect font installation success
    SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);
}

bool ClearUserFontCaches() {
    bool success = true;

    std::string localAppData = GetEnvVariable("LOCALAPPDATA");
    if (!localAppData.empty()) {
        fs::path local(localAppData);
        std::cout << "  - Clearing Windows user font cache directories...\n";
        if (!DeleteDirectoryTree(local / "FontCache", "user font cache directory")) success = false;
        if (!DeleteDirectoryTree(local / "Microsoft" / "Windows" / "FontCache", "user Microsoft font cache directory")) success = false;
        std::cout << "  - Removing Adobe cache files (LocalAppData)...\n";
        if (!ClearAdobeCachesIn(local / "Adobe")) success = false;
    } else {
        std::cerr << "    Warning: LOCALAPPDATA environment variable not set; skipping user font cache directories.\n";
    }

    std::string roamingAppData = GetEnvVariable("APPDATA");
    if (!roamingAppData.empty()) {
        std::cout << "  - Removing Adobe cache files (AppData)...\n";
        if (!ClearAdobeCachesIn(fs::path(roamingAppData) / "Adobe")) success = false;
    } else {
        std::cerr << "    Warning: APPDATA environment variable not set; skipping roaming Adobe caches.\n";
    }

    return success;
}

bool ClearSystemFontCaches() {
    std::cout << "  - Stopping Windows Font Cache Service (FontCache)...\n";
    if (!StopFontCacheService()) {
        std::cerr << "    Error: Failed to stop font cache service.\n";
        return false;
    }
    std::cout << "  - Deleting cache files...\n";
    bool deleted = DeleteSystemCacheFiles();
    if (!deleted) std::cerr << "    Warning: Could not delete all cache files.\n";
    std::cout << "  - Starting Windows Font Cache Service (FontCache)...\n";
    if (!StartFontCacheService()) {
        std::cerr << "    Error: Failed to restart font cache service.\n";
        return false;
    }
    return deleted;
}

} // namespace SysUtils
