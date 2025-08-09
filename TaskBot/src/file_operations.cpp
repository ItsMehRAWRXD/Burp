#include "../include/task_bot.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace TaskBot {

bool FileOperations::createFile(const std::string& path, const std::string& content) {
    try {
        std::ofstream file(path);
        if (!file.is_open()) {
            Logger::getInstance().error("Failed to create file: " + path);
            return false;
        }
        
        file << content;
        file.close();
        
        Logger::getInstance().info("Created file: " + path);
        return true;
    } catch (const std::exception& e) {
        Logger::getInstance().error("Exception creating file " + path + ": " + e.what());
        return false;
    }
}

bool FileOperations::readFile(const std::string& path, std::string& content) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            Logger::getInstance().error("Failed to open file for reading: " + path);
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
        file.close();
        
        Logger::getInstance().debug("Read file: " + path + " (" + 
                                   std::to_string(content.size()) + " bytes)");
        return true;
    } catch (const std::exception& e) {
        Logger::getInstance().error("Exception reading file " + path + ": " + e.what());
        return false;
    }
}

bool FileOperations::updateFile(const std::string& path, const std::string& content) {
    try {
        if (!fileExists(path)) {
            Logger::getInstance().warning("File doesn't exist for update, creating new: " + path);
        }
        
        return createFile(path, content);
    } catch (const std::exception& e) {
        Logger::getInstance().error("Exception updating file " + path + ": " + e.what());
        return false;
    }
}

bool FileOperations::deleteFile(const std::string& path) {
    try {
        if (!std::filesystem::exists(path)) {
            Logger::getInstance().warning("File doesn't exist for deletion: " + path);
            return false;
        }
        
        bool result = std::filesystem::remove(path);
        if (result) {
            Logger::getInstance().info("Deleted file: " + path);
        } else {
            Logger::getInstance().error("Failed to delete file: " + path);
        }
        return result;
    } catch (const std::exception& e) {
        Logger::getInstance().error("Exception deleting file " + path + ": " + e.what());
        return false;
    }
}

bool FileOperations::fileExists(const std::string& path) {
    return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
}

std::vector<std::string> FileOperations::listDirectory(const std::string& path) {
    std::vector<std::string> files;
    
    try {
        if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
            Logger::getInstance().error("Invalid directory path: " + path);
            return files;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            files.push_back(entry.path().filename().string());
        }
        
        std::sort(files.begin(), files.end());
        Logger::getInstance().debug("Listed " + std::to_string(files.size()) + 
                                   " items in directory: " + path);
    } catch (const std::exception& e) {
        Logger::getInstance().error("Exception listing directory " + path + ": " + e.what());
    }
    
    return files;
}

bool FileOperations::createDirectory(const std::string& path) {
    try {
        if (std::filesystem::exists(path)) {
            Logger::getInstance().warning("Directory already exists: " + path);
            return true;
        }
        
        bool result = std::filesystem::create_directories(path);
        if (result) {
            Logger::getInstance().info("Created directory: " + path);
        } else {
            Logger::getInstance().error("Failed to create directory: " + path);
        }
        return result;
    } catch (const std::exception& e) {
        Logger::getInstance().error("Exception creating directory " + path + ": " + e.what());
        return false;
    }
}

bool FileOperations::copyFile(const std::string& source, const std::string& dest) {
    try {
        if (!fileExists(source)) {
            Logger::getInstance().error("Source file doesn't exist: " + source);
            return false;
        }
        
        std::filesystem::copy_file(source, dest, 
                                  std::filesystem::copy_options::overwrite_existing);
        
        Logger::getInstance().info("Copied file from " + source + " to " + dest);
        return true;
    } catch (const std::exception& e) {
        Logger::getInstance().error("Exception copying file from " + source + 
                                   " to " + dest + ": " + e.what());
        return false;
    }
}

bool FileOperations::moveFile(const std::string& source, const std::string& dest) {
    try {
        if (!fileExists(source)) {
            Logger::getInstance().error("Source file doesn't exist: " + source);
            return false;
        }
        
        std::filesystem::rename(source, dest);
        Logger::getInstance().info("Moved file from " + source + " to " + dest);
        return true;
    } catch (const std::exception& e) {
        Logger::getInstance().error("Exception moving file from " + source + 
                                   " to " + dest + ": " + e.what());
        return false;
    }
}

size_t FileOperations::getFileSize(const std::string& path) {
    try {
        if (!fileExists(path)) {
            Logger::getInstance().error("File doesn't exist: " + path);
            return 0;
        }
        
        return std::filesystem::file_size(path);
    } catch (const std::exception& e) {
        Logger::getInstance().error("Exception getting file size for " + path + ": " + e.what());
        return 0;
    }
}

// FileTask implementation
FileTask::FileTask(const std::string& name, Operation op, const std::string& path,
                   const std::string& content, const std::string& destPath)
    : Task(name, Priority::NORMAL), operation_(op), path_(path), 
      content_(content), destPath_(destPath) {}

bool FileTask::execute() {
    switch (operation_) {
        case Operation::CREATE:
            return FileOperations::createFile(path_, content_);
            
        case Operation::READ: {
            std::string readContent;
            bool success = FileOperations::readFile(path_, readContent);
            if (success) {
                Logger::getInstance().info("Read " + std::to_string(readContent.size()) + 
                                         " bytes from " + path_);
            }
            return success;
        }
        
        case Operation::UPDATE:
            return FileOperations::updateFile(path_, content_);
            
        case Operation::DELETE:
            return FileOperations::deleteFile(path_);
            
        case Operation::COPY:
            return FileOperations::copyFile(path_, destPath_);
            
        case Operation::MOVE:
            return FileOperations::moveFile(path_, destPath_);
            
        default:
            Logger::getInstance().error("Unknown file operation");
            return false;
    }
}

std::string FileTask::getDescription() const {
    std::string desc = "File operation: ";
    
    switch (operation_) {
        case Operation::CREATE: desc += "CREATE " + path_; break;
        case Operation::READ: desc += "READ " + path_; break;
        case Operation::UPDATE: desc += "UPDATE " + path_; break;
        case Operation::DELETE: desc += "DELETE " + path_; break;
        case Operation::COPY: desc += "COPY " + path_ + " to " + destPath_; break;
        case Operation::MOVE: desc += "MOVE " + path_ + " to " + destPath_; break;
    }
    
    return desc;
}

} // namespace TaskBot