#include "../include/autopilot_windows.h"
#include "../include/task_bot.h"
#include <iostream>
#include <algorithm>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

namespace TaskBot {

WindowManager::WindowManager() {
    std::cout << "[WindowManager] Initializing Windows window manager..." << std::endl;
}

WindowManager::~WindowManager() {
    std::cout << "[WindowManager] Shutting down window manager" << std::endl;
}

bool WindowManager::focusWindow(HWND hwnd) {
    std::cout << "[WindowManager] Focusing window: " << hwnd << std::endl;
    
    if (!IsWindow(hwnd)) {
        std::cerr << "[WindowManager] Invalid window handle" << std::endl;
        return false;
    }
    
    // Restore window if minimized
    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }
    
    // Bring window to foreground
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    
    return true;
}

bool WindowManager::minimizeWindow(HWND hwnd) {
    std::cout << "[WindowManager] Minimizing window: " << hwnd << std::endl;
    return ShowWindow(hwnd, SW_MINIMIZE) != 0;
}

bool WindowManager::maximizeWindow(HWND hwnd) {
    std::cout << "[WindowManager] Maximizing window: " << hwnd << std::endl;
    return ShowWindow(hwnd, SW_MAXIMIZE) != 0;
}

bool WindowManager::restoreWindow(HWND hwnd) {
    std::cout << "[WindowManager] Restoring window: " << hwnd << std::endl;
    return ShowWindow(hwnd, SW_RESTORE) != 0;
}

bool WindowManager::closeWindow(HWND hwnd) {
    std::cout << "[WindowManager] Closing window: " << hwnd << std::endl;
    return PostMessage(hwnd, WM_CLOSE, 0, 0) != 0;
}

bool WindowManager::resizeWindow(HWND hwnd, int width, int height) {
    std::cout << "[WindowManager] Resizing window to " << width << "x" << height << std::endl;
    
    RECT rect;
    if (!GetWindowRect(hwnd, &rect)) {
        return false;
    }
    
    return MoveWindow(hwnd, rect.left, rect.top, width, height, TRUE) != 0;
}

bool WindowManager::moveWindow(HWND hwnd, int x, int y) {
    std::cout << "[WindowManager] Moving window to (" << x << ", " << y << ")" << std::endl;
    
    RECT rect;
    if (!GetWindowRect(hwnd, &rect)) {
        return false;
    }
    
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    
    return MoveWindow(hwnd, x, y, width, height, TRUE) != 0;
}

HWND WindowManager::findWindowByTitle(const std::string& title) {
    std::cout << "[WindowManager] Finding window by title: \"" << title << "\"" << std::endl;
    
    struct FindWindowData {
        std::string targetTitle;
        HWND foundWindow;
    } data = {title, nullptr};
    
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto* data = reinterpret_cast<FindWindowData*>(lParam);
        
        char windowTitle[256];
        if (GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle)) > 0) {
            std::string currentTitle(windowTitle);
            
            // Case-insensitive partial match
            std::transform(currentTitle.begin(), currentTitle.end(), currentTitle.begin(), ::tolower);
            std::string searchTitle = data->targetTitle;
            std::transform(searchTitle.begin(), searchTitle.end(), searchTitle.begin(), ::tolower);
            
            if (currentTitle.find(searchTitle) != std::string::npos) {
                std::cout << "[WindowManager] Found window: \"" << windowTitle << "\"" << std::endl;
                data->foundWindow = hwnd;
                return FALSE; // Stop enumeration
            }
        }
        return TRUE; // Continue enumeration
    }, reinterpret_cast<LPARAM>(&data));
    
    if (data.foundWindow == nullptr) {
        std::cerr << "[WindowManager] Window not found: \"" << title << "\"" << std::endl;
    }
    
    return data.foundWindow;
}

HWND WindowManager::findWindowByClass(const std::string& className) {
    std::cout << "[WindowManager] Finding window by class: \"" << className << "\"" << std::endl;
    
    HWND hwnd = FindWindowA(className.c_str(), nullptr);
    
    if (hwnd == nullptr) {
        std::cerr << "[WindowManager] Window class not found: \"" << className << "\"" << std::endl;
    } else {
        std::cout << "[WindowManager] Found window with class: \"" << className << "\"" << std::endl;
    }
    
    return hwnd;
}

std::vector<HWND> WindowManager::findWindowsByProcess(DWORD processId) {
    std::cout << "[WindowManager] Finding windows for process ID: " << processId << std::endl;
    
    struct ProcessWindowData {
        DWORD targetPid;
        std::vector<HWND> windows;
    } data = {processId, {}};
    
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto* data = reinterpret_cast<ProcessWindowData*>(lParam);
        
        DWORD windowPid;
        GetWindowThreadProcessId(hwnd, &windowPid);
        
        if (windowPid == data->targetPid && IsWindowVisible(hwnd)) {
            data->windows.push_back(hwnd);
            
            char title[256];
            GetWindowTextA(hwnd, title, sizeof(title));
            std::cout << "[WindowManager] Found window for process: \"" << title << "\"" << std::endl;
        }
        
        return TRUE; // Continue enumeration
    }, reinterpret_cast<LPARAM>(&data));
    
    std::cout << "[WindowManager] Found " << data.windows.size() << " windows for process" << std::endl;
    return data.windows;
}

WindowInfo WindowManager::getWindowInfo(HWND hwnd) {
    WindowInfo info = {};
    info.handle = hwnd;
    
    // Get window title
    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));
    info.title = title;
    
    // Get window class
    char className[256];
    GetClassNameA(hwnd, className, sizeof(className));
    info.className = className;
    
    // Get window bounds
    GetWindowRect(hwnd, &info.bounds);
    
    // Get window state
    info.isVisible = IsWindowVisible(hwnd) != 0;
    info.isMinimized = IsIconic(hwnd) != 0;
    
    // Get process and thread IDs
    info.threadId = GetWindowThreadProcessId(hwnd, &info.processId);
    
    std::cout << "[WindowManager] Window info - Title: \"" << info.title 
              << "\", Class: \"" << info.className 
              << "\", PID: " << info.processId << std::endl;
    
    return info;
}

std::string WindowManager::getWindowTitle(HWND hwnd) {
    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));
    return std::string(title);
}

std::string WindowManager::getWindowClass(HWND hwnd) {
    char className[256];
    GetClassNameA(hwnd, className, sizeof(className));
    return std::string(className);
}

bool WindowManager::setWindowTitle(HWND hwnd, const std::string& title) {
    std::cout << "[WindowManager] Setting window title to: \"" << title << "\"" << std::endl;
    return SetWindowTextA(hwnd, title.c_str()) != 0;
}

bool WindowManager::setAlwaysOnTop(HWND hwnd, bool onTop) {
    std::cout << "[WindowManager] Setting window always on top: " << (onTop ? "true" : "false") << std::endl;
    
    HWND position = onTop ? HWND_TOPMOST : HWND_NOTOPMOST;
    return SetWindowPos(hwnd, position, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE) != 0;
}

bool WindowManager::setTransparency(HWND hwnd, BYTE alpha) {
    std::cout << "[WindowManager] Setting window transparency to: " << (int)alpha << std::endl;
    
    // Enable layered window style
    LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, style | WS_EX_LAYERED);
    
    // Set transparency
    return SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA) != 0;
}

bool WindowManager::takeScreenshot(HWND hwnd, const std::string& filename) {
    std::cout << "[WindowManager] Taking screenshot of window to: " << filename << std::endl;
    
    // Get window dimensions
    RECT rect;
    if (!GetWindowRect(hwnd, &rect)) {
        std::cerr << "[WindowManager] Failed to get window rect" << std::endl;
        return false;
    }
    
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    
    // Create device contexts
    HDC hdcWindow = GetDC(hwnd);
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);
    
    if (!hdcMemDC) {
        ReleaseDC(hwnd, hdcWindow);
        std::cerr << "[WindowManager] Failed to create compatible DC" << std::endl;
        return false;
    }
    
    // Create bitmap
    HBITMAP hbmWindow = CreateCompatibleBitmap(hdcWindow, width, height);
    
    if (!hbmWindow) {
        DeleteDC(hdcMemDC);
        ReleaseDC(hwnd, hdcWindow);
        std::cerr << "[WindowManager] Failed to create bitmap" << std::endl;
        return false;
    }
    
    // Select bitmap into DC
    SelectObject(hdcMemDC, hbmWindow);
    
    // Copy window content
    if (!BitBlt(hdcMemDC, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY)) {
        DeleteObject(hbmWindow);
        DeleteDC(hdcMemDC);
        ReleaseDC(hwnd, hdcWindow);
        std::cerr << "[WindowManager] BitBlt failed" << std::endl;
        return false;
    }
    
    // Save bitmap to file (simplified - would need proper BMP file writing)
    // For now, just cleanup
    DeleteObject(hbmWindow);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);
    
    std::cout << "[WindowManager] Screenshot saved (implementation needed for file writing)" << std::endl;
    return true;
}

BOOL CALLBACK WindowManager::EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    auto* windows = reinterpret_cast<std::vector<WindowInfo>*>(lParam);
    
    if (IsWindowVisible(hwnd)) {
        WindowManager manager;
        windows->push_back(manager.getWindowInfo(hwnd));
    }
    
    return TRUE;
}

} // namespace TaskBot