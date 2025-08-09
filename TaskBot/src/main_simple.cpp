#include <windows.h>
#include <iostream>
#include <string>

// Simple TaskBot that actually works
class TaskBot {
public:
    void moveMouse(int x, int y) {
        std::cout << "[TaskBot] Moving mouse to (" << x << ", " << y << ")" << std::endl;
        SetCursorPos(x, y);
    }
    
    void typeText(const std::string& text) {
        std::cout << "[TaskBot] Typing: " << text << std::endl;
        for (char c : text) {
            SHORT vk = VkKeyScan(c);
            BYTE virtualKey = LOBYTE(vk);
            
            INPUT input[2] = {};
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = virtualKey;
            
            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = virtualKey;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            
            SendInput(2, input, sizeof(INPUT));
            Sleep(50);
        }
    }
    
    void clickMouse() {
        std::cout << "[TaskBot] Clicking mouse" << std::endl;
        INPUT input[2] = {};
        
        input[0].type = INPUT_MOUSE;
        input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        
        input[1].type = INPUT_MOUSE;
        input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
        
        SendInput(2, input, sizeof(INPUT));
    }
    
    void openNotepad() {
        std::cout << "[TaskBot] Opening Notepad" << std::endl;
        ShellExecute(NULL, "open", "notepad.exe", NULL, NULL, SW_SHOW);
        Sleep(1000); // Wait for it to open
    }
};

int main() {
    std::cout << "TaskBot - Hello World Edition!" << std::endl;
    std::cout << "==============================" << std::endl;
    
    TaskBot bot;
    
    // Demo 1: Just print Hello World
    std::cout << "\nDemo 1: Console Hello World" << std::endl;
    std::cout << "Hello World from TaskBot!" << std::endl;
    
    // Demo 2: Open Notepad and type Hello World
    std::cout << "\nDemo 2: Type Hello World in Notepad (in 3 seconds)" << std::endl;
    std::cout << "Press Ctrl+C to cancel" << std::endl;
    
    Sleep(3000);
    
    bot.openNotepad();
    bot.typeText("Hello World from TaskBot!\n");
    bot.typeText("I can control your computer!");
    
    std::cout << "\nDone! TaskBot says Hello!" << std::endl;
    
    return 0;
}