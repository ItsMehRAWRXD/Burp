#include "CodeGeneratorBot.h"
#include <ctime>
#include <iomanip>

// ========== Logger Implementation ==========
Logger::Logger(const std::string& filename) : logFile(filename) {
    // Create log file if it doesn't exist
    std::ofstream file(logFile, std::ios::app);
    if (file.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        file << "\n=== Bot Session Started at " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << " ===\n";
    }
}

void Logger::log(const std::string& level, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::ofstream file(logFile, std::ios::app);
    if (file.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        file << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S") << "] "
             << "[" << level << "] " << message << std::endl;
    }
    
    // Also output to console for real-time monitoring
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::cout << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S") << "] "
              << "[" << level << "] " << message << std::endl;
}

void Logger::info(const std::string& message) { log("INFO", message); }
void Logger::warning(const std::string& message) { log("WARN", message); }
void Logger::error(const std::string& message) { log("ERROR", message); }
void Logger::debug(const std::string& message) { log("DEBUG", message); }

// ========== ConfigManager Implementation ==========
ConfigManager::ConfigManager(const std::string& filename) : configFile(filename) {
    loadConfig();
}

void ConfigManager::loadConfig() {
    std::ifstream file(configFile);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                config[key] = value;
            }
        }
    } else {
        // Create default config
        set("max_tasks_per_hour", "10");
        set("auto_compile", "true");
        set("auto_execute", "true");
        set("self_improvement_interval", "300");
        set("working_directory", "bot_workspace");
        set("compiler", "g++");
        set("compiler_flags", "-std=c++17 -O2");
        saveConfig();
    }
}

void ConfigManager::saveConfig() {
    std::ofstream file(configFile);
    if (file.is_open()) {
        file << "# CodeGeneratorBot Configuration\n";
        for (const auto& pair : config) {
            file << pair.first << "=" << pair.second << "\n";
        }
    }
}

std::string ConfigManager::get(const std::string& key, const std::string& defaultValue) {
    auto it = config.find(key);
    return (it != config.end()) ? it->second : defaultValue;
}

void ConfigManager::set(const std::string& key, const std::string& value) {
    config[key] = value;
}

int ConfigManager::getInt(const std::string& key, int defaultValue) {
    auto it = config.find(key);
    return (it != config.end()) ? std::stoi(it->second) : defaultValue;
}

void ConfigManager::setInt(const std::string& key, int value) {
    config[key] = std::to_string(value);
}

// ========== CodeGenerator Implementation ==========
CodeGenerator::CodeGenerator(std::shared_ptr<Logger> log, std::shared_ptr<ConfigManager> cfg)
    : logger(log), config(cfg), gen(rd()) {
    
    // Initialize with basic templates
    classTemplates = {
        "class {NAME} {\npublic:\n    {NAME}();\n    ~{NAME}();\n    void process();\nprivate:\n    int data;\n};",
        "class {NAME} {\npublic:\n    {NAME}(int value) : value_(value) {}\n    int getValue() const { return value_; }\n    void setValue(int v) { value_ = v; }\nprivate:\n    int value_;\n};",
        "class {NAME} {\npublic:\n    virtual ~{NAME}() = default;\n    virtual void execute() = 0;\n    virtual std::string getType() const = 0;\n};",
        "class {NAME} {\npublic:\n    {NAME}() = default;\n    void initialize();\n    void update();\n    void render();\nprivate:\n    bool initialized = false;\n};"
    };
    
    functionTemplates = {
        "void {NAME}() {\n    std::cout << \"Function {NAME} executed\\n\";\n}",
        "int {NAME}(int a, int b) {\n    return a + b;\n}",
        "std::string {NAME}(const std::string& input) {\n    return \"Processed: \" + input;\n}",
        "bool {NAME}(int value) {\n    return value > 0;\n}",
        "void {NAME}(std::vector<int>& data) {\n    std::sort(data.begin(), data.end());\n}"
    };
    
    logger->info("CodeGenerator initialized with " + std::to_string(classTemplates.size()) + 
                " class templates and " + std::to_string(functionTemplates.size()) + " function templates");
}

std::string CodeGenerator::generateClass(const std::string& className) {
    std::string name = className.empty() ? generateRandomName("Class") : className;
    std::string tmpl = selectRandomTemplate(classTemplates);
    
    // Replace {NAME} placeholder with actual name
    size_t pos = 0;
    while ((pos = tmpl.find("{NAME}", pos)) != std::string::npos) {
        tmpl.replace(pos, 6, name);
        pos += name.length();
    }
    
    logger->debug("Generated class: " + name);
    return tmpl;
}

std::string CodeGenerator::generateFunction(const std::string& functionName) {
    std::string name = functionName.empty() ? generateRandomName("function") : functionName;
    std::string tmpl = selectRandomTemplate(functionTemplates);
    
    // Replace {NAME} placeholder with actual name
    size_t pos = 0;
    while ((pos = tmpl.find("{NAME}", pos)) != std::string::npos) {
        tmpl.replace(pos, 6, name);
        pos += name.length();
    }
    
    logger->debug("Generated function: " + name);
    return tmpl;
}

std::string CodeGenerator::generateProgram(const std::string& programType) {
    std::stringstream program;
    program << "#include <iostream>\n";
    program << "#include <string>\n";
    program << "#include <vector>\n";
    program << "#include <algorithm>\n\n";
    
    if (programType == "class_demo" || programType.empty()) {
        std::string className = generateClass();
        program << className << "\n\n";
        
        // Add implementation
        std::string name = "GeneratedClass";
        size_t pos = className.find("class ");
        if (pos != std::string::npos) {
            size_t nameStart = pos + 6;
            size_t nameEnd = className.find(' ', nameStart);
            if (nameEnd == std::string::npos) nameEnd = className.find('{', nameStart);
            if (nameEnd != std::string::npos) {
                name = className.substr(nameStart, nameEnd - nameStart);
            }
        }
        
        program << name << "::" << name << "() {}\n";
        program << name << "::~" << name << "() {}\n";
        program << "void " << name << "::process() { std::cout << \"Processing...\\n\"; }\n\n";
    }
    
    // Add main function
    program << "int main() {\n";
    program << "    std::cout << \"Generated program executing...\\n\";\n";
    
    if (programType == "function_demo") {
        std::string func = generateFunction();
        program << "    " << func << "\n";
        program << "    " << generateRandomName("function") << "();\n";
    } else if (programType == "class_demo" || programType.empty()) {
        program << "    // Class demonstration\n";
        program << "    auto obj = std::make_unique<GeneratedClass>();\n";
        program << "    obj->process();\n";
    }
    
    program << "    std::cout << \"Program completed successfully\\n\";\n";
    program << "    return 0;\n";
    program << "}\n";
    
    logger->info("Generated complete program of type: " + programType);
    return program.str();
}

std::string CodeGenerator::generateRandomCode() {
    std::uniform_int_distribution<> dis(1, 3);
    int type = dis(gen);
    
    switch (type) {
        case 1: return generateClass();
        case 2: return generateFunction();
        case 3: return generateProgram();
        default: return generateFunction();
    }
}

void CodeGenerator::addClassTemplate(const std::string& tmpl) {
    classTemplates.push_back(tmpl);
    logger->debug("Added new class template");
}

void CodeGenerator::addFunctionTemplate(const std::string& tmpl) {
    functionTemplates.push_back(tmpl);
    logger->debug("Added new function template");
}

std::string CodeGenerator::generateRandomName(const std::string& prefix) {
    std::uniform_int_distribution<> dis(1000, 9999);
    return prefix + std::to_string(dis(gen));
}

std::string CodeGenerator::selectRandomTemplate(const std::vector<std::string>& templates) {
    if (templates.empty()) return "";
    std::uniform_int_distribution<> dis(0, templates.size() - 1);
    return templates[dis(gen)];
}

// ========== CodeExecutor Implementation ==========
CodeExecutor::CodeExecutor(std::shared_ptr<Logger> log, std::shared_ptr<ConfigManager> cfg)
    : logger(log), config(cfg) {
    workingDirectory = config->get("working_directory", "bot_workspace");
    
    // Create working directory if it doesn't exist
    try {
        fs::create_directories(workingDirectory);
        logger->info("Working directory set to: " + workingDirectory);
    } catch (const std::exception& e) {
        logger->error("Failed to create working directory: " + std::string(e.what()));
    }
}

bool CodeExecutor::compileCode(const std::string& sourceFile, const std::string& outputFile) {
    std::string compiler = config->get("compiler", "g++");
    std::string flags = config->get("compiler_flags", "-std=c++17 -O2");
    std::string output = outputFile.empty() ? sourceFile.substr(0, sourceFile.find_last_of('.')) : outputFile;
    
    std::string command = compiler + " " + flags + " \"" + sourceFile + "\" -o \"" + output + "\"";
    
    logger->info("Compiling: " + command);
    
    bool success = executeCommand(command);
    if (success) {
        logger->info("Compilation successful: " + output);
    } else {
        logger->error("Compilation failed for: " + sourceFile);
    }
    
    return success;
}

bool CodeExecutor::executeProgram(const std::string& executable, const std::string& args) {
    std::string command = "\"" + executable + "\"";
    if (!args.empty()) {
        command += " " + args;
    }
    
    logger->info("Executing: " + command);
    
    bool success = executeCommand(command);
    if (success) {
        logger->info("Execution successful: " + executable);
    } else {
        logger->error("Execution failed: " + executable);
    }
    
    return success;
}

std::string CodeExecutor::runCode(const std::string& code, bool keepFiles) {
    std::string sourceFile = generateTempFilename(".cpp");
    std::string outputFile = generateTempFilename("");
    
    // Write code to file
    std::ofstream file(sourceFile);
    if (!file.is_open()) {
        logger->error("Failed to create source file: " + sourceFile);
        return "";
    }
    file << code;
    file.close();
    
    // Compile and execute
    if (compileCode(sourceFile, outputFile)) {
        if (executeProgram(outputFile)) {
            logger->info("Code executed successfully");
            
            // Cleanup if requested
            if (!keepFiles) {
                fs::remove(sourceFile);
                fs::remove(outputFile);
            }
            
            return "SUCCESS";
        }
    }
    
    // Cleanup on failure
    if (!keepFiles) {
        fs::remove(sourceFile);
        fs::remove(outputFile);
    }
    
    return "FAILED";
}

bool CodeExecutor::testCode(const std::string& code) {
    return runCode(code, false) == "SUCCESS";
}

void CodeExecutor::setWorkingDirectory(const std::string& dir) {
    workingDirectory = dir;
    try {
        fs::create_directories(workingDirectory);
        logger->info("Working directory changed to: " + workingDirectory);
    } catch (const std::exception& e) {
        logger->error("Failed to change working directory: " + std::string(e.what()));
    }
}

std::string CodeExecutor::getWorkingDirectory() const {
    return workingDirectory;
}

std::string CodeExecutor::generateTempFilename(const std::string& extension) {
    static int counter = 0;
    return workingDirectory + "/temp_" + std::to_string(++counter) + extension;
}

bool CodeExecutor::executeCommand(const std::string& command) {
    std::string fullCommand = "cd \"" + workingDirectory + "\" && " + command + " 2>&1";
    
    FILE* pipe = popen(fullCommand.c_str(), "r");
    if (!pipe) {
        logger->error("Failed to execute command: " + command);
        return false;
    }
    
    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    int exitCode = pclose(pipe);
    
    if (!result.empty()) {
        logger->debug("Command output: " + result);
    }
    
    return exitCode == 0;
}

// ========== TaskManager Implementation ==========
TaskManager::TaskManager(std::shared_ptr<Logger> log, std::shared_ptr<ConfigManager> cfg)
    : logger(log), config(cfg), running(false) {
    logger->info("TaskManager initialized");
}

void TaskManager::addTask(const Task& task) {
    std::lock_guard<std::mutex> lock(queueMutex);
    taskQueue.push(task);
    queueCondition.notify_one();
    logger->debug("Task added: " + task.description);
}

bool TaskManager::getNextTask(Task& task) {
    std::unique_lock<std::mutex> lock(queueMutex);
    queueCondition.wait(lock, [this] { return !taskQueue.empty() || !running; });
    
    if (!running) return false;
    
    if (!taskQueue.empty()) {
        task = taskQueue.top();
        taskQueue.pop();
        return true;
    }
    
    return false;
}

void TaskManager::start() {
    running = true;
    logger->info("TaskManager started");
}

void TaskManager::stop() {
    running = false;
    queueCondition.notify_all();
    logger->info("TaskManager stopped");
}

bool TaskManager::isRunning() const {
    return running;
}

size_t TaskManager::getQueueSize() {
    std::lock_guard<std::mutex> lock(queueMutex);
    return taskQueue.size();
}

void TaskManager::addGenerateClassTask(const std::string& className, Priority p) {
    Task task(TaskType::GENERATE_CLASS, p, "Generate class: " + (className.empty() ? "random" : className));
    if (!className.empty()) {
        task.parameters["className"] = className;
    }
    addTask(task);
}

void TaskManager::addGenerateFunctionTask(const std::string& functionName, Priority p) {
    Task task(TaskType::GENERATE_FUNCTION, p, "Generate function: " + (functionName.empty() ? "random" : functionName));
    if (!functionName.empty()) {
        task.parameters["functionName"] = functionName;
    }
    addTask(task);
}

void TaskManager::addCompileTask(const std::string& sourceFile, Priority p) {
    Task task(TaskType::COMPILE_CODE, p, "Compile: " + sourceFile);
    task.parameters["sourceFile"] = sourceFile;
    addTask(task);
}

void TaskManager::addExecuteTask(const std::string& executable, Priority p) {
    Task task(TaskType::EXECUTE_CODE, p, "Execute: " + executable);
    task.parameters["executable"] = executable;
    addTask(task);
}

// ========== SelfImprovement Implementation ==========
SelfImprovement::SelfImprovement(std::shared_ptr<Logger> log, std::shared_ptr<ConfigManager> cfg,
                                std::shared_ptr<CodeGenerator> gen, std::shared_ptr<CodeExecutor> exec)
    : logger(log), config(cfg), codeGen(gen), executor(exec) {
    logger->info("SelfImprovement system initialized");
}

void SelfImprovement::recordPerformance(const std::string& taskType, double time, bool success) {
    PerformanceMetric metric;
    metric.taskType = taskType;
    metric.executionTime = time;
    metric.success = success;
    metric.timestamp = std::chrono::system_clock::now();
    
    metrics.push_back(metric);
    
    // Keep only recent metrics (last 1000)
    if (metrics.size() > 1000) {
        metrics.erase(metrics.begin(), metrics.begin() + 100);
    }
    
    logger->debug("Performance recorded: " + taskType + " (" + 
                 std::to_string(time) + "s, " + (success ? "success" : "failed") + ")");
}

void SelfImprovement::analyzePerformance() {
    if (metrics.empty()) return;
    
    std::map<std::string, std::vector<PerformanceMetric>> taskMetrics;
    for (const auto& metric : metrics) {
        taskMetrics[metric.taskType].push_back(metric);
    }
    
    logger->info("=== Performance Analysis ===");
    for (const auto& pair : taskMetrics) {
        const std::string& taskType = pair.first;
        double successRate = calculateSuccessRate(taskType);
        double avgTime = calculateAverageTime(taskType);
        
        logger->info(taskType + ": " + std::to_string(successRate * 100) + "% success, " +
                    std::to_string(avgTime) + "s avg time");
        
        // Adjust behavior based on performance
        if (successRate < 0.7) {
            logger->warning("Low success rate for " + taskType + ", adjusting parameters");
            // Could adjust compilation flags, timeouts, etc.
        }
    }
}

void SelfImprovement::optimizeTemplates() {
    // Analyze which templates are most successful
    logger->info("Optimizing code templates based on performance data");
    
    // This is a simplified version - in practice, you'd analyze which
    // generated code patterns are most successful and generate new templates
    if (calculateSuccessRate("GENERATE_CLASS") > 0.8) {
        // Add a new successful template variation
        std::string newTemplate = "class {NAME} {\npublic:\n    {NAME}() = default;\n    void run() { /* optimized */ }\nprivate:\n    std::string status = \"ready\";\n};";
        codeGen->addClassTemplate(newTemplate);
        logger->info("Added optimized class template");
    }
}

void SelfImprovement::updateConfiguration() {
    // Adjust configuration based on performance
    logger->info("Updating configuration based on performance analysis");
    
    double overallSuccess = 0.0;
    if (!metrics.empty()) {
        int successCount = 0;
        for (const auto& metric : metrics) {
            if (metric.success) successCount++;
        }
        overallSuccess = static_cast<double>(successCount) / metrics.size();
    }
    
    if (overallSuccess < 0.6) {
        // Reduce task frequency if success rate is low
        int currentMax = config->getInt("max_tasks_per_hour", 10);
        config->setInt("max_tasks_per_hour", std::max(1, currentMax - 1));
        logger->info("Reduced task frequency due to low success rate");
    } else if (overallSuccess > 0.9) {
        // Increase task frequency if success rate is high
        int currentMax = config->getInt("max_tasks_per_hour", 10);
        config->setInt("max_tasks_per_hour", currentMax + 1);
        logger->info("Increased task frequency due to high success rate");
    }
    
    config->saveConfig();
}

void SelfImprovement::generateBetterCode() {
    logger->info("Generating improved code based on learning");
    
    // Generate code that incorporates successful patterns
    std::string improvedCode = "#include <iostream>\n#include <memory>\n\n";
    improvedCode += codeGen->generateClass("ImprovedClass");
    improvedCode += "\n\nint main() {\n";
    improvedCode += "    auto obj = std::make_unique<ImprovedClass>();\n";
    improvedCode += "    std::cout << \"Self-improved code running...\\n\";\n";
    improvedCode += "    return 0;\n}\n";
    
    // Test the improved code
    if (executor->testCode(improvedCode)) {
        logger->info("Successfully generated and tested improved code");
    } else {
        logger->warning("Improved code failed to execute");
    }
}

double SelfImprovement::calculateSuccessRate(const std::string& taskType) {
    auto taskMetrics = std::count_if(metrics.begin(), metrics.end(),
        [&taskType](const PerformanceMetric& m) { return m.taskType == taskType; });
    
    if (taskMetrics == 0) return 0.0;
    
    auto successCount = std::count_if(metrics.begin(), metrics.end(),
        [&taskType](const PerformanceMetric& m) { 
            return m.taskType == taskType && m.success; 
        });
    
    return static_cast<double>(successCount) / taskMetrics;
}

double SelfImprovement::calculateAverageTime(const std::string& taskType) {
    double totalTime = 0.0;
    int count = 0;
    
    for (const auto& metric : metrics) {
        if (metric.taskType == taskType) {
            totalTime += metric.executionTime;
            count++;
        }
    }
    
    return count > 0 ? totalTime / count : 0.0;
}

// ========== CodeGeneratorBot Implementation ==========
CodeGeneratorBot::CodeGeneratorBot() 
    : running(false), tasksCompleted(0), tasksSuccessful(0) {
    
    // Initialize all components
    logger = std::make_shared<Logger>("bot_activity.log");
    config = std::make_shared<ConfigManager>("bot_config.txt");
    codeGenerator = std::make_shared<CodeGenerator>(logger, config);
    codeExecutor = std::make_shared<CodeExecutor>(logger, config);
    taskManager = std::make_shared<TaskManager>(logger, config);
    selfImprovement = std::make_shared<SelfImprovement>(logger, config, codeGenerator, codeExecutor);
    
    logger->info("CodeGeneratorBot initialized");
}

CodeGeneratorBot::~CodeGeneratorBot() {
    stop();
}

void CodeGeneratorBot::initialize() {
    logger->info("Initializing bot systems...");
    
    createWorkingDirectories();
    loadInitialTemplates();
    
    startTime = std::chrono::system_clock::now();
    logger->info("Bot initialization complete");
}

void CodeGeneratorBot::start() {
    if (running) return;
    
    running = true;
    taskManager->start();
    
    // Start worker threads
    workerThread = std::thread(&CodeGeneratorBot::workerLoop, this);
    selfImprovementThread = std::thread(&CodeGeneratorBot::selfImprovementLoop, this);
    
    logger->info("Bot started - entering autonomous mode");
    
    // Add initial tasks
    taskManager->addGenerateClassTask("", Priority::NORMAL);
    taskManager->addGenerateFunctionTask("", Priority::NORMAL);
    
    // Add a self-improvement task
    Task selfImprovementTask(TaskType::SELF_IMPROVE, Priority::LOW, "Self-improvement cycle");
    taskManager->addTask(selfImprovementTask);
}

void CodeGeneratorBot::stop() {
    if (!running) return;
    
    running = false;
    taskManager->stop();
    
    // Wait for threads to complete
    if (workerThread.joinable()) {
        workerThread.join();
    }
    if (selfImprovementThread.joinable()) {
        selfImprovementThread.join();
    }
    
    logger->info("Bot stopped");
}

void CodeGeneratorBot::run() {
    initialize();
    start();
    
    // Main control loop
    std::cout << "\nCodeGeneratorBot is now running autonomously!\n";
    std::cout << "Commands: 'status', 'stop', 'generate', 'task <type>', 'help'\n";
    std::cout << "Type 'stop' to shutdown the bot.\n\n";
    
    std::string command;
    while (running && std::getline(std::cin, command)) {
        if (command == "stop") {
            break;
        } else if (command == "status") {
            printStatus();
        } else if (command == "generate") {
            generateAndExecuteRandomCode();
        } else if (command == "help") {
            std::cout << "Available commands:\n";
            std::cout << "  status  - Show bot status\n";
            std::cout << "  stop    - Shutdown bot\n";
            std::cout << "  generate - Generate and execute random code\n";
            std::cout << "  task <type> - Add manual task (class/function/program)\n";
            std::cout << "  help    - Show this help\n";
        } else if (command.find("task ") == 0) {
            std::string taskType = command.substr(5);
            processUserCommand(taskType);
        } else if (!command.empty()) {
            std::cout << "Unknown command: " << command << " (type 'help' for commands)\n";
        }
    }
    
    stop();
}

void CodeGeneratorBot::generateAndExecuteRandomCode() {
    logger->info("Manual code generation requested");
    
    std::string code = codeGenerator->generateProgram();
    std::cout << "\n=== Generated Code ===\n" << code << "\n=====================\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    std::string result = codeExecutor->runCode(code, true);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    bool success = (result == "SUCCESS");
    
    selfImprovement->recordPerformance("MANUAL_GENERATE", duration / 1000.0, success);
    
    std::cout << "Execution result: " << result << " (" << duration << "ms)\n";
}

void CodeGeneratorBot::performSelfMaintenance() {
    logger->info("Performing self-maintenance");
    
    selfImprovement->analyzePerformance();
    selfImprovement->optimizeTemplates();
    selfImprovement->updateConfiguration();
    
    // Schedule next self-improvement
    Task nextImprovement(TaskType::SELF_IMPROVE, Priority::LOW, "Scheduled self-improvement");
    taskManager->addTask(nextImprovement);
}

void CodeGeneratorBot::printStatus() {
    auto now = std::chrono::system_clock::now();
    auto runtime = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
    
    std::cout << "\n=== Bot Status ===\n";
    std::cout << "Running: " << (running ? "Yes" : "No") << "\n";
    std::cout << "Runtime: " << runtime << " seconds\n";
    std::cout << "Tasks completed: " << tasksCompleted << "\n";
    std::cout << "Tasks successful: " << tasksSuccessful << "\n";
    std::cout << "Success rate: " << (tasksCompleted > 0 ? (tasksSuccessful * 100.0 / tasksCompleted) : 0) << "%\n";
    std::cout << "Queue size: " << taskManager->getQueueSize() << "\n";
    std::cout << "Working directory: " << codeExecutor->getWorkingDirectory() << "\n";
    std::cout << "==================\n\n";
}

void CodeGeneratorBot::addManualTask(TaskType type, const std::string& description) {
    Task task(type, Priority::HIGH, description.empty() ? "Manual task" : description);
    taskManager->addTask(task);
    logger->info("Manual task added: " + task.description);
}

void CodeGeneratorBot::processUserCommand(const std::string& command) {
    if (command == "class") {
        addManualTask(TaskType::GENERATE_CLASS, "User requested class generation");
    } else if (command == "function") {
        addManualTask(TaskType::GENERATE_FUNCTION, "User requested function generation");
    } else if (command == "program") {
        addManualTask(TaskType::GENERATE_PROGRAM, "User requested program generation");
    } else {
        std::cout << "Unknown task type: " << command << "\n";
        std::cout << "Available types: class, function, program\n";
    }
}

void CodeGeneratorBot::workerLoop() {
    logger->info("Worker thread started");
    
    while (running) {
        Task task(TaskType::CLEANUP, Priority::LOW, "");
        if (taskManager->getNextTask(task)) {
            auto start = std::chrono::high_resolution_clock::now();
            
            executeTask(task);
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            
            tasksCompleted++;
            if (task.completed) {
                tasksSuccessful++;
            }
            
            selfImprovement->recordPerformance(
                [task]() {
                    switch (task.type) {
                        case TaskType::GENERATE_CLASS: return "GENERATE_CLASS";
                        case TaskType::GENERATE_FUNCTION: return "GENERATE_FUNCTION";
                        case TaskType::GENERATE_PROGRAM: return "GENERATE_PROGRAM";
                        case TaskType::COMPILE_CODE: return "COMPILE_CODE";
                        case TaskType::EXECUTE_CODE: return "EXECUTE_CODE";
                        default: return "OTHER";
                    }
                }(),
                duration / 1000.0,
                task.completed
            );
        }
        
        // Small delay to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    logger->info("Worker thread stopped");
}

void CodeGeneratorBot::selfImprovementLoop() {
    logger->info("Self-improvement thread started");
    
    int interval = config->getInt("self_improvement_interval", 300);
    
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(interval));
        
        if (running) {
            performSelfMaintenance();
            
            // Add some autonomous tasks
            taskManager->addGenerateClassTask("", Priority::NORMAL);
            taskManager->addGenerateFunctionTask("", Priority::NORMAL);
            
            // Occasionally generate a full program
            static int cycleCount = 0;
            if (++cycleCount % 3 == 0) {
                Task programTask(TaskType::GENERATE_PROGRAM, Priority::NORMAL, "Autonomous program generation");
                taskManager->addTask(programTask);
            }
        }
    }
    
    logger->info("Self-improvement thread stopped");
}

void CodeGeneratorBot::executeTask(const Task& task) {
    logger->debug("Executing task: " + task.description);
    
    Task& mutableTask = const_cast<Task&>(task);
    
    try {
        switch (task.type) {
            case TaskType::GENERATE_CLASS: {
                std::string className = task.parameters.count("className") ? 
                                      task.parameters.at("className") : "";
                std::string code = codeGenerator->generateClass(className);
                mutableTask.result = code;
                mutableTask.completed = true;
                logger->info("Generated class code");
                break;
            }
            
            case TaskType::GENERATE_FUNCTION: {
                std::string functionName = task.parameters.count("functionName") ? 
                                         task.parameters.at("functionName") : "";
                std::string code = codeGenerator->generateFunction(functionName);
                mutableTask.result = code;
                mutableTask.completed = true;
                logger->info("Generated function code");
                break;
            }
            
            case TaskType::GENERATE_PROGRAM: {
                std::string programType = task.parameters.count("programType") ? 
                                        task.parameters.at("programType") : "";
                std::string code = codeGenerator->generateProgram(programType);
                std::string result = codeExecutor->runCode(code, false);
                mutableTask.result = result;
                mutableTask.completed = (result == "SUCCESS");
                logger->info("Generated and executed program: " + result);
                break;
            }
            
            case TaskType::COMPILE_CODE: {
                if (task.parameters.count("sourceFile")) {
                    bool success = codeExecutor->compileCode(task.parameters.at("sourceFile"));
                    mutableTask.completed = success;
                    mutableTask.result = success ? "COMPILED" : "FAILED";
                }
                break;
            }
            
            case TaskType::EXECUTE_CODE: {
                if (task.parameters.count("executable")) {
                    bool success = codeExecutor->executeProgram(task.parameters.at("executable"));
                    mutableTask.completed = success;
                    mutableTask.result = success ? "EXECUTED" : "FAILED";
                }
                break;
            }
            
            case TaskType::SELF_IMPROVE: {
                performSelfMaintenance();
                mutableTask.completed = true;
                mutableTask.result = "IMPROVED";
                break;
            }
            
            case TaskType::CLEANUP: {
                // Cleanup temporary files
                try {
                    for (const auto& entry : fs::directory_iterator(codeExecutor->getWorkingDirectory())) {
                        if (entry.path().filename().string().find("temp_") == 0) {
                            fs::remove(entry);
                        }
                    }
                    mutableTask.completed = true;
                    mutableTask.result = "CLEANED";
                } catch (...) {
                    mutableTask.completed = false;
                    mutableTask.result = "CLEANUP_FAILED";
                }
                break;
            }
            
            default:
                logger->warning("Unknown task type");
                mutableTask.completed = false;
                break;
        }
    } catch (const std::exception& e) {
        logger->error("Task execution failed: " + std::string(e.what()));
        mutableTask.completed = false;
        mutableTask.result = "EXCEPTION: " + std::string(e.what());
    }
}

void CodeGeneratorBot::createWorkingDirectories() {
    try {
        std::string workDir = config->get("working_directory", "bot_workspace");
        fs::create_directories(workDir);
        fs::create_directories(workDir + "/generated");
        fs::create_directories(workDir + "/compiled");
        fs::create_directories(workDir + "/logs");
        
        logger->info("Working directories created");
    } catch (const std::exception& e) {
        logger->error("Failed to create working directories: " + std::string(e.what()));
    }
}

void CodeGeneratorBot::loadInitialTemplates() {
    // Load any additional templates from files if they exist
    logger->info("Initial templates loaded");
}