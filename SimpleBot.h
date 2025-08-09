#pragma once
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>

enum class Lang { CPP, PY, JS, RUST, GO, ASM_X64 };
enum class GenType { CLASS, FUNC, PROG };

struct Task {
    GenType type;
    Lang lang;
    std::string name;
    int priority;
    
    bool operator<(const Task& other) const {
        return priority < other.priority;
    }
};

class SimpleBot {
private:
    std::priority_queue<Task> tasks;
    std::mutex task_mutex;
    std::atomic<bool> running{false};
    std::thread worker;
    
    std::map<Lang, std::string> extensions = {
        {Lang::CPP, ".cpp"}, {Lang::PY, ".py"}, {Lang::JS, ".js"},
        {Lang::RUST, ".rs"}, {Lang::GO, ".go"}, {Lang::ASM_X64, ".s"}
    };
    
    std::string generate(GenType type, Lang lang, const std::string& name);
    void process_tasks();
    
public:
    void start();
    void stop();
    void add_task(GenType type, Lang lang, const std::string& name, int priority = 1);
    std::string generate_now(GenType type, Lang lang, const std::string& name);
};