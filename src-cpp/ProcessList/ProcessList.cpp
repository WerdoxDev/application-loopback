#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

#pragma comment(lib, "psapi.lib")

// Helper function to convert wide string to UTF-8
std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) return std::string();

    int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &result[0], size, nullptr, nullptr);
    return result;
}

struct AppInfo {
    DWORD processId;
    std::string processName;
    std::string windowTitle;
    HWND windowHandle;
    bool isVisible;
};

// Callback function for EnumWindows
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    std::vector<AppInfo>* apps = reinterpret_cast<std::vector<AppInfo>*>(lParam);

    // Get window title using Unicode version
    wchar_t windowTitle[256];
    GetWindowTextW(hwnd, windowTitle, sizeof(windowTitle) / sizeof(wchar_t));

    // Skip windows without titles (usually system windows)
    if (wcslen(windowTitle) == 0) {
        return TRUE;
    }

    // Convert to UTF-8 string
    std::string windowTitleUtf8 = WideToUtf8(windowTitle);

    // Get process ID
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);

    // Get process handle
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL) {
        return TRUE;
    }

    // Get process name using Unicode version
    wchar_t processName[MAX_PATH];
    DWORD size = sizeof(processName) / sizeof(wchar_t);
    if (!QueryFullProcessImageNameW(hProcess, 0, processName, &size)) {
        CloseHandle(hProcess);
        return TRUE;
    }

    // Extract just the filename from full path and convert to UTF-8
    std::wstring fullPath(processName);
    size_t lastSlash = fullPath.find_last_of(L"\\");
    std::wstring fileName = (lastSlash != std::wstring::npos) ?
        fullPath.substr(lastSlash + 1) : fullPath;
    std::string fileNameUtf8 = WideToUtf8(fileName);

    // Check if window is visible
    bool isVisible = IsWindowVisible(hwnd);

    // Add to our list
    AppInfo app;
    app.processId = processId;
    app.processName = fileNameUtf8;
    app.windowTitle = windowTitleUtf8;
    app.windowHandle = hwnd;
    app.isVisible = isVisible;

    apps->push_back(app);

    CloseHandle(hProcess);
    return TRUE;
}

// Alternative method using process enumeration - REMOVED for cleaner output

void PrintApplicationsWithWindows(const std::vector<AppInfo>& apps) {
    for (const auto& app : apps) {
        std::cout << app.processId<< ";" << app.windowTitle << "\n";
    }
}

int main() {
    // Set console to UTF-8 for proper character display
    SetConsoleOutputCP(CP_UTF8);

    // Enumerate windows to get all windows with titles
    std::vector<AppInfo> windowApps;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windowApps));

    // Filter only windows with titles (no duplicate removal)
    std::vector<AppInfo> appsWithTitles;
    for (const auto& app : windowApps) {
        if (!app.windowTitle.empty()) {
            appsWithTitles.push_back(app);
        }
    }

    PrintApplicationsWithWindows(appsWithTitles);

    return 0;
}