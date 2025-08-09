// TaskBot - Direct Action
// Does the task or doesn't. No bullshit.
// Compile: g++ taskbot.cpp -o taskbot.exe -luser32 -lshell32 -lwininet

#include <windows.h>
#include <wininet.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#pragma comment(lib, "wininet.lib")

class TaskBot {
private:
    bool lastResult = true;
    
public:
    // Create a file with content
    bool createFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        if (file.is_open()) {
            file << content;
            file.close();
            std::cout << "[SUCCESS] Created " << filename << std::endl;
            return true;
        }
        std::cout << "[FAILED] Could not create " << filename << std::endl;
        return false;
    }
    
    // Delete a file
    bool deleteFile(const std::string& filename) {
        if (DeleteFileA(filename.c_str())) {
            std::cout << "[SUCCESS] Deleted " << filename << std::endl;
            return true;
        }
        std::cout << "[FAILED] Could not delete " << filename << std::endl;
        return false;
    }
    
    // Create directory
    bool createDirectory(const std::string& dirname) {
        if (CreateDirectoryA(dirname.c_str(), NULL)) {
            std::cout << "[SUCCESS] Created directory " << dirname << std::endl;
            return true;
        }
        std::cout << "[FAILED] Could not create directory " << dirname << std::endl;
        return false;
    }
    
    // Run a program
    bool runProgram(const std::string& program, const std::string& params = "") {
        SHELLEXECUTEINFOA sei = {0};
        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.lpVerb = "open";
        sei.lpFile = program.c_str();
        sei.lpParameters = params.empty() ? NULL : params.c_str();
        sei.nShow = SW_SHOW;
        
        if (ShellExecuteExA(&sei)) {
            std::cout << "[SUCCESS] Started " << program << std::endl;
            return true;
        }
        std::cout << "[FAILED] Could not start " << program << std::endl;
        return false;
    }
    
    // Kill a process by name
    bool killProcess(const std::string& processName) {
        std::string cmd = "taskkill /F /IM " + processName;
        int result = system(cmd.c_str());
        if (result == 0) {
            std::cout << "[SUCCESS] Killed " << processName << std::endl;
            return true;
        }
        std::cout << "[FAILED] Could not kill " << processName << std::endl;
        return false;
    }
    
    // Download a file from URL
    bool downloadFile(const std::string& url, const std::string& filename) {
        HINTERNET hInternet = InternetOpenA("TaskBot", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) {
            std::cout << "[FAILED] No internet connection" << std::endl;
            return false;
        }
        
        HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hUrl) {
            InternetCloseHandle(hInternet);
            std::cout << "[FAILED] Could not open URL" << std::endl;
            return false;
        }
        
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            InternetCloseHandle(hUrl);
            InternetCloseHandle(hInternet);
            std::cout << "[FAILED] Could not create output file" << std::endl;
            return false;
        }
        
        char buffer[4096];
        DWORD bytesRead;
        while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            file.write(buffer, bytesRead);
        }
        
        file.close();
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        
        std::cout << "[SUCCESS] Downloaded " << url << " to " << filename << std::endl;
        return true;
    }
    
    // Copy file
    bool copyFile(const std::string& source, const std::string& dest) {
        if (CopyFileA(source.c_str(), dest.c_str(), FALSE)) {
            std::cout << "[SUCCESS] Copied " << source << " to " << dest << std::endl;
            return true;
        }
        std::cout << "[FAILED] Could not copy file" << std::endl;
        return false;
    }
    
    // Move file
    bool moveFile(const std::string& source, const std::string& dest) {
        if (MoveFileA(source.c_str(), dest.c_str())) {
            std::cout << "[SUCCESS] Moved " << source << " to " << dest << std::endl;
            return true;
        }
        std::cout << "[FAILED] Could not move file" << std::endl;
        return false;
    }
    
    // Execute system command
    bool executeCommand(const std::string& command) {
        int result = system(command.c_str());
        if (result == 0) {
            std::cout << "[SUCCESS] Executed: " << command << std::endl;
            return true;
        }
        std::cout << "[FAILED] Command returned: " << result << std::endl;
        return false;
    }
    
    // Type text in active window
    bool typeText(const std::string& text) {
        Sleep(2000); // Give user time to focus target window
        
        for (char c : text) {
            if (c == '\n') {
                keybd_event(VK_RETURN, 0, 0, 0);
                keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
            } else {
                SHORT vk = VkKeyScan(c);
                BYTE virtualKey = LOBYTE(vk);
                BYTE shift = HIBYTE(vk) & 1;
                
                if (shift) keybd_event(VK_SHIFT, 0, 0, 0);
                keybd_event(virtualKey, 0, 0, 0);
                keybd_event(virtualKey, 0, KEYEVENTF_KEYUP, 0);
                if (shift) keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
            }
            Sleep(30);
        }
        
        std::cout << "[SUCCESS] Typed text" << std::endl;
        return true;
    }
    
    // Click at position
    bool clickAt(int x, int y) {
        SetCursorPos(x, y);
        mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        std::cout << "[SUCCESS] Clicked at " << x << "," << y << std::endl;
        return true;
    }
    
    // Take screenshot
    bool screenshot(const std::string& filename) {
        // This is a complex operation - simplified for now
        std::cout << "[FAILED] Screenshot not implemented yet" << std::endl;
        return false;
    }
    
    // Send keypress
    bool sendKey(BYTE vk) {
        keybd_event(vk, 0, 0, 0);
        Sleep(50);
        keybd_event(vk, 0, KEYEVENTF_KEYUP, 0);
        std::cout << "[SUCCESS] Sent key" << std::endl;
        return true;
    }
    
    // Check if file exists
    bool fileExists(const std::string& filename) {
        DWORD attrib = GetFileAttributesA(filename.c_str());
        bool exists = (attrib != INVALID_FILE_ATTRIBUTES);
        std::cout << "[" << (exists ? "EXISTS" : "NOT FOUND") << "] " << filename << std::endl;
        return exists;
    }
    
    // Get file size
    bool getFileSize(const std::string& filename, DWORD& size) {
        WIN32_FILE_ATTRIBUTE_DATA fad;
        if (GetFileAttributesExA(filename.c_str(), GetFileExInfoStandard, &fad)) {
            size = fad.nFileSizeLow;
            std::cout << "[SUCCESS] " << filename << " is " << size << " bytes" << std::endl;
            return true;
        }
        std::cout << "[FAILED] Could not get file size" << std::endl;
        return false;
    }
};

int main() {
    std::cout << "TaskBot - Direct Action" << std::endl;
    std::cout << "=======================" << std::endl;
    std::cout << "Either does it or doesn't.\n" << std::endl;
    
    TaskBot bot;
    std::string input;
    
    while (true) {
        std::cout << "\n> ";
        std::getline(std::cin, input);
        
        if (input == "exit" || input == "quit") {
            break;
        }
        
        // Parse command
        std::istringstream iss(input);
        std::string cmd;
        iss >> cmd;
        
        if (cmd == "create") {
            std::string filename, content;
            iss >> filename;
            std::getline(iss, content);
            bot.createFile(filename, content);
        }
        else if (cmd == "delete") {
            std::string filename;
            iss >> filename;
            bot.deleteFile(filename);
        }
        else if (cmd == "mkdir") {
            std::string dirname;
            iss >> dirname;
            bot.createDirectory(dirname);
        }
        else if (cmd == "run") {
            std::string program;
            iss >> program;
            bot.runProgram(program);
        }
        else if (cmd == "kill") {
            std::string process;
            iss >> process;
            bot.killProcess(process);
        }
        else if (cmd == "download") {
            std::string url, filename;
            iss >> url >> filename;
            bot.downloadFile(url, filename);
        }
        else if (cmd == "copy") {
            std::string src, dst;
            iss >> src >> dst;
            bot.copyFile(src, dst);
        }
        else if (cmd == "move") {
            std::string src, dst;
            iss >> src >> dst;
            bot.moveFile(src, dst);
        }
        else if (cmd == "exec") {
            std::string command;
            std::getline(iss, command);
            bot.executeCommand(command);
        }
        else if (cmd == "type") {
            std::string text;
            std::getline(iss, text);
            bot.typeText(text);
        }
        else if (cmd == "click") {
            int x, y;
            iss >> x >> y;
            bot.clickAt(x, y);
        }
        else if (cmd == "exists") {
            std::string filename;
            iss >> filename;
            bot.fileExists(filename);
        }
        else if (cmd == "size") {
            std::string filename;
            iss >> filename;
            DWORD size;
            bot.getFileSize(filename, size);
        }
        else if (cmd == "help") {
            std::cout << "Commands:" << std::endl;
            std::cout << "  create <file> <content> - Create file" << std::endl;
            std::cout << "  delete <file>          - Delete file" << std::endl;
            std::cout << "  mkdir <dir>            - Create directory" << std::endl;
            std::cout << "  run <program>          - Run program" << std::endl;
            std::cout << "  kill <process>         - Kill process" << std::endl;
            std::cout << "  download <url> <file>  - Download file" << std::endl;
            std::cout << "  copy <src> <dst>       - Copy file" << std::endl;
            std::cout << "  move <src> <dst>       - Move file" << std::endl;
            std::cout << "  exec <command>         - Execute command" << std::endl;
            std::cout << "  type <text>            - Type text (2s delay)" << std::endl;
            std::cout << "  click <x> <y>          - Click at position" << std::endl;
            std::cout << "  exists <file>          - Check if file exists" << std::endl;
            std::cout << "  size <file>            - Get file size" << std::endl;
            std::cout << "  exit                   - Exit" << std::endl;
        }
        else {
            std::cout << "[ERROR] Unknown command: " << cmd << std::endl;
        }
    }
    
    return 0;
}