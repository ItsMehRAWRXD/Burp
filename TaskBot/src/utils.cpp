#include "../include/task_bot.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <unistd.h>
#include <limits.h>
#include <filesystem>

namespace TaskBot {
namespace Utils {

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string formatBytes(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
    return ss.str();
}

std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

std::string joinStrings(const std::vector<std::string>& strings, const std::string& delimiter) {
    if (strings.empty()) {
        return "";
    }
    
    std::stringstream ss;
    for (size_t i = 0; i < strings.size(); ++i) {
        if (i > 0) {
            ss << delimiter;
        }
        ss << strings[i];
    }
    
    return ss.str();
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }
    
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

bool isValidPath(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    
    // Check for invalid characters
    const std::string invalidChars = "<>:\"|?*";
    for (char c : path) {
        if (invalidChars.find(c) != std::string::npos && c != ':' && path[1] != ':') {
            // Allow : only in position 1 for Windows drive letters
            return false;
        }
        if (c < 32) { // Control characters
            return false;
        }
    }
    
    // Check path components
    auto components = splitString(path, '/');
    for (const auto& component : components) {
        if (component.empty()) {
            continue; // Allow multiple slashes
        }
        
        // Check for reserved names (Windows)
        std::string upperComponent = component;
        std::transform(upperComponent.begin(), upperComponent.end(), 
                      upperComponent.begin(), ::toupper);
        
        const std::vector<std::string> reserved = {
            "CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3", "COM4",
            "COM5", "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2", "LPT3",
            "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
        };
        
        if (std::find(reserved.begin(), reserved.end(), upperComponent) != reserved.end()) {
            return false;
        }
    }
    
    return true;
}

std::string getExecutablePath() {
    char buffer[PATH_MAX];
    
#ifdef __linux__
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        return std::string(buffer);
    }
#endif
    
    // Fallback
    return std::filesystem::current_path().string();
}

} // namespace Utils
} // namespace TaskBot