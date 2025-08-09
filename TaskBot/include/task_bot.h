#ifndef TASK_BOT_H
#define TASK_BOT_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace TaskBot {

// Forward declarations
class Task;
class TaskManager;
class FileOperations;
class SystemMonitor;
class CommandExecutor;
class ConfigManager;
class Logger;

// Task priorities
enum class Priority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};

// Task status
enum class TaskStatus {
    PENDING,
    RUNNING,
    COMPLETED,
    FAILED,
    CANCELLED
};

// Base Task class
class Task {
public:
    Task(const std::string& name, Priority priority = Priority::NORMAL);
    virtual ~Task() = default;
    
    virtual bool execute() = 0;
    virtual std::string getDescription() const = 0;
    
    std::string getName() const { return name_; }
    Priority getPriority() const { return priority_; }
    TaskStatus getStatus() const { return status_; }
    void setStatus(TaskStatus status) { status_ = status; }
    
protected:
    std::string name_;
    Priority priority_;
    TaskStatus status_;
};

// Task Manager - Core of the bot
class TaskManager {
public:
    TaskManager();
    ~TaskManager();
    
    void addTask(std::shared_ptr<Task> task);
    void start();
    void stop();
    void executeOnce();
    bool isRunning() const { return running_; }
    
    std::vector<std::shared_ptr<Task>> getPendingTasks() const;
    std::vector<std::shared_ptr<Task>> getCompletedTasks() const;
    
private:
    void workerThread();
    std::shared_ptr<Task> getNextTask();
    
    std::priority_queue<std::shared_ptr<Task>, 
                       std::vector<std::shared_ptr<Task>>,
                       std::function<bool(const std::shared_ptr<Task>&, 
                                        const std::shared_ptr<Task>&)>> taskQueue_;
    
    std::vector<std::shared_ptr<Task>> completedTasks_;
    std::vector<std::thread> workers_;
    std::mutex queueMutex_;
    std::condition_variable cv_;
    bool running_;
    size_t numWorkers_;
};

// File Operations
class FileOperations {
public:
    static bool createFile(const std::string& path, const std::string& content);
    static bool readFile(const std::string& path, std::string& content);
    static bool updateFile(const std::string& path, const std::string& content);
    static bool deleteFile(const std::string& path);
    static bool fileExists(const std::string& path);
    static std::vector<std::string> listDirectory(const std::string& path);
    static bool createDirectory(const std::string& path);
    static bool copyFile(const std::string& source, const std::string& dest);
    static bool moveFile(const std::string& source, const std::string& dest);
    static size_t getFileSize(const std::string& path);
};

// System Monitor
class SystemMonitor {
public:
    struct SystemInfo {
        double cpuUsage;
        size_t totalMemory;
        size_t availableMemory;
        size_t totalDisk;
        size_t availableDisk;
        double processMemoryMB;
        double processCpuUsage;
    };
    
    static SystemInfo getSystemInfo();
    static double getCPUUsage();
    static size_t getMemoryUsage();
    static size_t getDiskUsage(const std::string& path = "/");
    static std::vector<std::string> getRunningProcesses();
    static bool isProcessRunning(const std::string& processName);
};

// Command Executor
class CommandExecutor {
public:
    struct CommandResult {
        int exitCode;
        std::string output;
        std::string error;
        bool success;
    };
    
    static CommandResult execute(const std::string& command);
    static CommandResult executeWithTimeout(const std::string& command, int timeoutSeconds);
    static bool executeAsync(const std::string& command);
    static std::vector<std::string> getEnvironmentVariables();
    static bool setEnvironmentVariable(const std::string& name, const std::string& value);
};

// Configuration Manager
class ConfigManager {
public:
    ConfigManager(const std::string& configFile);
    
    bool load();
    bool save();
    
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;
    bool getBool(const std::string& key, bool defaultValue = false) const;
    double getDouble(const std::string& key, double defaultValue = 0.0) const;
    
    void setString(const std::string& key, const std::string& value);
    void setInt(const std::string& key, int value);
    void setBool(const std::string& key, bool value);
    void setDouble(const std::string& key, double value);
    
    std::vector<std::string> getKeys() const;
    
private:
    std::string configFile_;
    std::map<std::string, std::string> config_;
};

// Logger
class Logger {
public:
    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };
    
    static Logger& getInstance();
    
    void setLogFile(const std::string& filename);
    void setLogLevel(LogLevel level);
    void enableConsoleOutput(bool enable);
    
    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);
    
private:
    Logger();
    ~Logger();
    
    std::string logFile_;
    LogLevel currentLevel_;
    bool consoleOutput_;
    std::mutex logMutex_;
};

// Predefined Task Types
class FileTask : public Task {
public:
    enum class Operation {
        CREATE,
        READ,
        UPDATE,
        DELETE,
        COPY,
        MOVE
    };
    
    FileTask(const std::string& name, Operation op, const std::string& path,
             const std::string& content = "", const std::string& destPath = "");
    
    bool execute() override;
    std::string getDescription() const override;
    
private:
    Operation operation_;
    std::string path_;
    std::string content_;
    std::string destPath_;
};

class CommandTask : public Task {
public:
    CommandTask(const std::string& name, const std::string& command, 
                int timeoutSeconds = 0);
    
    bool execute() override;
    std::string getDescription() const override;
    
    CommandExecutor::CommandResult getResult() const { return result_; }
    
private:
    std::string command_;
    int timeoutSeconds_;
    CommandExecutor::CommandResult result_;
};

class MonitorTask : public Task {
public:
    MonitorTask(const std::string& name, 
                std::function<bool(const SystemMonitor::SystemInfo&)> condition,
                std::function<void(const SystemMonitor::SystemInfo&)> action);
    
    bool execute() override;
    std::string getDescription() const override;
    
private:
    std::function<bool(const SystemMonitor::SystemInfo&)> condition_;
    std::function<void(const SystemMonitor::SystemInfo&)> action_;
};

class ScheduledTask : public Task {
public:
    ScheduledTask(const std::string& name, std::shared_ptr<Task> task,
                  std::chrono::system_clock::time_point executeTime);
    
    bool execute() override;
    std::string getDescription() const override;
    
    bool isReady() const;
    
private:
    std::shared_ptr<Task> task_;
    std::chrono::system_clock::time_point executeTime_;
};

class RecurringTask : public Task {
public:
    RecurringTask(const std::string& name, std::shared_ptr<Task> task,
                  std::chrono::seconds interval, int maxRuns = -1);
    
    bool execute() override;
    std::string getDescription() const override;
    
private:
    std::shared_ptr<Task> task_;
    std::chrono::seconds interval_;
    std::chrono::system_clock::time_point lastRun_;
    int maxRuns_;
    int runCount_;
};

// Utility functions
namespace Utils {
    std::string getCurrentTimestamp();
    std::string formatBytes(size_t bytes);
    std::vector<std::string> splitString(const std::string& str, char delimiter);
    std::string joinStrings(const std::vector<std::string>& strings, const std::string& delimiter);
    std::string trim(const std::string& str);
    bool isValidPath(const std::string& path);
    std::string getExecutablePath();
}

} // namespace TaskBot

#endif // TASK_BOT_H