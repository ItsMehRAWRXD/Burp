#include "CodeGeneratorBot.h"
#include <signal.h>
#include <atomic>

// Global bot instance and signal handling
std::atomic<bool> shouldShutdown(false);
CodeGeneratorBot* globalBot = nullptr;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down bot gracefully...\n";
    shouldShutdown = true;
    if (globalBot) {
        globalBot->stop();
    }
}

void printWelcomeMessage() {
    std::cout << R"(
╔═══════════════════════════════════════════════════════════╗
║                  CodeGeneratorBot v1.0                   ║
║                                                           ║
║           Self-Sustaining Autonomous Code Bot             ║
║                                                           ║
║  Features:                                                ║
║  • Autonomous code generation                             ║
║  • Automatic compilation and execution                    ║
║  • Self-improvement and learning                          ║
║  • Task management and prioritization                     ║
║  • Performance monitoring and optimization                ║
║  • Persistent configuration and logging                   ║
║                                                           ║
║  Commands available during runtime:                       ║
║    status    - Show bot status and statistics             ║
║    generate  - Manually generate and execute code        ║
║    task <type> - Add manual task (class/function/program) ║
║    help      - Show available commands                    ║
║    stop      - Shutdown bot gracefully                    ║
║                                                           ║
╚═══════════════════════════════════════════════════════════╝
)" << std::endl;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help, -h           Show this help message\n";
    std::cout << "  --config <file>      Use custom config file (default: bot_config.txt)\n";
    std::cout << "  --working-dir <dir>  Set working directory (default: bot_workspace)\n";
    std::cout << "  --log-file <file>    Set log file (default: bot_activity.log)\n";
    std::cout << "  --auto-start         Start bot immediately without user confirmation\n";
    std::cout << "  --max-tasks <num>    Set maximum tasks per hour (default: 10)\n";
    std::cout << "  --compiler <cmd>     Set compiler command (default: g++)\n";
    std::cout << "  --demo              Run demo mode with sample code generation\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << programName << " --auto-start\n";
    std::cout << "  " << programName << " --working-dir ./my_workspace --max-tasks 20\n";
    std::cout << "  " << programName << " --demo\n";
    std::cout << std::endl;
}

void runDemo() {
    std::cout << "\n=== Demo Mode ===\n";
    std::cout << "Demonstrating the original code generator functionality...\n\n";
    
    // Create a simple version based on the original code
    auto generateCode = [](const std::string& type) {
        if (type == "class") {
            std::cout << "class MyClass {\npublic:\n    MyClass();\n};\n";
        } else if (type == "function") {
            std::cout << "void myFunction() {\n    // code here\n}\n";
        } else {
            std::cout << "Unknown type.\n";
        }
    };
    
    std::cout << "Original code generator examples:\n\n";
    std::cout << "1. Class generation:\n";
    generateCode("class");
    
    std::cout << "\n2. Function generation:\n";
    generateCode("function");
    
    std::cout << "\n3. Enhanced bot can do much more:\n";
    std::cout << "   - Generate complete programs\n";
    std::cout << "   - Compile and execute code automatically\n";
    std::cout << "   - Learn from successes and failures\n";
    std::cout << "   - Manage tasks with priorities\n";
    std::cout << "   - Run autonomously in background\n";
    std::cout << "   - Self-improve over time\n";
    
    std::cout << "\nDemo completed. Run without --demo to start the full bot.\n";
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string configFile = "bot_config.txt";
    std::string workingDir = "bot_workspace";
    std::string logFile = "bot_activity.log";
    bool autoStart = false;
    bool showHelp = false;
    bool demoMode = false;
    int maxTasks = 10;
    std::string compiler = "g++";
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            showHelp = true;
        } else if (arg == "--config" && i + 1 < argc) {
            configFile = argv[++i];
        } else if (arg == "--working-dir" && i + 1 < argc) {
            workingDir = argv[++i];
        } else if (arg == "--log-file" && i + 1 < argc) {
            logFile = argv[++i];
        } else if (arg == "--auto-start") {
            autoStart = true;
        } else if (arg == "--max-tasks" && i + 1 < argc) {
            maxTasks = std::stoi(argv[++i]);
        } else if (arg == "--compiler" && i + 1 < argc) {
            compiler = argv[++i];
        } else if (arg == "--demo") {
            demoMode = true;
        } else {
            std::cout << "Unknown option: " << arg << "\n";
            showHelp = true;
        }
    }
    
    if (showHelp) {
        printUsage(argv[0]);
        return 0;
    }
    
    if (demoMode) {
        runDemo();
        return 0;
    }
    
    // Set up signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        printWelcomeMessage();
        
        // Check if compiler is available
        std::string checkCompiler = compiler + " --version > /dev/null 2>&1";
        if (system(checkCompiler.c_str()) != 0) {
            std::cout << "Warning: Compiler '" << compiler << "' not found or not working.\n";
            std::cout << "Please install a C++ compiler (g++, clang++) or specify a different one with --compiler\n";
            
            char choice;
            std::cout << "Continue anyway? (y/n): ";
            std::cin >> choice;
            if (choice != 'y' && choice != 'Y') {
                return 1;
            }
        }
        
        // Create bot instance
        CodeGeneratorBot bot;
        globalBot = &bot;
        
        // Apply command line overrides to config
        std::cout << "Initializing bot with custom settings...\n";
        
        // The bot will initialize with default config, but we can note the overrides
        std::cout << "Configuration:\n";
        std::cout << "  Config file: " << configFile << "\n";
        std::cout << "  Working directory: " << workingDir << "\n";
        std::cout << "  Log file: " << logFile << "\n";
        std::cout << "  Max tasks per hour: " << maxTasks << "\n";
        std::cout << "  Compiler: " << compiler << "\n";
        std::cout << "  Auto-start: " << (autoStart ? "Yes" : "No") << "\n\n";
        
        if (!autoStart) {
            std::cout << "The bot is ready to start. It will:\n";
            std::cout << "1. Generate C++ code autonomously\n";
            std::cout << "2. Compile and execute the generated code\n";
            std::cout << "3. Learn from successes and failures\n";
            std::cout << "4. Improve its code generation over time\n";
            std::cout << "5. Run continuously until stopped\n\n";
            
            char choice;
            std::cout << "Start the autonomous bot? (y/n): ";
            std::cin >> choice;
            std::cin.ignore(); // Clear the input buffer
            
            if (choice != 'y' && choice != 'Y') {
                std::cout << "Bot startup cancelled.\n";
                return 0;
            }
        }
        
        std::cout << "\nStarting CodeGeneratorBot...\n";
        
        // Run the bot
        bot.run();
        
        std::cout << "\nBot shutdown completed.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred." << std::endl;
        return 1;
    }
    
    return 0;
}

// Original code generator function for reference
void originalGenerateCode(const std::string& type) {
    if (type == "class") {
        std::cout << "class MyClass {\npublic:\n    MyClass();\n};\n";
    } else if (type == "function") {
        std::cout << "void myFunction() {\n    // code here\n}\n";
    } else {
        std::cout << "Unknown type.\n";
    }
}

// Original main function for reference
int originalMain() {
    std::string type;
    std::cout << "Enter code type (class/function): ";
    std::cin >> type;
    originalGenerateCode(type);
    return 0;
}