// TaskBot - Zero Dependencies, Pure Windows API
// Compiles with: g++ taskbot_standalone.cpp -o taskbot.exe -luser32 -lshell32
// Or Visual Studio: cl taskbot_standalone.cpp user32.lib shell32.lib

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

class TaskBot {
public:
    // Mouse control
    void moveMouse(int x, int y) {
        std::cout << "[TaskBot] Moving mouse to (" << x << ", " << y << ")" << std::endl;
        SetCursorPos(x, y);
    }
    
    void clickMouse() {
        std::cout << "[TaskBot] Click!" << std::endl;
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        Sleep(50);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    }
    
    void rightClick() {
        std::cout << "[TaskBot] Right click!" << std::endl;
        mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
        Sleep(50);
        mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
    }
    
    void doubleClick() {
        std::cout << "[TaskBot] Double click!" << std::endl;
        clickMouse();
        Sleep(100);
        clickMouse();
    }
    
    // Keyboard control
    void pressKey(BYTE vk) {
        keybd_event(vk, 0, 0, 0);
        Sleep(50);
        keybd_event(vk, 0, KEYEVENTF_KEYUP, 0);
    }
    
    void typeText(const std::string& text) {
        std::cout << "[TaskBot] Typing: " << text << std::endl;
        for (char c : text) {
            if (c == '\n') {
                pressKey(VK_RETURN);
            } else {
                SHORT vk = VkKeyScan(c);
                BYTE virtualKey = LOBYTE(vk);
                BYTE shiftState = HIBYTE(vk);
                
                if (shiftState & 1) {
                    keybd_event(VK_SHIFT, 0, 0, 0);
                }
                
                keybd_event(virtualKey, 0, 0, 0);
                keybd_event(virtualKey, 0, KEYEVENTF_KEYUP, 0);
                
                if (shiftState & 1) {
                    keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                }
                
                Sleep(30);
            }
        }
    }
    
    void shortcut(BYTE modifier, BYTE key) {
        std::cout << "[TaskBot] Shortcut: " << (char)key << std::endl;
        keybd_event(modifier, 0, 0, 0);
        keybd_event(key, 0, 0, 0);
        Sleep(50);
        keybd_event(key, 0, KEYEVENTF_KEYUP, 0);
        keybd_event(modifier, 0, KEYEVENTF_KEYUP, 0);
    }
    
    // Window control
    HWND findWindow(const std::string& title) {
        std::cout << "[TaskBot] Finding window: " << title << std::endl;
        return FindWindowA(NULL, title.c_str());
    }
    
    void focusWindow(HWND hwnd) {
        if (hwnd) {
            std::cout << "[TaskBot] Focusing window" << std::endl;
            SetForegroundWindow(hwnd);
            SetFocus(hwnd);
        }
    }
    
    // Application control
    void run(const std::string& program) {
        std::cout << "[TaskBot] Running: " << program << std::endl;
        ShellExecuteA(NULL, "open", program.c_str(), NULL, NULL, SW_SHOW);
        Sleep(1000);
    }
    
    // System control
    void wait(int ms) {
        std::cout << "[TaskBot] Waiting " << ms << "ms" << std::endl;
        Sleep(ms);
    }
    
    POINT getMousePos() {
        POINT p;
        GetCursorPos(&p);
        return p;
    }
    
    // Demo workflows
    void helloWorldDemo() {
        std::cout << "\n=== Hello World Demo ===" << std::endl;
        run("notepad.exe");
        wait(1000);
        typeText("Hello World from TaskBot!\n\n");
        typeText("I can control:\n");
        typeText("- Your mouse\n");
        typeText("- Your keyboard\n");
        typeText("- Your applications\n");
        typeText("- Your entire system!\n\n");
        typeText("No dependencies required!");
    }
    
    void drawWithMouse() {
        std::cout << "\n=== Mouse Drawing Demo ===" << std::endl;
        run("mspaint.exe");
        wait(2000);
        
        // Draw a square
        moveMouse(200, 200);
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        
        moveMouse(400, 200);
        wait(100);
        moveMouse(400, 400);
        wait(100);
        moveMouse(200, 400);
        wait(100);
        moveMouse(200, 200);
        
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    }
};

// Simple menu system
void showMenu() {
    std::cout << "\nTaskBot Menu:" << std::endl;
    std::cout << "1. Hello World (Notepad)" << std::endl;
    std::cout << "2. Mouse Drawing (Paint)" << std::endl;
    std::cout << "3. Custom Command" << std::endl;
    std::cout << "4. Exit" << std::endl;
    std::cout << "Choice: ";
}

int main() {
    std::cout << "TaskBot - Zero Dependencies Edition" << std::endl;
    std::cout << "===================================" << std::endl;
    std::cout << "Pure Windows API, no external libs!" << std::endl;
    
    TaskBot bot;
    int choice;
    
    while (true) {
        showMenu();
        std::cin >> choice;
        std::cin.ignore(); // Clear newline
        
        switch (choice) {
            case 1:
                bot.helloWorldDemo();
                break;
                
            case 2:
                bot.drawWithMouse();
                break;
                
            case 3: {
                std::cout << "Enter command: ";
                std::string cmd;
                std::getline(std::cin, cmd);
                bot.run(cmd);
                break;
            }
                
            case 4:
                std::cout << "Goodbye!" << std::endl;
                return 0;
                
            default:
                std::cout << "Invalid choice!" << std::endl;
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }
    
    return 0;
}