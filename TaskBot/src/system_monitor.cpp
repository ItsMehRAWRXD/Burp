#include "../include/task_bot.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <thread>
#include <chrono>

#ifdef __linux__
#include <sys/sysinfo.h>
#include <sys/resource.h>
#endif

namespace TaskBot {

// Helper function to read first line from a file
static std::string readFirstLine(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    if (file.is_open() && std::getline(file, line)) {
        return line;
    }
    return "";
}

// Helper to parse /proc/stat for CPU usage
static void getCPUTimes(unsigned long long& totalTime, unsigned long long& idleTime) {
    std::ifstream file("/proc/stat");
    std::string line;
    
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string cpu;
        iss >> cpu;
        
        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
        iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
        
        idleTime = idle + iowait;
        totalTime = user + nice + system + idle + iowait + irq + softirq + steal;
    }
}

SystemMonitor::SystemInfo SystemMonitor::getSystemInfo() {
    SystemInfo info = {0};
    
    // Get CPU usage
    info.cpuUsage = getCPUUsage();
    
    // Get memory info
#ifdef __linux__
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        info.totalMemory = si.totalram;
        info.availableMemory = si.freeram + si.bufferram;
    }
#endif
    
    // Get disk usage
    struct statvfs stat;
    if (statvfs("/", &stat) == 0) {
        info.totalDisk = stat.f_blocks * stat.f_frsize;
        info.availableDisk = stat.f_bavail * stat.f_frsize;
    }
    
    // Get process memory usage
    std::ifstream statusFile("/proc/self/status");
    std::string line;
    while (std::getline(statusFile, line)) {
        if (line.find("VmRSS:") == 0) {
            std::istringstream iss(line);
            std::string label;
            size_t value;
            std::string unit;
            iss >> label >> value >> unit;
            info.processMemoryMB = value / 1024.0; // Convert KB to MB
            break;
        }
    }
    
    // Process CPU usage (simplified)
    info.processCpuUsage = 0.0; // Would need more complex tracking over time
    
    return info;
}

double SystemMonitor::getCPUUsage() {
    static unsigned long long prevTotalTime = 0;
    static unsigned long long prevIdleTime = 0;
    
    unsigned long long totalTime, idleTime;
    getCPUTimes(totalTime, idleTime);
    
    if (prevTotalTime == 0) {
        // First call, just store values
        prevTotalTime = totalTime;
        prevIdleTime = idleTime;
        
        // Wait a bit and measure again
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        getCPUTimes(totalTime, idleTime);
    }
    
    unsigned long long totalDiff = totalTime - prevTotalTime;
    unsigned long long idleDiff = idleTime - prevIdleTime;
    
    double usage = 100.0;
    if (totalDiff > 0) {
        usage = 100.0 * (1.0 - (double)idleDiff / totalDiff);
    }
    
    prevTotalTime = totalTime;
    prevIdleTime = idleTime;
    
    return usage;
}

size_t SystemMonitor::getMemoryUsage() {
#ifdef __linux__
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        return si.totalram - si.freeram;
    }
#endif
    return 0;
}

size_t SystemMonitor::getDiskUsage(const std::string& path) {
    struct statvfs stat;
    if (statvfs(path.c_str(), &stat) == 0) {
        size_t totalSpace = stat.f_blocks * stat.f_frsize;
        size_t freeSpace = stat.f_bavail * stat.f_frsize;
        return totalSpace - freeSpace;
    }
    return 0;
}

std::vector<std::string> SystemMonitor::getRunningProcesses() {
    std::vector<std::string> processes;
    
#ifdef __linux__
    DIR* dir = opendir("/proc");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            // Check if the directory name is a PID (all digits)
            bool isPID = true;
            for (int i = 0; entry->d_name[i] != '\0'; i++) {
                if (!isdigit(entry->d_name[i])) {
                    isPID = false;
                    break;
                }
            }
            
            if (isPID) {
                std::string cmdlinePath = std::string("/proc/") + entry->d_name + "/cmdline";
                std::ifstream cmdlineFile(cmdlinePath);
                if (cmdlineFile.is_open()) {
                    std::string cmdline;
                    std::getline(cmdlineFile, cmdline, '\0');
                    if (!cmdline.empty()) {
                        processes.push_back(entry->d_name + std::string(": ") + cmdline);
                    }
                }
            }
        }
        closedir(dir);
    }
#endif
    
    return processes;
}

bool SystemMonitor::isProcessRunning(const std::string& processName) {
    auto processes = getRunningProcesses();
    for (const auto& process : processes) {
        if (process.find(processName) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// MonitorTask implementation
MonitorTask::MonitorTask(const std::string& name,
                        std::function<bool(const SystemMonitor::SystemInfo&)> condition,
                        std::function<void(const SystemMonitor::SystemInfo&)> action)
    : Task(name, Priority::NORMAL), condition_(condition), action_(action) {}

bool MonitorTask::execute() {
    try {
        SystemMonitor::SystemInfo info = SystemMonitor::getSystemInfo();
        
        if (condition_(info)) {
            action_(info);
            Logger::getInstance().info("Monitor condition triggered for task: " + getName());
            return true;
        }
        
        return true; // Still successful even if condition not met
    } catch (const std::exception& e) {
        Logger::getInstance().error("Monitor task failed: " + getName() + " - " + e.what());
        return false;
    }
}

std::string MonitorTask::getDescription() const {
    return "System monitor task: " + getName();
}

} // namespace TaskBot