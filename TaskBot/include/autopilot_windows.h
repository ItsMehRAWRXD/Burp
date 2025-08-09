#ifndef AUTOPILOT_WINDOWS_H
#define AUTOPILOT_WINDOWS_H

#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <thread>
#include <chrono>

namespace TaskBot {

// Forward declarations
class SystemController;
class WindowManager;
class InputSimulator;
class ScreenCapture;
class ApplicationController;

// Mouse button definitions
enum class MouseButton {
    LEFT = 0,
    MIDDLE = 1,
    RIGHT = 2,
    XBUTTON1 = 3,
    XBUTTON2 = 4
};

// Key modifiers
enum class KeyModifier {
    NONE = 0,
    SHIFT = 1 << 0,
    CTRL = 1 << 1,
    ALT = 1 << 2,
    WIN = 1 << 3
};

// Window information
struct WindowInfo {
    HWND handle;
    std::string title;
    std::string className;
    RECT bounds;
    bool isVisible;
    bool isMinimized;
    DWORD processId;
    DWORD threadId;
};

// Process information
struct ProcessInfo {
    DWORD pid;
    std::string name;
    std::string path;
    std::vector<HWND> windows;
    double cpuUsage;
    size_t memoryUsage;
};

// Screen region
struct ScreenRegion {
    int x, y, width, height;
};

// System Controller - Core of the AutoPilot
class SystemController {
public:
    SystemController();
    ~SystemController();
    
    // System control
    bool initialize();
    void shutdown();
    bool isRunning() const { return running_; }
    
    // High-level automation
    bool executeAutomationScript(const std::string& script);
    bool recordActions(const std::string& outputFile);
    bool playbackActions(const std::string& inputFile);
    
    // Application control
    bool launchApplication(const std::string& path, const std::string& args = "");
    bool terminateApplication(const std::string& processName);
    bool switchToApplication(const std::string& windowTitle);
    
    // System monitoring
    std::vector<ProcessInfo> getRunningProcesses();
    std::vector<WindowInfo> getOpenWindows();
    WindowInfo getActiveWindow();
    
    // Global hotkeys
    bool registerHotkey(int id, UINT modifiers, UINT vk, std::function<void()> callback);
    bool unregisterHotkey(int id);
    
private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void messageLoop();
    
    bool running_;
    HWND messageWindow_;
    std::thread messageThread_;
    std::map<int, std::function<void()>> hotkeys_;
    static SystemController* instance_;
};

// Input Simulator for keyboard and mouse automation
class InputSimulator {
public:
    InputSimulator();
    ~InputSimulator();
    
    // Mouse operations
    bool moveMouse(int x, int y, int duration = 0);
    bool clickMouse(MouseButton button = MouseButton::LEFT);
    bool doubleClick(MouseButton button = MouseButton::LEFT);
    bool dragMouse(int startX, int startY, int endX, int endY, int duration = 500);
    bool scrollMouse(int direction, int amount = 3);
    
    // Keyboard operations
    bool typeText(const std::string& text, int delayMs = 50);
    bool pressKey(WORD virtualKey, KeyModifier modifiers = KeyModifier::NONE);
    bool pressKeyCombo(const std::vector<WORD>& keys);
    bool holdKey(WORD virtualKey, int durationMs);
    
    // Complex inputs
    bool executeShortcut(const std::string& shortcut); // e.g., "Ctrl+C"
    bool pasteText(const std::string& text);
    bool selectAll();
    bool copy();
    bool paste();
    bool undo();
    bool redo();
    
    // Get current state
    POINT getMousePosition();
    bool isKeyPressed(WORD virtualKey);
    
private:
    WORD getVirtualKey(const std::string& key);
    void sendKeyInput(WORD vk, bool keyDown);
    void sendMouseInput(DWORD flags, int x = 0, int y = 0, DWORD data = 0);
};

// Window Manager for application window control
class WindowManager {
public:
    WindowManager();
    ~WindowManager();
    
    // Window operations
    bool focusWindow(HWND hwnd);
    bool minimizeWindow(HWND hwnd);
    bool maximizeWindow(HWND hwnd);
    bool restoreWindow(HWND hwnd);
    bool closeWindow(HWND hwnd);
    bool resizeWindow(HWND hwnd, int width, int height);
    bool moveWindow(HWND hwnd, int x, int y);
    
    // Window search
    HWND findWindowByTitle(const std::string& title);
    HWND findWindowByClass(const std::string& className);
    std::vector<HWND> findWindowsByProcess(DWORD processId);
    
    // Window properties
    WindowInfo getWindowInfo(HWND hwnd);
    std::string getWindowTitle(HWND hwnd);
    std::string getWindowClass(HWND hwnd);
    bool setWindowTitle(HWND hwnd, const std::string& title);
    
    // Advanced operations
    bool setAlwaysOnTop(HWND hwnd, bool onTop);
    bool setTransparency(HWND hwnd, BYTE alpha);
    bool takeScreenshot(HWND hwnd, const std::string& filename);
    
    // Window enumeration
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
    
private:
    std::vector<WindowInfo> windows_;
};

// Screen Capture and OCR
class ScreenCapture {
public:
    ScreenCapture();
    ~ScreenCapture();
    
    // Screenshot operations
    bool captureScreen(const std::string& filename);
    bool captureRegion(const ScreenRegion& region, const std::string& filename);
    bool captureWindow(HWND hwnd, const std::string& filename);
    
    // Bitmap operations
    HBITMAP captureScreenToBitmap();
    HBITMAP captureRegionToBitmap(const ScreenRegion& region);
    HBITMAP captureWindowToBitmap(HWND hwnd);
    bool saveBitmap(HBITMAP hBitmap, const std::string& filename);
    
    // Pixel operations
    COLORREF getPixelColor(int x, int y);
    bool waitForPixelColor(int x, int y, COLORREF color, int timeoutMs);
    
    // Clipboard operations
    bool copyToClipboard(HBITMAP hBitmap);
    std::string getClipboardText();
    bool setClipboardText(const std::string& text);
    
private:
    HDC screenDC_;
    HDC memoryDC_;
};

// Application Controller for deep application integration
class ApplicationController {
public:
    ApplicationController();
    ~ApplicationController();
    
    // Process control
    bool attachToProcess(DWORD processId);
    bool detachFromProcess();
    bool injectDLL(const std::string& dllPath);
    
    // Memory operations
    bool readProcessMemory(LPVOID address, LPVOID buffer, SIZE_T size);
    bool writeProcessMemory(LPVOID address, LPCVOID buffer, SIZE_T size);
    LPVOID allocateMemory(SIZE_T size);
    bool freeMemory(LPVOID address);
    
    // Window message automation
    bool sendMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    bool postMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    // Control automation
    bool clickButton(HWND parent, const std::string& buttonText);
    bool setEditText(HWND parent, const std::string& text);
    bool selectComboBoxItem(HWND parent, int index);
    bool checkCheckBox(HWND parent, bool check);
    
    // Menu automation
    bool clickMenuItem(HWND hwnd, const std::string& menuPath);
    HMENU findMenu(HWND hwnd, const std::string& menuName);
    
private:
    HANDLE processHandle_;
    DWORD attachedProcessId_;
    bool isAttached_;
};

// AutoPilot Manager - High-level automation orchestrator
class AutoPilotManager {
public:
    AutoPilotManager();
    ~AutoPilotManager();
    
    // Component access
    SystemController& getSystemController() { return *systemController_; }
    InputSimulator& getInputSimulator() { return *inputSimulator_; }
    WindowManager& getWindowManager() { return *windowManager_; }
    ScreenCapture& getScreenCapture() { return *screenCapture_; }
    ApplicationController& getAppController() { return *appController_; }
    
    // Automation workflows
    bool executeWorkflow(const std::string& workflowName);
    bool registerWorkflow(const std::string& name, std::function<bool()> workflow);
    
    // Predefined automations
    bool autoCompleteCode(const std::string& partialCode);
    bool extractDataFromApplication(const std::string& appName, const std::string& dataPattern);
    bool synchronizeApplications(const std::vector<std::string>& apps);
    bool automateDataEntry(const std::map<std::string, std::string>& data);
    
    // Macro recording and playback
    bool startRecording(const std::string& macroName);
    bool stopRecording();
    bool playMacro(const std::string& macroName, int speed = 1);
    bool saveMacro(const std::string& macroName, const std::string& filename);
    bool loadMacro(const std::string& filename);
    
    // Windows-specific features
    bool automateWindowsApp(const std::string& appName, const std::string& actions);
    bool extractFromClipboard();
    bool automateContextMenu(const std::string& option);
    bool automateWindowsDialog(const std::string& title, const std::map<std::string, std::string>& inputs);
    
private:
    void registerDefaultWorkflows();
    void initializeWindowsHooks();
    
    std::unique_ptr<SystemController> systemController_;
    std::unique_ptr<InputSimulator> inputSimulator_;
    std::unique_ptr<WindowManager> windowManager_;
    std::unique_ptr<ScreenCapture> screenCapture_;
    std::unique_ptr<ApplicationController> appController_;
    
    std::map<std::string, std::function<bool()>> workflows_;
    bool isRecording_;
    std::vector<std::string> recordedActions_;
    
    // Windows hooks
    HHOOK keyboardHook_;
    HHOOK mouseHook_;
};

// Helper functions
namespace AutoPilotHelpers {
    std::string generateAutomationScript(const std::string& description);
    bool validateScript(const std::string& script);
    std::vector<std::string> getInstalledApplications();
    bool isApplicationResponding(const std::string& appName);
    std::string getWindowsVersion();
    bool isElevated();
    bool runAsAdmin(const std::string& command);
    
    // Windows-specific helpers
    std::string getErrorMessage(DWORD errorCode);
    std::vector<std::string> getStartMenuPrograms();
    bool createShortcut(const std::string& targetPath, const std::string& shortcutPath);
}

} // namespace TaskBot

#endif // AUTOPILOT_WINDOWS_H