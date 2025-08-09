#include "../include/task_bot.h"
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <array>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>
#include <cstring>

namespace TaskBot {

CommandExecutor::CommandResult CommandExecutor::execute(const std::string& command) {
    CommandResult result;
    result.exitCode = -1;
    result.success = false;
    
    Logger::getInstance().info("Executing command: " + command);
    
    // Use popen for simple command execution
    std::array<char, 4096> buffer;
    std::string fullCommand = command + " 2>&1"; // Redirect stderr to stdout
    
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(fullCommand.c_str(), "r"), pclose);
    
    if (!pipe) {
        result.error = "Failed to execute command";
        Logger::getInstance().error("Failed to execute command: " + command);
        return result;
    }
    
    // Read output
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result.output += buffer.data();
    }
    
    // Get exit code
    int status = pclose(pipe.release());
    if (WIFEXITED(status)) {
        result.exitCode = WEXITSTATUS(status);
        result.success = (result.exitCode == 0);
    }
    
    Logger::getInstance().debug("Command completed with exit code: " + 
                               std::to_string(result.exitCode));
    
    return result;
}

CommandExecutor::CommandResult CommandExecutor::executeWithTimeout(const std::string& command, 
                                                                  int timeoutSeconds) {
    CommandResult result;
    result.exitCode = -1;
    result.success = false;
    
    Logger::getInstance().info("Executing command with timeout " + 
                              std::to_string(timeoutSeconds) + "s: " + command);
    
    int pipefd[2];
    int errfd[2];
    
    if (pipe(pipefd) == -1 || pipe(errfd) == -1) {
        result.error = "Failed to create pipes";
        return result;
    }
    
    pid_t pid = fork();
    
    if (pid == -1) {
        result.error = "Failed to fork process";
        close(pipefd[0]); close(pipefd[1]);
        close(errfd[0]); close(errfd[1]);
        return result;
    }
    
    if (pid == 0) {
        // Child process
        close(pipefd[0]); // Close read end
        close(errfd[0]);
        
        // Redirect stdout and stderr
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(errfd[1], STDERR_FILENO);
        
        close(pipefd[1]);
        close(errfd[1]);
        
        // Execute command
        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        _exit(127); // If exec fails
    }
    
    // Parent process
    close(pipefd[1]); // Close write end
    close(errfd[1]);
    
    // Set non-blocking mode
    fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
    fcntl(errfd[0], F_SETFL, O_NONBLOCK);
    
    // Read output with timeout
    auto startTime = std::chrono::steady_clock::now();
    bool timedOut = false;
    
    while (true) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - startTime).count();
            
        if (elapsed >= timeoutSeconds) {
            timedOut = true;
            kill(pid, SIGTERM);
            usleep(100000); // Give it 100ms to terminate
            kill(pid, SIGKILL); // Force kill if still running
            break;
        }
        
        // Check if process has exited
        int status;
        pid_t wpid = waitpid(pid, &status, WNOHANG);
        
        if (wpid == pid) {
            // Process has exited
            if (WIFEXITED(status)) {
                result.exitCode = WEXITSTATUS(status);
                result.success = (result.exitCode == 0);
            }
            break;
        }
        
        // Read available output
        char buffer[4096];
        ssize_t bytesRead;
        
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            result.output += buffer;
        }
        
        while ((bytesRead = read(errfd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            result.error += buffer;
        }
        
        usleep(10000); // Sleep 10ms
    }
    
    // Read any remaining output
    char buffer[4096];
    ssize_t bytesRead;
    
    while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        result.output += buffer;
    }
    
    while ((bytesRead = read(errfd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        result.error += buffer;
    }
    
    close(pipefd[0]);
    close(errfd[0]);
    
    if (timedOut) {
        result.error = "Command timed out after " + std::to_string(timeoutSeconds) + " seconds";
        result.success = false;
        Logger::getInstance().error("Command timed out: " + command);
    }
    
    return result;
}

bool CommandExecutor::executeAsync(const std::string& command) {
    Logger::getInstance().info("Executing async command: " + command);
    
    pid_t pid = fork();
    
    if (pid == -1) {
        Logger::getInstance().error("Failed to fork for async command: " + command);
        return false;
    }
    
    if (pid == 0) {
        // Child process
        // Close all file descriptors
        for (int i = 3; i < 1024; i++) {
            close(i);
        }
        
        // Redirect stdout and stderr to /dev/null
        int devnull = open("/dev/null", O_RDWR);
        dup2(devnull, STDOUT_FILENO);
        dup2(devnull, STDERR_FILENO);
        close(devnull);
        
        // Execute command
        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        _exit(127); // If exec fails
    }
    
    // Parent process - don't wait for child
    Logger::getInstance().info("Async command started with PID: " + std::to_string(pid));
    return true;
}

std::vector<std::string> CommandExecutor::getEnvironmentVariables() {
    std::vector<std::string> envVars;
    
    extern char** environ;
    for (char** env = environ; *env != nullptr; env++) {
        envVars.push_back(std::string(*env));
    }
    
    return envVars;
}

bool CommandExecutor::setEnvironmentVariable(const std::string& name, const std::string& value) {
    int result = setenv(name.c_str(), value.c_str(), 1);
    
    if (result == 0) {
        Logger::getInstance().info("Set environment variable: " + name + "=" + value);
        return true;
    } else {
        Logger::getInstance().error("Failed to set environment variable: " + name);
        return false;
    }
}

// CommandTask implementation
CommandTask::CommandTask(const std::string& name, const std::string& command, int timeoutSeconds)
    : Task(name, Priority::NORMAL), command_(command), timeoutSeconds_(timeoutSeconds) {}

bool CommandTask::execute() {
    if (timeoutSeconds_ > 0) {
        result_ = CommandExecutor::executeWithTimeout(command_, timeoutSeconds_);
    } else {
        result_ = CommandExecutor::execute(command_);
    }
    
    if (result_.success) {
        Logger::getInstance().info("Command task completed: " + getName());
    } else {
        Logger::getInstance().error("Command task failed: " + getName() + 
                                  " - Exit code: " + std::to_string(result_.exitCode));
        if (!result_.error.empty()) {
            Logger::getInstance().error("Error output: " + result_.error);
        }
    }
    
    return result_.success;
}

std::string CommandTask::getDescription() const {
    std::string desc = "Execute command: " + command_;
    if (timeoutSeconds_ > 0) {
        desc += " (timeout: " + std::to_string(timeoutSeconds_) + "s)";
    }
    return desc;
}

} // namespace TaskBot