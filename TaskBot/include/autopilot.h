#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <chrono>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

namespace TaskBot {

// Forward declarations
class SystemController;
class ProcessManager;
class WindowManager;
class InputSimulator;
class ScreenCapture;
class ApplicationController;

// Mouse button definitions
enum class MouseButton {
    LEFT = 1,
    MIDDLE = 2,
    RIGHT = 3,
    SCROLL_UP = 4,
    SCROLL_DOWN = 5
};

// Key modifiers
enum class KeyModifier {
    NONE = 0,
    SHIFT = 1 << 0,
    CTRL = 1 << 1,
    ALT = 1 << 2,
    SUPER = 1 << 3
};

// Window information
struct WindowInfo {
    unsigned long id;
    std::string title;
    std::string className;
    int x, y, width, height;
    bool isVisible;
    bool isMinimized;
    pid_t pid;
};

// Process information
struct ProcessInfo {
    pid_t pid;
    std::string name;
    std::string cmdline;
    std::vector<unsigned long> windows;
    double cpuUsage;
    size_t memoryUsage;
};

// Screen region
struct ScreenRegion {
    int x, y, width, height;
};

// OCR result
struct OCRResult {
    std::string text;
    ScreenRegion boundingBox;
    double confidence;
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
    bool launchApplication(const std::string& command, const std::vector<std::string>& args);
    bool terminateApplication(const std::string& processName);
    bool switchToApplication(const std::string& windowTitle);
    
    // System monitoring
    std::vector<ProcessInfo> getRunningProcesses();
    std::vector<WindowInfo> getOpenWindows();
    WindowInfo getActiveWindow();
    
    // Global hotkeys
    bool registerHotkey(const std::string& key, std::function<void()> callback);
    bool unregisterHotkey(const std::string& key);
    
private:
    bool running_;
    Display* display_;
    std::thread eventThread_;
    std::map<std::string, std::function<void()>> hotkeys_;
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
    bool pressKey(const std::string& key, KeyModifier modifiers = KeyModifier::NONE);
    bool pressKeyCombo(const std::vector<std::string>& keys);
    bool holdKey(const std::string& key, int durationMs);
    
    // Complex inputs
    bool executeShortcut(const std::string& shortcut); // e.g., "Ctrl+C"
    bool pasteText(const std::string& text);
    bool selectAll();
    bool copy();
    bool paste();
    
    // Get current state
    std::pair<int, int> getMousePosition();
    bool isKeyPressed(const std::string& key);
    
private:
    Display* display_;
    int screen_;
    Window rootWindow_;
    
    KeyCode getKeyCode(const std::string& key);
    unsigned int getModifierMask(KeyModifier modifiers);
};

// Window Manager for application window control
class WindowManager {
public:
    WindowManager();
    ~WindowManager();
    
    // Window operations
    bool focusWindow(unsigned long windowId);
    bool minimizeWindow(unsigned long windowId);
    bool maximizeWindow(unsigned long windowId);
    bool closeWindow(unsigned long windowId);
    bool resizeWindow(unsigned long windowId, int width, int height);
    bool moveWindow(unsigned long windowId, int x, int y);
    
    // Window search
    unsigned long findWindowByTitle(const std::string& title);
    unsigned long findWindowByClass(const std::string& className);
    std::vector<unsigned long> findWindowsByPID(pid_t pid);
    
    // Window properties
    WindowInfo getWindowInfo(unsigned long windowId);
    std::string getWindowTitle(unsigned long windowId);
    bool setWindowTitle(unsigned long windowId, const std::string& title);
    
    // Advanced operations
    bool setAlwaysOnTop(unsigned long windowId, bool onTop);
    bool setTransparency(unsigned long windowId, double opacity);
    bool takeScreenshot(unsigned long windowId, const std::string& filename);
    
private:
    Display* display_;
    Atom getAtom(const char* atomName);
};

// Screen Capture and OCR
class ScreenCapture {
public:
    ScreenCapture();
    ~ScreenCapture();
    
    // Screenshot operations
    bool captureScreen(const std::string& filename);
    bool captureRegion(const ScreenRegion& region, const std::string& filename);
    bool captureWindow(unsigned long windowId, const std::string& filename);
    
    // Image search
    bool findImageOnScreen(const std::string& templateImage, ScreenRegion& result);
    std::vector<ScreenRegion> findAllImages(const std::string& templateImage);
    bool waitForImage(const std::string& templateImage, int timeoutMs, ScreenRegion& result);
    
    // OCR operations
    std::string readTextFromScreen(const ScreenRegion& region);
    std::vector<OCRResult> findTextOnScreen(const std::string& searchText);
    bool clickOnText(const std::string& text);
    
    // Pixel operations
    std::string getPixelColor(int x, int y);
    bool waitForPixelColor(int x, int y, const std::string& color, int timeoutMs);
    
private:
    Display* display_;
    bool initializeOCR();
    void* ocrEngine_; // Tesseract handle
};

// Application Controller for deep application integration
class ApplicationController {
public:
    ApplicationController();
    ~ApplicationController();
    
    // Process control
    bool attachToProcess(pid_t pid);
    bool detachFromProcess();
    bool injectLibrary(const std::string& libraryPath);
    
    // Memory operations
    bool readProcessMemory(void* address, void* buffer, size_t size);
    bool writeProcessMemory(void* address, const void* buffer, size_t size);
    std::vector<void*> searchMemory(const void* pattern, size_t patternSize);
    
    // Function hooking
    bool hookFunction(void* targetFunction, void* hookFunction);
    bool unhookFunction(void* targetFunction);
    
    // GUI automation helpers
    bool clickButton(const std::string& buttonText);
    bool fillTextField(const std::string& fieldName, const std::string& text);
    bool selectMenuItem(const std::string& menuPath);
    bool selectComboBoxItem(const std::string& comboBoxName, const std::string& item);
    
    // Application-specific automation
    bool automateWebBrowser(const std::string& url, const std::string& actions);
    bool automateTextEditor(const std::string& filename, const std::string& content);
    bool automateTerminal(const std::vector<std::string>& commands);
    
private:
    pid_t attachedPid_;
    std::map<void*, void*> hooks_;
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
    
    // AI-assisted automation (using web search)
    bool automateFromDescription(const std::string& taskDescription);
    bool learnFromDemonstration(const std::string& taskName);
    
private:
    std::unique_ptr<SystemController> systemController_;
    std::unique_ptr<InputSimulator> inputSimulator_;
    std::unique_ptr<WindowManager> windowManager_;
    std::unique_ptr<ScreenCapture> screenCapture_;
    std::unique_ptr<ApplicationController> appController_;
    
    std::map<std::string, std::function<bool()>> workflows_;
    bool isRecording_;
    std::vector<std::string> recordedActions_;
};

// AutoPilot Task for TaskBot integration
class AutoPilotTask : public Task {
public:
    enum class Operation {
        EXECUTE_WORKFLOW,
        RECORD_MACRO,
        PLAY_MACRO,
        AUTOMATE_APP,
        EXTRACT_DATA,
        SYNC_APPS,
        CUSTOM_AUTOMATION
    };
    
    AutoPilotTask(const std::string& name, Operation op,
                  std::shared_ptr<AutoPilotManager> manager,
                  const std::map<std::string, std::string>& params);
    
    bool execute() override;
    std::string getDescription() const override;
    
private:
    Operation operation_;
    std::shared_ptr<AutoPilotManager> manager_;
    std::map<std::string, std::string> params_;
};

// Helper functions
namespace AutoPilotHelpers {
    std::string generateAutomationScript(const std::string& description);
    bool validateScript(const std::string& script);
    std::vector<std::string> getAvailableApplications();
    bool isApplicationResponding(const std::string& appName);
    std::string getClipboardContent();
    bool setClipboardContent(const std::string& content);
}

} // namespace TaskBot

#endif // AUTOPILOT_H