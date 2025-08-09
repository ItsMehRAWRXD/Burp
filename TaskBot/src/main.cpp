#include "../include/task_bot.h"
#include <iostream>
#include <memory>
#include <csignal>

using namespace TaskBot;

// Global task manager for signal handling
TaskManager* g_taskManager = nullptr;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nReceived shutdown signal. Stopping TaskBot...\n";
        if (g_taskManager) {
            g_taskManager->stop();
        }
    }
}

// Custom task example
class BackupTask : public Task {
public:
    BackupTask(const std::string& sourceDir, const std::string& backupDir)
        : Task("Backup " + sourceDir, Priority::NORMAL),
          sourceDir_(sourceDir), backupDir_(backupDir) {}
    
    bool execute() override {
        Logger::getInstance().info("Starting backup from " + sourceDir_ + " to " + backupDir_);
        
        // Create backup directory
        if (!FileOperations::createDirectory(backupDir_)) {
            return false;
        }
        
        // Get list of files
        auto files = FileOperations::listDirectory(sourceDir_);
        int backed_up = 0;
        
        for (const auto& file : files) {
            std::string sourcePath = sourceDir_ + "/" + file;
            std::string destPath = backupDir_ + "/" + file;
            
            if (FileOperations::copyFile(sourcePath, destPath)) {
                backed_up++;
            }
        }
        
        Logger::getInstance().info("Backup completed: " + std::to_string(backed_up) + 
                                  " files backed up");
        return true;
    }
    
    std::string getDescription() const override {
        return "Backup files from " + sourceDir_ + " to " + backupDir_;
    }
    
private:
    std::string sourceDir_;
    std::string backupDir_;
};

// System health check task
class HealthCheckTask : public Task {
public:
    HealthCheckTask() : Task("System Health Check", Priority::HIGH) {}
    
    bool execute() override {
        auto info = SystemMonitor::getSystemInfo();
        
        Logger::getInstance().info("=== System Health Check ===");
        Logger::getInstance().info("CPU Usage: " + std::to_string(info.cpuUsage) + "%");
        Logger::getInstance().info("Memory: " + Utils::formatBytes(info.totalMemory - info.availableMemory) + 
                                  " / " + Utils::formatBytes(info.totalMemory));
        Logger::getInstance().info("Disk: " + Utils::formatBytes(info.totalDisk - info.availableDisk) + 
                                  " / " + Utils::formatBytes(info.totalDisk));
        Logger::getInstance().info("Process Memory: " + std::to_string(info.processMemoryMB) + " MB");
        
        // Alert if resources are high
        if (info.cpuUsage > 80.0) {
            Logger::getInstance().warning("High CPU usage detected!");
        }
        
        double memoryPercent = 100.0 * (info.totalMemory - info.availableMemory) / info.totalMemory;
        if (memoryPercent > 80.0) {
            Logger::getInstance().warning("High memory usage detected!");
        }
        
        return true;
    }
    
    std::string getDescription() const override {
        return "Perform system health check and log resource usage";
    }
};

void demonstrateTaskBot() {
    std::cout << "=== TaskBot Demonstration ===\n\n";
    
    // Initialize components
    Logger& logger = Logger::getInstance();
    logger.setLogFile("taskbot.log");
    logger.setLogLevel(Logger::LogLevel::DEBUG);
    
    ConfigManager config("taskbot.config");
    config.setString("bot_name", "TaskBot v1.0");
    config.setInt("max_workers", 4);
    config.setBool("enable_monitoring", true);
    config.setDouble("check_interval", 60.0);
    config.save();
    
    // Create task manager
    TaskManager taskManager;
    g_taskManager = &taskManager;
    
    // 1. File Operations Demo
    std::cout << "1. File Operations Demo\n";
    auto createTask = std::make_shared<FileTask>(
        "Create test file",
        FileTask::Operation::CREATE,
        "test_data.txt",
        "This is test data created by TaskBot.\nTimestamp: " + Utils::getCurrentTimestamp()
    );
    taskManager.addTask(createTask);
    
    // 2. Command Execution Demo
    std::cout << "2. Command Execution Demo\n";
    auto listTask = std::make_shared<CommandTask>(
        "List current directory",
        "ls -la"
    );
    taskManager.addTask(listTask);
    
    // 3. System Monitoring Demo
    std::cout << "3. System Monitoring Demo\n";
    auto healthTask = std::make_shared<HealthCheckTask>();
    taskManager.addTask(healthTask);
    
    // 4. Scheduled Task Demo
    std::cout << "4. Scheduled Task Demo\n";
    auto futureTime = std::chrono::system_clock::now() + std::chrono::seconds(5);
    auto delayedTask = std::make_shared<CommandTask>("Delayed echo", "echo 'This task was scheduled!'");
    auto scheduledTask = std::make_shared<ScheduledTask>("Scheduled echo", delayedTask, futureTime);
    taskManager.addTask(scheduledTask);
    
    // 5. Recurring Task Demo
    std::cout << "5. Recurring Task Demo\n";
    auto monitorTask = std::make_shared<MonitorTask>(
        "CPU Monitor",
        [](const SystemMonitor::SystemInfo& info) {
            return info.cpuUsage > 50.0; // Trigger if CPU > 50%
        },
        [](const SystemMonitor::SystemInfo& info) {
            Logger::getInstance().warning("High CPU detected: " + 
                                        std::to_string(info.cpuUsage) + "%");
        }
    );
    auto recurringMonitor = std::make_shared<RecurringTask>(
        "Recurring CPU Monitor",
        monitorTask,
        std::chrono::seconds(10),
        3 // Run max 3 times
    );
    taskManager.addTask(recurringMonitor);
    
    // 6. Custom Task Demo
    std::cout << "6. Custom Task Demo\n";
    FileOperations::createDirectory("test_source");
    FileOperations::createFile("test_source/file1.txt", "Content of file 1");
    FileOperations::createFile("test_source/file2.txt", "Content of file 2");
    
    auto backupTask = std::make_shared<BackupTask>("test_source", "test_backup");
    taskManager.addTask(backupTask);
    
    // 7. Priority Task Demo
    std::cout << "7. Priority Task Demo\n";
    auto criticalTask = std::make_shared<CommandTask>("Critical task", "echo 'CRITICAL TASK'");
    criticalTask->Task::Task("Critical echo", Priority::CRITICAL); // High priority
    taskManager.addTask(criticalTask);
    
    // Start the task manager
    std::cout << "\n--- Starting TaskBot ---\n";
    std::cout << "Press Ctrl+C to stop\n\n";
    
    taskManager.start();
    
    // Wait for tasks to complete or interrupt
    while (taskManager.isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Show pending tasks
        auto pending = taskManager.getPendingTasks();
        if (!pending.empty()) {
            std::cout << "Pending tasks: " << pending.size() << std::endl;
        }
    }
    
    // Show completed tasks
    auto completed = taskManager.getCompletedTasks();
    std::cout << "\n--- Completed Tasks ---\n";
    for (const auto& task : completed) {
        std::cout << "- " << task->getName() << " [" 
                  << (task->getStatus() == TaskStatus::COMPLETED ? "SUCCESS" : "FAILED") 
                  << "]\n";
    }
    
    // Cleanup
    FileOperations::deleteFile("test_data.txt");
    CommandExecutor::execute("rm -rf test_source test_backup");
}

int main(int argc, char* argv[]) {
    // Set up signal handling
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    try {
        if (argc > 1 && std::string(argv[1]) == "--demo") {
            demonstrateTaskBot();
        } else {
            std::cout << "TaskBot - A Self-Sustained Automation Bot\n";
            std::cout << "==========================================\n\n";
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --demo    Run demonstration of all features\n";
            std::cout << "  --help    Show this help message\n\n";
            
            std::cout << "Features:\n";
            std::cout << "- Multi-threaded task execution with priority queue\n";
            std::cout << "- File operations (create, read, update, delete, copy, move)\n";
            std::cout << "- System monitoring (CPU, memory, disk usage)\n";
            std::cout << "- Command execution with timeout support\n";
            std::cout << "- Scheduled and recurring tasks\n";
            std::cout << "- Configuration management\n";
            std::cout << "- Comprehensive logging system\n";
            std::cout << "- Custom task creation\n\n";
            
            std::cout << "Example usage in your code:\n";
            std::cout << "  TaskManager manager;\n";
            std::cout << "  auto task = std::make_shared<CommandTask>(\"List files\", \"ls -la\");\n";
            std::cout << "  manager.addTask(task);\n";
            std::cout << "  manager.start();\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}