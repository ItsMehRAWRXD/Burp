#include "SimpleBot.h"
#include <iostream>
#include <fstream>
#include <sstream>

void SimpleBot::start() {
    running = true;
    worker = std::thread(&SimpleBot::process_tasks, this);
}

void SimpleBot::stop() {
    running = false;
    if (worker.joinable()) worker.join();
}

void SimpleBot::add_task(GenType type, Lang lang, const std::string& name, int priority) {
    std::lock_guard<std::mutex> lock(task_mutex);
    tasks.push({type, lang, name, priority});
}

std::string SimpleBot::generate_now(GenType type, Lang lang, const std::string& name) {
    return generate(type, lang, name);
}

void SimpleBot::process_tasks() {
    while (running) {
        Task task;
        {
            std::lock_guard<std::mutex> lock(task_mutex);
            if (!tasks.empty()) {
                task = tasks.top();
                tasks.pop();
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
        }
        
        std::string code = generate(task.type, task.lang, task.name);
        std::string filename = task.name + extensions[task.lang];
        
        std::ofstream file(filename);
        file << code;
        file.close();
        
        std::cout << "Generated: " << filename << std::endl;
    }
}

std::string SimpleBot::generate(GenType type, Lang lang, const std::string& name) {
    std::stringstream ss;
    
    switch (lang) {
        case Lang::CPP:
            if (type == GenType::CLASS) {
                ss << "#include <iostream>\n\nclass " << name << " {\npublic:\n    " << name << "() {}\n    void process() { std::cout << \"Processing...\\n\"; }\n};\n";
            } else if (type == GenType::FUNC) {
                ss << "#include <iostream>\n\nvoid " << name << "() {\n    std::cout << \"Function " << name << " executed\\n\";\n}\n";
            } else {
                ss << "#include <iostream>\n\nint main() {\n    std::cout << \"Hello from " << name << "\\n\";\n    return 0;\n}\n";
            }
            break;
            
        case Lang::PY:
            if (type == GenType::CLASS) {
                ss << "class " << name << ":\n    def __init__(self):\n        pass\n    \n    def process(self):\n        print('Processing...')\n";
            } else if (type == GenType::FUNC) {
                ss << "def " << name << "():\n    print('Function " << name << " executed')\n";
            } else {
                ss << "print('Hello from " << name << "')\n";
            }
            break;
            
        case Lang::JS:
            if (type == GenType::CLASS) {
                ss << "class " << name << " {\n    constructor() {}\n    process() { console.log('Processing...'); }\n}\n";
            } else if (type == GenType::FUNC) {
                ss << "function " << name << "() {\n    console.log('Function " << name << " executed');\n}\n";
            } else {
                ss << "console.log('Hello from " << name << "');\n";
            }
            break;
            
        case Lang::RUST:
            if (type == GenType::CLASS) {
                ss << "struct " << name << " {}\n\nimpl " << name << " {\n    fn new() -> Self { " << name << " {} }\n    fn process(&self) { println!(\"Processing...\"); }\n}\n";
            } else if (type == GenType::FUNC) {
                ss << "fn " << name << "() {\n    println!(\"Function " << name << " executed\");\n}\n";
            } else {
                ss << "fn main() {\n    println!(\"Hello from " << name << "\");\n}\n";
            }
            break;
            
        case Lang::GO:
            if (type == GenType::CLASS) {
                ss << "package main\n\nimport \"fmt\"\n\ntype " << name << " struct {}\n\nfunc (c *" << name << ") Process() {\n    fmt.Println(\"Processing...\")\n}\n";
            } else if (type == GenType::FUNC) {
                ss << "package main\n\nimport \"fmt\"\n\nfunc " << name << "() {\n    fmt.Println(\"Function " << name << " executed\")\n}\n";
            } else {
                ss << "package main\n\nimport \"fmt\"\n\nfunc main() {\n    fmt.Println(\"Hello from " << name << "\")\n}\n";
            }
            break;
            
        case Lang::ASM_X64:
            if (type == GenType::FUNC) {
                ss << ".section .text\n.global " << name << "\n" << name << ":\n    push %rbp\n    mov %rsp, %rbp\n    mov $0, %rax\n    pop %rbp\n    ret\n";
            } else {
                ss << ".section .data\n    msg: .ascii \"Hello from " << name << "\\n\"\n    msg_len = . - msg\n\n.section .text\n.global _start\n\n_start:\n    mov $1, %rax\n    mov $1, %rdi\n    mov $msg, %rsi\n    mov $msg_len, %rdx\n    syscall\n    mov $60, %rax\n    mov $0, %rdi\n    syscall\n";
            }
            break;
    }
    
    return ss.str();
}