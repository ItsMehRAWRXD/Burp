#include "ReverseBot.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <command> <target>\n";
        std::cout << "Commands:\n";
        std::cout << "  disasm <binary>    - Disassemble binary\n";
        std::cout << "  strings <binary>   - Extract strings\n";
        std::cout << "  symbols <binary>   - Show symbols\n";
        std::cout << "  headers <binary>   - Show ELF headers\n";
        std::cout << "  flows <binary>     - Show control flows\n";
        std::cout << "  exploit <type>     - Generate exploit template\n";
        std::cout << "  shellcode <arch>   - Generate shellcode\n";
        std::cout << "  patch <binary> <addr> <bytes> - Patch binary\n";
        std::cout << "  learn <concept>    - Learn reverse engineering concept\n";
        return 1;
    }
    
    ReverseBot bot;
    std::string cmd = argv[1];
    std::string target = argv[2];
    
    if (cmd == "disasm") {
        bot.analyze(target, AnalysisType::DISASM);
    }
    else if (cmd == "strings") {
        bot.analyze(target, AnalysisType::STRINGS);
    }
    else if (cmd == "symbols") {
        bot.analyze(target, AnalysisType::SYMBOLS);
    }
    else if (cmd == "headers") {
        bot.analyze(target, AnalysisType::HEADERS);
    }
    else if (cmd == "flows") {
        bot.analyze(target, AnalysisType::CONTROL_FLOW);
    }
    else if (cmd == "exploit") {
        bot.create_exploit(target);
    }
    else if (cmd == "shellcode") {
        Arch arch = (target == "x64") ? Arch::X64 : Arch::X86;
        bot.generate_shellcode(arch, "execve");
    }
    else if (cmd == "learn") {
        bot.teach_concept(target);
    }
    else if (cmd == "patch" && argc >= 5) {
        uint64_t addr = std::stoull(argv[3], nullptr, 16);
        std::vector<uint8_t> bytes;
        std::string byte_str = argv[4];
        for (size_t i = 0; i < byte_str.length(); i += 2) {
            bytes.push_back(std::stoi(byte_str.substr(i, 2), nullptr, 16));
        }
        bot.patch_binary(addr, bytes);
    }
    
    return 0;
}