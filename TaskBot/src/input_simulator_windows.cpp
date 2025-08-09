#include "../include/autopilot_windows.h"
#include "../include/task_bot.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>

namespace TaskBot {

InputSimulator::InputSimulator() {
    std::cout << "[InputSimulator] Initializing Windows input simulator..." << std::endl;
    std::cout << "[InputSimulator] Ready to control mouse and keyboard!" << std::endl;
}

InputSimulator::~InputSimulator() {
    std::cout << "[InputSimulator] Shutting down input simulator" << std::endl;
}

bool InputSimulator::moveMouse(int x, int y, int duration) {
    std::cout << "[InputSimulator] Moving mouse to (" << x << ", " << y << ")" << std::endl;
    
    if (duration > 0) {
        // Smooth movement animation
        POINT currentPos;
        GetCursorPos(&currentPos);
        
        int steps = duration / 10; // 10ms per step
        if (steps < 1) steps = 1;
        
        for (int i = 1; i <= steps; i++) {
            int currentX = currentPos.x + (x - currentPos.x) * i / steps;
            int currentY = currentPos.y + (y - currentPos.y) * i / steps;
            
            SetCursorPos(currentX, currentY);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } else {
        // Instant movement
        SetCursorPos(x, y);
    }
    
    return true;
}

bool InputSimulator::clickMouse(MouseButton button) {
    std::cout << "[InputSimulator] Clicking mouse button " << static_cast<int>(button) << std::endl;
    
    DWORD downFlag, upFlag;
    
    switch (button) {
        case MouseButton::LEFT:
            downFlag = MOUSEEVENTF_LEFTDOWN;
            upFlag = MOUSEEVENTF_LEFTUP;
            break;
        case MouseButton::RIGHT:
            downFlag = MOUSEEVENTF_RIGHTDOWN;
            upFlag = MOUSEEVENTF_RIGHTUP;
            break;
        case MouseButton::MIDDLE:
            downFlag = MOUSEEVENTF_MIDDLEDOWN;
            upFlag = MOUSEEVENTF_MIDDLEUP;
            break;
        default:
            return false;
    }
    
    // Send mouse down
    sendMouseInput(downFlag);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Send mouse up
    sendMouseInput(upFlag);
    
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
    sendMouseInput(MOUSEEVENTF_LEFTDOWN);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Move to end position
    moveMouse(endX, endY, duration);
    
    // Release mouse button
    sendMouseInput(MOUSEEVENTF_LEFTUP);
    
    return true;
}

bool InputSimulator::scrollMouse(int direction, int amount) {
    std::cout << "[InputSimulator] Scrolling " << (direction > 0 ? "up" : "down") 
              << " by " << amount << " units" << std::endl;
    
    for (int i = 0; i < amount; i++) {
        sendMouseInput(MOUSEEVENTF_WHEEL, 0, 0, direction > 0 ? 120 : -120);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    return true;
}

bool InputSimulator::typeText(const std::string& text, int delayMs) {
    std::cout << "[InputSimulator] Typing text: \"" << text << "\"" << std::endl;
    
    for (char c : text) {
        // Handle special characters
        SHORT vk = VkKeyScan(c);
        BYTE virtualKey = LOBYTE(vk);
        BYTE shiftState = HIBYTE(vk);
        
        // Press shift if needed
        if (shiftState & 1) {
            sendKeyInput(VK_SHIFT, true);
        }
        
        // Press and release the key
        sendKeyInput(virtualKey, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        sendKeyInput(virtualKey, false);
        
        // Release shift if it was pressed
        if (shiftState & 1) {
            sendKeyInput(VK_SHIFT, false);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    }
    
    return true;
}

bool InputSimulator::pressKey(WORD virtualKey, KeyModifier modifiers) {
    std::cout << "[InputSimulator] Pressing key: " << virtualKey 
              << " with modifiers: " << static_cast<int>(modifiers) << std::endl;
    
    // Press modifiers
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::CTRL)) {
        sendKeyInput(VK_CONTROL, true);
    }
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::ALT)) {
        sendKeyInput(VK_MENU, true);
    }
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::SHIFT)) {
        sendKeyInput(VK_SHIFT, true);
    }
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::WIN)) {
        sendKeyInput(VK_LWIN, true);
    }
    
    // Press the main key
    sendKeyInput(virtualKey, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    sendKeyInput(virtualKey, false);
    
    // Release modifiers in reverse order
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::WIN)) {
        sendKeyInput(VK_LWIN, false);
    }
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::SHIFT)) {
        sendKeyInput(VK_SHIFT, false);
    }
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::ALT)) {
        sendKeyInput(VK_MENU, false);
    }
    if (static_cast<int>(modifiers) & static_cast<int>(KeyModifier::CTRL)) {
        sendKeyInput(VK_CONTROL, false);
    }
    
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
        } else if (key == "Win" || key == "Windows") {
            modifiers = static_cast<KeyModifier>(static_cast<int>(modifiers) | static_cast<int>(KeyModifier::WIN));
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
        WORD vk = getVirtualKey(key);
        if (vk != 0) {
            pressKey(vk, modifiers);
        }
    }
    
    return true;
}

bool InputSimulator::selectAll() {
    std::cout << "[InputSimulator] Selecting all (Ctrl+A)" << std::endl;
    return pressKey('A', KeyModifier::CTRL);
}

bool InputSimulator::copy() {
    std::cout << "[InputSimulator] Copying (Ctrl+C)" << std::endl;
    return pressKey('C', KeyModifier::CTRL);
}

bool InputSimulator::paste() {
    std::cout << "[InputSimulator] Pasting (Ctrl+V)" << std::endl;
    return pressKey('V', KeyModifier::CTRL);
}

bool InputSimulator::undo() {
    std::cout << "[InputSimulator] Undo (Ctrl+Z)" << std::endl;
    return pressKey('Z', KeyModifier::CTRL);
}

bool InputSimulator::redo() {
    std::cout << "[InputSimulator] Redo (Ctrl+Y)" << std::endl;
    return pressKey('Y', KeyModifier::CTRL);
}

POINT InputSimulator::getMousePosition() {
    POINT pos;
    GetCursorPos(&pos);
    std::cout << "[InputSimulator] Current mouse position: (" << pos.x << ", " << pos.y << ")" << std::endl;
    return pos;
}

bool InputSimulator::isKeyPressed(WORD virtualKey) {
    return (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
}

WORD InputSimulator::getVirtualKey(const std::string& key) {
    // Handle single characters
    if (key.length() == 1) {
        char c = key[0];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            return toupper(c);
        }
        if (c >= '0' && c <= '9') {
            return c;
        }
    }
    
    // Handle special keys
    std::map<std::string, WORD> keyMap = {
        {"Enter", VK_RETURN},
        {"Return", VK_RETURN},
        {"Tab", VK_TAB},
        {"Escape", VK_ESCAPE},
        {"Esc", VK_ESCAPE},
        {"Space", VK_SPACE},
        {"Backspace", VK_BACK},
        {"Delete", VK_DELETE},
        {"Del", VK_DELETE},
        {"Home", VK_HOME},
        {"End", VK_END},
        {"PageUp", VK_PRIOR},
        {"PageDown", VK_NEXT},
        {"Left", VK_LEFT},
        {"Right", VK_RIGHT},
        {"Up", VK_UP},
        {"Down", VK_DOWN},
        {"F1", VK_F1},
        {"F2", VK_F2},
        {"F3", VK_F3},
        {"F4", VK_F4},
        {"F5", VK_F5},
        {"F6", VK_F6},
        {"F7", VK_F7},
        {"F8", VK_F8},
        {"F9", VK_F9},
        {"F10", VK_F10},
        {"F11", VK_F11},
        {"F12", VK_F12}
    };
    
    auto it = keyMap.find(key);
    if (it != keyMap.end()) {
        return it->second;
    }
    
    std::cerr << "[InputSimulator] Unknown key: " << key << std::endl;
    return 0;
}

void InputSimulator::sendKeyInput(WORD vk, bool keyDown) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.dwFlags = keyDown ? 0 : KEYEVENTF_KEYUP;
    
    SendInput(1, &input, sizeof(INPUT));
}

void InputSimulator::sendMouseInput(DWORD flags, int x, int y, DWORD data) {
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = flags;
    input.mi.dx = x;
    input.mi.dy = y;
    input.mi.mouseData = data;
    
    SendInput(1, &input, sizeof(INPUT));
}

} // namespace TaskBot