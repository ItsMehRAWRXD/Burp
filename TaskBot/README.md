# TaskBot - Advanced Automation Framework with AutoPilot

TaskBot is a powerful, self-sustained automation bot framework that can control your entire system, automate complex tasks, and integrate with various applications and IDEs. It features an advanced AutoPilot system that can take control of your mouse and keyboard, perform web searches, and orchestrate multiple applications simultaneously.

## 🚀 Features

### Core TaskBot Features
- **Multi-threaded Task Execution**: Priority-based task queue with worker thread pool
- **File Operations**: Complete file system manipulation (create, read, update, delete, copy, move)
- **System Monitoring**: Real-time CPU, memory, and disk usage tracking
- **Command Execution**: Run system commands with timeout support
- **Task Scheduling**: Scheduled and recurring tasks with cron-like functionality
- **Configuration Management**: Flexible configuration system with multiple data types
- **Comprehensive Logging**: Multi-level logging with file and console output
- **Custom Task Creation**: Extensible framework for creating custom automation tasks

### AutoPilot Features
- **System-wide Automation**: Control mouse and keyboard across all applications
- **Application Management**: Find, focus, resize, and control application windows
- **Screen Capture & OCR**: Capture screenshots and extract text from screen
- **Web Search Integration**: Google search integration for finding automation solutions
- **Macro Recording**: Record and playback complex automation sequences
- **Multi-app Orchestration**: Synchronize data and actions across multiple applications
- **IDE Integration**: Plugin system for VSCode, IntelliJ, and other IDEs
- **Visual Automation**: Find and click on UI elements using image recognition

## 🛠️ Installation

### Prerequisites
- Linux (Ubuntu/Debian recommended)
- C++17 compatible compiler (g++ 7.0+)
- CMake 3.10+
- X11 display server

### Dependencies
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libx11-dev \
    libxtst-dev \
    libcurl4-openssl-dev \
    libjsoncpp-dev \
    pkg-config
```

### Building TaskBot
```bash
# Clone the repository
git clone https://github.com/yourusername/TaskBot.git
cd TaskBot

# Make build script executable
chmod +x build.sh

# Build the project
./build.sh

# Or build with debug symbols
./build.sh --debug

# Run demo after building
./build.sh --run-demo
```

## 🎮 Usage

### Basic Usage
```bash
# Show help
./build/taskbot --help

# Run core TaskBot demonstration
./build/taskbot --demo

# Run AutoPilot demonstration (WARNING: Controls mouse/keyboard!)
./build/taskbot --autopilot
```

### Using TaskBot in Your Code

```cpp
#include "task_bot.h"
#include "autopilot.h"

using namespace TaskBot;

int main() {
    // Create task manager
    TaskManager manager;
    
    // Add a simple command task
    auto task = std::make_shared<CommandTask>("List files", "ls -la");
    manager.addTask(task);
    
    // Start task execution
    manager.start();
    
    // Wait for completion
    while (manager.isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
```

### AutoPilot Example

```cpp
// Initialize AutoPilot
auto autoPilot = std::make_shared<AutoPilotManager>();

// Move mouse to position
autoPilot->getInputSimulator().moveMouse(500, 300);

// Type text
autoPilot->getInputSimulator().typeText("Hello, World!");

// Execute keyboard shortcut
autoPilot->getInputSimulator().executeShortcut("Ctrl+S");

// Take screenshot
autoPilot->getScreenCapture().captureScreen("screenshot.png");

// Search Google and get code suggestions
auto searchEngine = std::make_shared<WebSearchEngine>();
searchEngine->setApiKey("YOUR_API_KEY");
auto results = searchEngine->searchCode("binary search implementation");
```

## 🔧 Configuration

Create a `taskbot.config` file:

```ini
# TaskBot Configuration
bot_name = MyTaskBot
max_workers = 4
enable_monitoring = true
check_interval = 60.0
log_level = DEBUG
log_file = taskbot.log
```

## 🤖 AutoPilot Workflows

TaskBot comes with pre-defined workflows:

- **open_browser**: Launch web browser
- **take_screenshot**: Capture current screen
- **switch_windows**: Alt+Tab between windows
- **copy_paste_between_apps**: Copy from one app and paste to another
- **fill_web_form**: Automated form filling

### Creating Custom Workflows

```cpp
autoPilot->registerWorkflow("my_workflow", [&autoPilot]() {
    std::cout << "Executing custom workflow" << std::endl;
    
    // Your automation logic here
    autoPilot->getInputSimulator().typeText("Custom automation");
    
    return true;
});
```

## ⚠️ Security Considerations

**WARNING**: TaskBot AutoPilot has complete control over your system. Use with caution!

- Never run TaskBot with elevated privileges unless absolutely necessary
- Be careful when recording macros - they may contain sensitive information
- Review automation scripts before execution
- Use in isolated environments when testing

## 🛡️ Safety Features

- Signal handling for graceful shutdown (Ctrl+C)
- Timeout mechanisms for all operations
- Comprehensive error logging
- Resource usage monitoring
- Task status tracking

## 📚 Examples

### File Automation
```cpp
auto fileTask = std::make_shared<FileTask>(
    "Backup Config",
    FileTask::Operation::COPY,
    "/etc/config.conf",
    "",
    "/backup/config.conf"
);
manager.addTask(fileTask);
```

### System Monitoring
```cpp
auto monitorTask = std::make_shared<MonitorTask>(
    "High CPU Alert",
    [](const SystemMonitor::SystemInfo& info) {
        return info.cpuUsage > 80.0;
    },
    [](const SystemMonitor::SystemInfo& info) {
        Logger::getInstance().critical("CPU usage critical: " + 
            std::to_string(info.cpuUsage) + "%");
    }
);
```

### Recurring Tasks
```cpp
auto backupTask = std::make_shared<RecurringTask>(
    "Hourly Backup",
    fileTask,
    std::chrono::hours(1),
    24  // Run 24 times
);
```

## 🐛 Troubleshooting

### Common Issues

1. **X11 Display Error**
   ```bash
   export DISPLAY=:0
   xhost +local:
   ```

2. **Permission Denied**
   - Ensure you have permissions for `/dev/uinput`
   - Add user to `input` group: `sudo usermod -a -G input $USER`

3. **Missing Dependencies**
   - Run `./build.sh` - it will check and prompt to install missing deps

## 🤝 Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## 📄 License

This project is licensed under the MIT License - see LICENSE file for details.

## ⚡ Performance Tips

- Use appropriate task priorities for time-sensitive operations
- Batch file operations when possible
- Configure worker thread count based on CPU cores
- Use scheduled tasks instead of polling loops
- Enable debug logging only when needed

## 🔮 Future Features

- [ ] Windows and macOS support
- [ ] Python bindings
- [ ] Web UI for remote control
- [ ] Cloud integration
- [ ] Machine learning for pattern recognition
- [ ] Voice control integration
- [ ] Mobile app companion

## 📞 Support

- Issue Tracker: [GitHub Issues](https://github.com/yourusername/TaskBot/issues)
- Documentation: [Wiki](https://github.com/yourusername/TaskBot/wiki)
- Email: support@taskbot.dev

---

**Remember**: With great automation comes great responsibility. Use TaskBot wisely!