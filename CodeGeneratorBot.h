#ifndef CODE_GENERATOR_BOT_H
#define CODE_GENERATOR_BOT_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <random>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstdlib>
#include <algorithm>

namespace fs = std::filesystem;

// Forward declarations
class Task;
class CodeGenerator;
class CodeExecutor;
class TaskManager;
class SelfImprovement;
class ConfigManager;
class Logger;

// Enum for task types
enum class TaskType {
    GENERATE_CLASS,
    GENERATE_FUNCTION,
    GENERATE_PROGRAM,
    COMPILE_CODE,
    EXECUTE_CODE,
    SELF_IMPROVE,
    CLEANUP,
    ANALYZE_CODE
};

// Enum for task priority
enum class Priority {
    LOW = 1,
    NORMAL = 2,
    HIGH = 3,
    CRITICAL = 4
};

// Task structure
class Task {
public:
    TaskType type;
    Priority priority;
    std::string description;
    std::map<std::string, std::string> parameters;
    std::string outputFile;
    bool completed;
    std::string result;
    std::chrono::system_clock::time_point createdAt;
    
    Task(TaskType t, Priority p, const std::string& desc) 
        : type(t), priority(p), description(desc), completed(false),
          createdAt(std::chrono::system_clock::now()) {}
    
    bool operator<(const Task& other) const {
        return static_cast<int>(priority) < static_cast<int>(other.priority);
    }
};

// Logger class for tracking bot activities
class Logger {
private:
    std::string logFile;
    std::mutex logMutex;

public:
    Logger(const std::string& filename = "bot_activity.log");
    void log(const std::string& level, const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void debug(const std::string& message);
};

// Configuration manager
class ConfigManager {
private:
    std::map<std::string, std::string> config;
    std::string configFile;

public:
    ConfigManager(const std::string& filename = "bot_config.txt");
    void loadConfig();
    void saveConfig();
    std::string get(const std::string& key, const std::string& defaultValue = "");
    void set(const std::string& key, const std::string& value);
    int getInt(const std::string& key, int defaultValue = 0);
    void setInt(const std::string& key, int value);
};

// Enhanced code generator
class CodeGenerator {
private:
    std::shared_ptr<Logger> logger;
    std::shared_ptr<ConfigManager> config;
    std::vector<std::string> classTemplates;
    std::vector<std::string> functionTemplates;
    std::random_device rd;
    std::mt19937 gen;

public:
    CodeGenerator(std::shared_ptr<Logger> log, std::shared_ptr<ConfigManager> cfg);
    
    std::string generateClass(const std::string& className = "");
    std::string generateFunction(const std::string& functionName = "");
    std::string generateProgram(const std::string& programType = "");
    std::string generateRandomCode();
    
    void addClassTemplate(const std::string& tmpl);
    void addFunctionTemplate(const std::string& tmpl);
    
private:
    std::string generateRandomName(const std::string& prefix = "Generated");
    std::string selectRandomTemplate(const std::vector<std::string>& templates);
};

// Code executor and compiler
class CodeExecutor {
private:
    std::shared_ptr<Logger> logger;
    std::shared_ptr<ConfigManager> config;
    std::string workingDirectory;

public:
    CodeExecutor(std::shared_ptr<Logger> log, std::shared_ptr<ConfigManager> cfg);
    
    bool compileCode(const std::string& sourceFile, const std::string& outputFile = "");
    bool executeProgram(const std::string& executable, const std::string& args = "");
    std::string runCode(const std::string& code, bool keepFiles = false);
    bool testCode(const std::string& code);
    
    void setWorkingDirectory(const std::string& dir);
    std::string getWorkingDirectory() const;

private:
    std::string generateTempFilename(const std::string& extension = ".cpp");
    bool executeCommand(const std::string& command);
};

// Self-improvement system
class SelfImprovement {
private:
    std::shared_ptr<Logger> logger;
    std::shared_ptr<ConfigManager> config;
    std::shared_ptr<CodeGenerator> codeGen;
    std::shared_ptr<CodeExecutor> executor;
    
    struct PerformanceMetric {
        std::string taskType;
        double executionTime;
        bool success;
        std::chrono::system_clock::time_point timestamp;
    };
    
    std::vector<PerformanceMetric> metrics;

public:
    SelfImprovement(std::shared_ptr<Logger> log, std::shared_ptr<ConfigManager> cfg,
                   std::shared_ptr<CodeGenerator> gen, std::shared_ptr<CodeExecutor> exec);
    
    void recordPerformance(const std::string& taskType, double time, bool success);
    void analyzePerformance();
    void optimizeTemplates();
    void updateConfiguration();
    void generateBetterCode();
    
private:
    double calculateSuccessRate(const std::string& taskType);
    double calculateAverageTime(const std::string& taskType);
};

// Task manager for handling queued tasks
class TaskManager {
private:
    std::priority_queue<Task> taskQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::shared_ptr<Logger> logger;
    std::shared_ptr<ConfigManager> config;
    bool running;

public:
    TaskManager(std::shared_ptr<Logger> log, std::shared_ptr<ConfigManager> cfg);
    
    void addTask(const Task& task);
    bool getNextTask(Task& task);
    void start();
    void stop();
    bool isRunning() const;
    size_t getQueueSize();
    
    void addGenerateClassTask(const std::string& className = "", Priority p = Priority::NORMAL);
    void addGenerateFunctionTask(const std::string& functionName = "", Priority p = Priority::NORMAL);
    void addCompileTask(const std::string& sourceFile, Priority p = Priority::NORMAL);
    void addExecuteTask(const std::string& executable, Priority p = Priority::NORMAL);
};

// Main bot class
class CodeGeneratorBot {
private:
    std::shared_ptr<Logger> logger;
    std::shared_ptr<ConfigManager> config;
    std::shared_ptr<CodeGenerator> codeGenerator;
    std::shared_ptr<CodeExecutor> codeExecutor;
    std::shared_ptr<TaskManager> taskManager;
    std::shared_ptr<SelfImprovement> selfImprovement;
    
    bool running;
    std::thread workerThread;
    std::thread selfImprovementThread;
    
    int tasksCompleted;
    int tasksSuccessful;
    std::chrono::system_clock::time_point startTime;

public:
    CodeGeneratorBot();
    ~CodeGeneratorBot();
    
    void initialize();
    void start();
    void stop();
    void run();
    
    void generateAndExecuteRandomCode();
    void performSelfMaintenance();
    void printStatus();
    
    // Manual task addition methods
    void addManualTask(TaskType type, const std::string& description = "");
    void processUserCommand(const std::string& command);
    
private:
    void workerLoop();
    void selfImprovementLoop();
    void executeTask(const Task& task);
    void createWorkingDirectories();
    void loadInitialTemplates();
};

#endif // CODE_GENERATOR_BOT_H