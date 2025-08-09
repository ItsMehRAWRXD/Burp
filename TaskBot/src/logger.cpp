#include "../include/task_bot.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

namespace TaskBot {

Logger::Logger() : currentLevel_(LogLevel::INFO), consoleOutput_(true) {}

Logger::~Logger() {
    // Ensure any pending logs are written
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(logMutex_);
    logFile_ = filename;
}

void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex_);
    currentLevel_ = level;
}

void Logger::enableConsoleOutput(bool enable) {
    std::lock_guard<std::mutex> lock(logMutex_);
    consoleOutput_ = enable;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (static_cast<int>(level) < static_cast<int>(currentLevel_)) {
        return; // Skip messages below current log level
    }
    
    std::lock_guard<std::mutex> lock(logMutex_);
    
    // Format timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    // Format log level
    std::string levelStr;
    switch (level) {
        case LogLevel::DEBUG:    levelStr = "DEBUG"; break;
        case LogLevel::INFO:     levelStr = "INFO "; break;
        case LogLevel::WARNING:  levelStr = "WARN "; break;
        case LogLevel::ERROR:    levelStr = "ERROR"; break;
        case LogLevel::CRITICAL: levelStr = "CRIT "; break;
    }
    
    // Build log message
    std::string logLine = ss.str() + " [" + levelStr + "] " + message;
    
    // Write to console if enabled
    if (consoleOutput_) {
        if (level >= LogLevel::ERROR) {
            std::cerr << logLine << std::endl;
        } else {
            std::cout << logLine << std::endl;
        }
    }
    
    // Write to file if configured
    if (!logFile_.empty()) {
        std::ofstream file(logFile_, std::ios::app);
        if (file.is_open()) {
            file << logLine << std::endl;
        }
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::CRITICAL, message);
}

} // namespace TaskBot