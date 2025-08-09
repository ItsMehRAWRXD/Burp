# CodeGeneratorBot v1.0

A **Self-Sustaining Autonomous Code Generator Bot** that evolved from a simple C++ code generator into a sophisticated, self-improving system.

## 🤖 What is CodeGeneratorBot?

CodeGeneratorBot is a completely autonomous bot that:
- **Generates C++ code** automatically (classes, functions, complete programs)
- **Compiles and executes** generated code
- **Learns from successes and failures** to improve over time
- **Manages tasks** with intelligent prioritization
- **Runs autonomously** without human intervention
- **Self-improves** by analyzing performance and optimizing templates

## 🚀 Features

### Core Capabilities
- **Autonomous Code Generation**: Creates classes, functions, and complete programs
- **Auto-Compilation**: Compiles generated code using configurable compiler
- **Auto-Execution**: Runs compiled programs and captures results
- **Task Management**: Priority-based task queue with threading
- **Performance Monitoring**: Tracks success rates and execution times
- **Self-Improvement**: Learns and optimizes based on performance data

### Advanced Features
- **Multi-threaded Architecture**: Separate threads for task processing and self-improvement
- **Persistent Configuration**: Saves and loads settings automatically
- **Comprehensive Logging**: Detailed activity logs with timestamps
- **Signal Handling**: Graceful shutdown on SIGINT/SIGTERM
- **Template System**: Expandable code templates with randomization
- **Working Directory Management**: Organized file structure for generated code

## 📋 Requirements

- **C++ Compiler**: g++ (with C++17 support) or clang++
- **Operating System**: Linux, macOS, or Windows with WSL
- **Memory**: Minimum 512MB RAM
- **Disk Space**: 100MB for bot + space for generated code

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential g++
```

**CentOS/RHEL/Fedora:**
```bash
sudo yum install gcc-c++
# or
sudo dnf install gcc-c++
```

**macOS:**
```bash
xcode-select --install
```

## 🔧 Building

### Quick Start
```bash
# Clone or extract the bot files
# Navigate to the directory containing the source files

# Build the bot
make

# Run demo to see original functionality
make demo

# Run the bot with confirmation
make run

# Run the bot in auto-start mode
make run-auto
```

### Manual Build
```bash
g++ -std=c++17 -Wall -Wextra -O2 -pthread main.cpp CodeGeneratorBot.cpp -o codegen_bot
```

### Build Options
```bash
make help           # Show all available commands
make debug          # Build with debug symbols
make check-deps     # Verify compiler installation
make test-build     # Compile without running
make clean          # Remove build artifacts
make distclean      # Remove all generated files
```

## 🎯 Usage

### Basic Usage
```bash
# Show help
./bin/codegen_bot --help

# Run with default settings
./bin/codegen_bot

# Auto-start without confirmation
./bin/codegen_bot --auto-start

# Demo mode (shows original simple functionality)
./bin/codegen_bot --demo
```

### Command Line Options
```bash
--help, -h           Show help message
--config <file>      Use custom config file (default: bot_config.txt)
--working-dir <dir>  Set working directory (default: bot_workspace)
--log-file <file>    Set log file (default: bot_activity.log)
--auto-start         Start immediately without confirmation
--max-tasks <num>    Set maximum tasks per hour (default: 10)
--compiler <cmd>     Set compiler command (default: g++)
--demo              Run demo mode showing original functionality
```

### Interactive Commands (while running)
When the bot is running, you can enter these commands:
- `status` - Show bot status and statistics
- `generate` - Manually trigger code generation
- `task class` - Add a class generation task
- `task function` - Add a function generation task  
- `task program` - Add a program generation task
- `help` - Show available commands
- `stop` - Shutdown the bot gracefully

## 📁 File Structure

```
codegen_bot/
├── main.cpp              # Main executable entry point
├── CodeGeneratorBot.h    # Header with all class definitions
├── CodeGeneratorBot.cpp  # Implementation of all classes
├── Makefile             # Build configuration
├── README.md            # This documentation
├── bot_config.txt       # Configuration file (auto-created)
├── bot_activity.log     # Activity log (auto-created)
├── bin/                 # Compiled executables
│   └── codegen_bot
├── obj/                 # Object files
└── bot_workspace/       # Working directory for generated code
    ├── generated/       # Generated source files
    ├── compiled/        # Compiled executables
    └── logs/           # Additional logs
```

## ⚙️ Configuration

The bot creates a `bot_config.txt` file with default settings:
```
# CodeGeneratorBot Configuration
max_tasks_per_hour=10
auto_compile=true
auto_execute=true
self_improvement_interval=300
working_directory=bot_workspace
compiler=g++
compiler_flags=-std=c++17 -O2
```

You can edit this file to customize behavior, or specify a different config file with `--config`.

## 🧠 How It Works

### Architecture Overview
1. **TaskManager**: Manages a priority queue of tasks using threading
2. **CodeGenerator**: Creates C++ code from templates with randomization
3. **CodeExecutor**: Compiles and executes generated code
4. **SelfImprovement**: Analyzes performance and optimizes the system
5. **Logger**: Records all activities with timestamps
6. **ConfigManager**: Handles persistent configuration

### Self-Improvement Process
1. **Performance Tracking**: Records execution time and success rate for each task
2. **Analysis**: Periodically analyzes performance metrics by task type
3. **Template Optimization**: Adds new successful code templates
4. **Configuration Tuning**: Adjusts task frequency based on success rates
5. **Continuous Learning**: Improves over time without human intervention

### Evolution from Original Code
The bot evolved from this simple original function:
```cpp
void generateCode(const std::string& type) {
    if (type == "class") {
        std::cout << "class MyClass {\npublic:\n    MyClass();\n};\n";
    } else if (type == "function") {
        std::cout << "void myFunction() {\n    // code here\n}\n";
    } else {
        std::cout << "Unknown type.\n";
    }
}
```

Into a fully autonomous system that can:
- Generate multiple code variants
- Compile and test the code
- Learn from results
- Improve automatically
- Run continuously
- Handle errors gracefully

## 📊 Monitoring

### Real-time Monitoring
- Console output shows real-time activity
- Use `status` command for current statistics
- Log files contain detailed historical data

### Log Files
- `bot_activity.log` - Main activity log with timestamps
- `bot_workspace/logs/` - Additional detailed logs

### Performance Metrics
The bot tracks:
- Tasks completed vs. successful
- Success rate by task type
- Average execution time
- Self-improvement cycles
- Configuration changes

## 🔒 Safety Features

- **Sandboxed Execution**: All generated code runs in isolated working directory
- **Resource Limits**: Configurable task frequency limits
- **Error Handling**: Graceful handling of compilation/execution failures
- **Signal Handling**: Clean shutdown on Ctrl+C or system signals
- **File Cleanup**: Automatic cleanup of temporary files

## 🛠️ Troubleshooting

### Common Issues

**"Compiler not found"**
```bash
# Install g++
sudo apt install build-essential
# or specify different compiler
./bin/codegen_bot --compiler clang++
```

**"Permission denied"**
```bash
# Make sure the executable has proper permissions
chmod +x bin/codegen_bot
```

**"Working directory errors"**
```bash
# Ensure you have write permissions in the current directory
# or specify a different working directory
./bin/codegen_bot --working-dir /tmp/bot_workspace
```

### Debug Mode
```bash
make debug
./bin/codegen_bot --auto-start
```

### Verbose Logging
Check the log files for detailed information:
```bash
tail -f bot_activity.log
```

## 🎨 Customization

### Adding New Code Templates
Edit `CodeGeneratorBot.cpp` and add templates to the constructor:
```cpp
classTemplates.push_back("your_new_template_here");
functionTemplates.push_back("your_function_template");
```

### Modifying Self-Improvement Logic
Customize the `SelfImprovement` class methods:
- `analyzePerformance()` - Change analysis criteria
- `optimizeTemplates()` - Modify template optimization
- `updateConfiguration()` - Adjust auto-tuning behavior

### Custom Task Types
Add new `TaskType` enums and corresponding logic in `executeTask()`.

## 🤝 Contributing

This is a self-contained project, but you can extend it by:
1. Adding new code generation templates
2. Implementing additional programming languages
3. Adding more sophisticated self-improvement algorithms
4. Creating a web interface for monitoring
5. Adding integration with external APIs

## 📜 License

This project is provided as-is for educational and experimental purposes. Use at your own risk.

## 🙏 Acknowledgments

- Evolved from a simple C++ code generator concept
- Built using modern C++17 features
- Inspired by autonomous systems and machine learning principles

---

**⚡ Ready to start?**
```bash
make run-auto
```

Let the bot generate code, learn, and improve itself! 🚀