#include "../include/task_bot.h"
#include <iostream>

namespace TaskBot {

// ScheduledTask implementation
ScheduledTask::ScheduledTask(const std::string& name, std::shared_ptr<Task> task,
                           std::chrono::system_clock::time_point executeTime)
    : Task(name, Priority::NORMAL), task_(task), executeTime_(executeTime) {}

bool ScheduledTask::execute() {
    if (!isReady()) {
        // Not time yet, re-queue the task
        return true;
    }
    
    Logger::getInstance().info("Executing scheduled task: " + getName());
    
    if (!task_) {
        Logger::getInstance().error("Scheduled task has null task pointer: " + getName());
        return false;
    }
    
    // Execute the underlying task
    bool result = task_->execute();
    
    if (result) {
        Logger::getInstance().info("Scheduled task completed: " + getName());
    } else {
        Logger::getInstance().error("Scheduled task failed: " + getName());
    }
    
    return result;
}

std::string ScheduledTask::getDescription() const {
    auto timeT = std::chrono::system_clock::to_time_t(executeTime_);
    std::stringstream ss;
    ss << "Scheduled task: " << getName() << " at " 
       << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");
    
    if (task_) {
        ss << " - " << task_->getDescription();
    }
    
    return ss.str();
}

bool ScheduledTask::isReady() const {
    return std::chrono::system_clock::now() >= executeTime_;
}

// RecurringTask implementation
RecurringTask::RecurringTask(const std::string& name, std::shared_ptr<Task> task,
                           std::chrono::seconds interval, int maxRuns)
    : Task(name, Priority::NORMAL), task_(task), interval_(interval), 
      maxRuns_(maxRuns), runCount_(0) {
    lastRun_ = std::chrono::system_clock::now() - interval_; // Allow immediate first run
}

bool RecurringTask::execute() {
    // Check if we've reached max runs
    if (maxRuns_ > 0 && runCount_ >= maxRuns_) {
        Logger::getInstance().info("Recurring task reached max runs: " + getName());
        return true;
    }
    
    // Check if enough time has passed since last run
    auto now = std::chrono::system_clock::now();
    auto timeSinceLastRun = std::chrono::duration_cast<std::chrono::seconds>(now - lastRun_);
    
    if (timeSinceLastRun < interval_) {
        // Not time yet, task will be re-queued
        return true;
    }
    
    Logger::getInstance().info("Executing recurring task: " + getName() + 
                              " (run " + std::to_string(runCount_ + 1) + ")");
    
    if (!task_) {
        Logger::getInstance().error("Recurring task has null task pointer: " + getName());
        return false;
    }
    
    // Execute the underlying task
    bool result = task_->execute();
    
    if (result) {
        runCount_++;
        lastRun_ = now;
        Logger::getInstance().info("Recurring task completed: " + getName());
        
        // Re-queue if we haven't reached max runs
        if (maxRuns_ < 0 || runCount_ < maxRuns_) {
            // Task manager should re-add this task
            setStatus(TaskStatus::PENDING);
        }
    } else {
        Logger::getInstance().error("Recurring task failed: " + getName());
    }
    
    return result;
}

std::string RecurringTask::getDescription() const {
    std::stringstream ss;
    ss << "Recurring task: " << getName() 
       << " every " << interval_.count() << " seconds";
    
    if (maxRuns_ > 0) {
        ss << " (max runs: " << maxRuns_ << ", current: " << runCount_ << ")";
    }
    
    if (task_) {
        ss << " - " << task_->getDescription();
    }
    
    return ss.str();
}

} // namespace TaskBot