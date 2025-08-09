#include "../include/task_bot.h"
#include <algorithm>
#include <iostream>

namespace TaskBot {

Task::Task(const std::string& name, Priority priority)
    : name_(name), priority_(priority), status_(TaskStatus::PENDING) {}

TaskManager::TaskManager() : running_(false), numWorkers_(std::thread::hardware_concurrency()) {
    if (numWorkers_ == 0) numWorkers_ = 4; // Default to 4 workers if detection fails
    
    // Priority comparator for the priority queue
    auto comparator = [](const std::shared_ptr<Task>& a, const std::shared_ptr<Task>& b) {
        return static_cast<int>(a->getPriority()) < static_cast<int>(b->getPriority());
    };
    
    taskQueue_ = std::priority_queue<std::shared_ptr<Task>, 
                                    std::vector<std::shared_ptr<Task>>,
                                    decltype(comparator)>(comparator);
}

TaskManager::~TaskManager() {
    stop();
}

void TaskManager::addTask(std::shared_ptr<Task> task) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    taskQueue_.push(task);
    cv_.notify_one();
    
    Logger::getInstance().info("Added task: " + task->getName() + 
                              " with priority: " + std::to_string(static_cast<int>(task->getPriority())));
}

void TaskManager::start() {
    if (running_) return;
    
    running_ = true;
    workers_.clear();
    
    for (size_t i = 0; i < numWorkers_; ++i) {
        workers_.emplace_back(&TaskManager::workerThread, this);
    }
    
    Logger::getInstance().info("TaskManager started with " + std::to_string(numWorkers_) + " workers");
}

void TaskManager::stop() {
    if (!running_) return;
    
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        running_ = false;
    }
    cv_.notify_all();
    
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
    Logger::getInstance().info("TaskManager stopped");
}

void TaskManager::executeOnce() {
    auto task = getNextTask();
    if (task) {
        task->setStatus(TaskStatus::RUNNING);
        Logger::getInstance().info("Executing task: " + task->getName());
        
        try {
            bool success = task->execute();
            task->setStatus(success ? TaskStatus::COMPLETED : TaskStatus::FAILED);
            
            if (success) {
                Logger::getInstance().info("Task completed successfully: " + task->getName());
            } else {
                Logger::getInstance().error("Task failed: " + task->getName());
            }
        } catch (const std::exception& e) {
            task->setStatus(TaskStatus::FAILED);
            Logger::getInstance().error("Task threw exception: " + task->getName() + 
                                      " - " + std::string(e.what()));
        }
        
        std::lock_guard<std::mutex> lock(queueMutex_);
        completedTasks_.push_back(task);
    }
}

void TaskManager::workerThread() {
    while (running_) {
        auto task = getNextTask();
        
        if (!task) {
            std::unique_lock<std::mutex> lock(queueMutex_);
            cv_.wait_for(lock, std::chrono::milliseconds(100), 
                        [this] { return !running_ || !taskQueue_.empty(); });
            continue;
        }
        
        task->setStatus(TaskStatus::RUNNING);
        Logger::getInstance().info("Worker executing task: " + task->getName());
        
        try {
            bool success = task->execute();
            task->setStatus(success ? TaskStatus::COMPLETED : TaskStatus::FAILED);
            
            if (success) {
                Logger::getInstance().info("Task completed: " + task->getName());
            } else {
                Logger::getInstance().error("Task failed: " + task->getName());
            }
        } catch (const std::exception& e) {
            task->setStatus(TaskStatus::FAILED);
            Logger::getInstance().error("Task exception: " + task->getName() + 
                                      " - " + std::string(e.what()));
        }
        
        std::lock_guard<std::mutex> lock(queueMutex_);
        completedTasks_.push_back(task);
    }
}

std::shared_ptr<Task> TaskManager::getNextTask() {
    std::lock_guard<std::mutex> lock(queueMutex_);
    
    if (taskQueue_.empty()) {
        return nullptr;
    }
    
    auto task = taskQueue_.top();
    taskQueue_.pop();
    return task;
}

std::vector<std::shared_ptr<Task>> TaskManager::getPendingTasks() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    std::vector<std::shared_ptr<Task>> pending;
    
    // Copy queue to get pending tasks
    auto tempQueue = taskQueue_;
    while (!tempQueue.empty()) {
        pending.push_back(tempQueue.top());
        tempQueue.pop();
    }
    
    return pending;
}

std::vector<std::shared_ptr<Task>> TaskManager::getCompletedTasks() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return completedTasks_;
}

} // namespace TaskBot