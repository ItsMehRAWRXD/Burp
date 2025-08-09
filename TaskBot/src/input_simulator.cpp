#include "../include/autopilot.h"
#include "../include/task_bot.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

namespace TaskBot {

InputSimulator::InputSimulator() {
    std::cout << "[InputSimulator] Initializing input simulator..." << std::endl;
    
    display_ = XOpenDisplay(nullptr);
    if (!display_) {
        std::cerr << "[InputSimulator] Failed to open X display!" << std::endl;
        throw std::runtime_error("Cannot open X display");
    }
    
    screen_ = DefaultScreen(display_);
    rootWindow_ = RootWindow(display_, screen_);
    
    // Check for XTest extension
    int event_base, error_base, major_version, minor_version;
    if (!XTestQueryExtension(display_, &event_base, &error_base, &major_version, &minor_version)) {
        std::cerr << "[InputSimulator] XTest extension not available!" << std::endl;
        XCloseDisplay(display_);
        throw std::runtime_error("XTest extension not available");
    }
    
    std::cout << "[InputSimulator] Initialized successfully with XTest " 
              << major_version << "." << minor_version << std::endl;
}

InputSimulator::~InputSimulator() {
    if (display_) {
        XCloseDisplay(display_);
        std::cout << "[InputSimulator] Closed X display" << std::endl;
    }
}

bool InputSimulator::moveMouse(int x, int y, int duration) {
    std::cout << "[InputSimulator] Moving mouse to (" << x << ", " << y << ")" << std::endl;
    
    if (duration > 0) {
        // Smooth movement
        auto currentPos = getMousePosition();
        int startX = currentPos.first;
        int startY = currentPos.second;
        
        int steps = duration / 10; // 10ms per step
        if (steps < 1) steps = 1;
        
        for (int i = 1; i <= steps; i++) {
            int currentX = startX + (x - startX) * i / steps;
            int currentY = startY + (y - startY) * i / steps;
            
            XTestFakeMotionEvent(display_, screen_, currentX, currentY, CurrentTime);
            XFlush(display_);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } else {
        // Instant movement
        XTestFakeMotionEvent(display_, screen_, x, y, CurrentTime);
        XFlush(display_);
    }
    
    return true;
}

bool InputSimulator::clickMouse(MouseButton button) {
    std::cout << "[InputSimulator] Clicking mouse button " << static_cast<int>(button) << std::endl;
    
    // Press and release
    XTestFakeButtonEvent(display_, static_cast<unsigned int>(button), True, CurrentTime);
    XFlush(display_);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    XTestFakeButtonEvent(display_, static_cast<unsigned int>(button), False, CurrentTime);
    XFlush(display_);
    
    return true;
}

bool InputSimulator::doubleClick(MouseButton button) {
    std::cout << "[InputSimulator] Double-clicking mouse button " << static_cast<int>(button) << std::endl;
    
    clickMouse(button);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    clickMouse(button);
    
    return true;
}

bool InputSimulator::dragMouse(int startX, int startY, int endX, int endY, int duration) {
    std::cout << "[InputSimulator] Dragging from (" << startX << ", " << startY 
              << ") to (" << endX << ", " << endY << ")" << std::endl;
    
    // Move to start position
    moveMouse(startX, startY);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Press mouse button
    XTestFakeButtonEvent(display_, static_cast<unsigned int>(MouseButton::LEFT), True, CurrentTime);
    XFlush(display_);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Move to end position
    moveMouse(endX, endY, duration);
    
    // Release mouse button
    XTestFakeButtonEvent(display_, static_cast<unsigned int>(MouseButton::LEFT), False, CurrentTime);
    XFlush(display_);
    
    return true;
}

bool InputSimulator::scrollMouse(int direction, int amount) {
    std::cout << "[InputSimulator] Scrolling " << (direction > 0 ? "up" : "down") 
              << " by " << amount << " units" << std::endl;
    
    MouseButton scrollButton = direction > 0 ? MouseButton::SCROLL_UP : MouseButton::SCROLL_DOWN;
    
    for (int i = 0; i < amount; i++) {
        XTestFakeButtonEvent(display_, static_cast<unsigned int>(scrollButton), True, CurrentTime);
        XTestFakeButtonEvent(display_, static_cast<unsigned int>(scrollButton), False, CurrentTime);
        XFlush(display_);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    return true;
}

bool InputSimulator::typeText(const std::string& text, int delayMs) {
    std::cout << "[InputSimulator] Typing text: \"" << text << "\"" << std::endl;
    
    for (char c : text) {
        KeySym keysym = XStringToKeysym(std::string(1, c).c_str());
        if (keysym == NoSymbol) {
            // Try to handle special characters
            keysym = c;
        }
        
        KeyCode keycode = XKeysymToKeycode(display_, keysym);
        if (keycode == 0) {
            std::cerr << "[InputSimulator] Cannot find keycode for character: " << c << std::endl;
            continue;
        }
        
        // Check if we need shift
        bool needShift = (c >= 'A' && c <= 'Z') || 
                        strchr("!@#$%^&*()_+{}|:\"<>?~", c) != nullptr;
        
        if (needShift) {
            XTestFakeKeyEvent(display_, XKeysymToKeycode(display_, XK_Shift_L), True, CurrentTime);
            XFlush(display_);
        }
        
        // Press and release key
        XTestFakeKeyEvent(display_, keycode, True, CurrentTime);
        XFlush(display_);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        XTestFakeKeyEvent(display_, keycode, False, CurrentTime);
        XFlush(display_);
        
        if (needShift) {
            XTestFakeKeyEvent(display_, XKeysymToKeycode(display_, XK_Shift_L), False, CurrentTime);
            XFlush(display_);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    }
    
    return true;
}

bool InputSimulator::pressKey(const std::string& key, KeyModifier modifiers) {
    std::cout << "[InputSimulator] Pressing key: " << key 
              << " with modifiers: " << static_cast<int>(modifiers) << std::endl;
    
    // Handle modifiers
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::CTRL)) {
        XTestFakeKeyEvent(display_, XKeysymToKeycode(display_, XK_Control_L), True, CurrentTime);
    }
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::ALT)) {
        XTestFakeKeyEvent(display_, XKeysymToKeycode(display_, XK_Alt_L), True, CurrentTime);
    }
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::SHIFT)) {
        XTestFakeKeyEvent(display_, XKeysymToKeycode(display_, XK_Shift_L), True, CurrentTime);
    }
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::SUPER)) {
        XTestFakeKeyEvent(display_, XKeysymToKeycode(display_, XK_Super_L), True, CurrentTime);
    }
    XFlush(display_);
    
    // Press the key
    KeyCode keycode = getKeyCode(key);
    if (keycode != 0) {
        XTestFakeKeyEvent(display_, keycode, True, CurrentTime);
        XFlush(display_);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        XTestFakeKeyEvent(display_, keycode, False, CurrentTime);
        XFlush(display_);
    }
    
    // Release modifiers
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::SUPER)) {
        XTestFakeKeyEvent(display_, XKeysymToKeycode(display_, XK_Super_L), False, CurrentTime);
    }
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::SHIFT)) {
        XTestFakeKeyEvent(display_, XKeysymToKeycode(display_, XK_Shift_L), False, CurrentTime);
    }
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::ALT)) {
        XTestFakeKeyEvent(display_, XKeysymToKeycode(display_, XK_Alt_L), False, CurrentTime);
    }
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::CTRL)) {
        XTestFakeKeyEvent(display_, XKeysymToKeycode(display_, XK_Control_L), False, CurrentTime);
    }
    XFlush(display_);
    
    return true;
}

bool InputSimulator::executeShortcut(const std::string& shortcut) {
    std::cout << "[InputSimulator] Executing shortcut: " << shortcut << std::endl;
    
    // Parse shortcut string (e.g., "Ctrl+C", "Alt+Tab", "Ctrl+Shift+T")
    std::vector<std::string> keys;
    KeyModifier modifiers = KeyModifier::NONE;
    
    size_t pos = 0;
    std::string temp = shortcut;
    while ((pos = temp.find('+')) != std::string::npos) {
        std::string key = temp.substr(0, pos);
        
        if (key == "Ctrl" || key == "Control") {
            modifiers = static_cast<KeyModifier>(static_cast<int>(modifiers) | static_cast<int>(KeyModifier::CTRL));
        } else if (key == "Alt") {
            modifiers = static_cast<KeyModifier>(static_cast<int>(modifiers) | static_cast<int>(KeyModifier::ALT));
        } else if (key == "Shift") {
            modifiers = static_cast<KeyModifier>(static_cast<int>(modifiers) | static_cast<int>(KeyModifier::SHIFT));
        } else if (key == "Super" || key == "Win" || key == "Cmd") {
            modifiers = static_cast<KeyModifier>(static_cast<int>(modifiers) | static_cast<int>(KeyModifier::SUPER));
        } else {
            keys.push_back(key);
        }
        
        temp.erase(0, pos + 1);
    }
    
    // Last part is the actual key
    if (!temp.empty()) {
        keys.push_back(temp);
    }
    
    // Execute the shortcut
    for (const auto& key : keys) {
        pressKey(key, modifiers);
    }
    
    return true;
}

bool InputSimulator::selectAll() {
    std::cout << "[InputSimulator] Selecting all (Ctrl+A)" << std::endl;
    return executeShortcut("Ctrl+A");
}

bool InputSimulator::copy() {
    std::cout << "[InputSimulator] Copying (Ctrl+C)" << std::endl;
    return executeShortcut("Ctrl+C");
}

bool InputSimulator::paste() {
    std::cout << "[InputSimulator] Pasting (Ctrl+V)" << std::endl;
    return executeShortcut("Ctrl+V");
}

std::pair<int, int> InputSimulator::getMousePosition() {
    Window root_return, child_return;
    int root_x, root_y, win_x, win_y;
    unsigned int mask_return;
    
    XQueryPointer(display_, rootWindow_, &root_return, &child_return,
                  &root_x, &root_y, &win_x, &win_y, &mask_return);
    
    std::cout << "[InputSimulator] Current mouse position: (" << root_x << ", " << root_y << ")" << std::endl;
    return {root_x, root_y};
}

KeyCode InputSimulator::getKeyCode(const std::string& key) {
    KeySym keysym = NoSymbol;
    
    // Map common key names to X11 keysyms
    if (key == "Return" || key == "Enter") keysym = XK_Return;
    else if (key == "Tab") keysym = XK_Tab;
    else if (key == "Escape" || key == "Esc") keysym = XK_Escape;
    else if (key == "Space") keysym = XK_space;
    else if (key == "BackSpace" || key == "Backspace") keysym = XK_BackSpace;
    else if (key == "Delete" || key == "Del") keysym = XK_Delete;
    else if (key == "Home") keysym = XK_Home;
    else if (key == "End") keysym = XK_End;
    else if (key == "PageUp" || key == "Page_Up") keysym = XK_Page_Up;
    else if (key == "PageDown" || key == "Page_Down") keysym = XK_Page_Down;
    else if (key == "Left") keysym = XK_Left;
    else if (key == "Right") keysym = XK_Right;
    else if (key == "Up") keysym = XK_Up;
    else if (key == "Down") keysym = XK_Down;
    else if (key.length() == 1) {
        keysym = XStringToKeysym(key.c_str());
    }
    
    if (keysym == NoSymbol) {
        std::cerr << "[InputSimulator] Unknown key: " << key << std::endl;
        return 0;
    }
    
    return XKeysymToKeycode(display_, keysym);
}

} // namespace TaskBot