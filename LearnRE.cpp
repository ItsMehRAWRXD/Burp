#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cstdint>

class LearnRE {
private:
    std::vector<uint8_t> data;
    
public:
    void load_binary(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            std::cout << "Can't open file\n";
            return;
        }
        
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0);
        
        data.resize(size);
        file.read(reinterpret_cast<char*>(data.data()), size);
        
        std::cout << "Loaded " << size << " bytes\n";
        show_hex_dump(0, 64);
    }
    
    void show_hex_dump(size_t start, size_t count) {
        std::cout << "\nHex dump:\n";
        for (size_t i = start; i < start + count && i < data.size(); i += 16) {
            std::cout << std::hex << std::setw(8) << std::setfill('0') << i << ": ";
            
            for (size_t j = 0; j < 16 && i + j < data.size(); j++) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') 
                         << static_cast<int>(data[i + j]) << " ";
            }
            std::cout << "\n";
        }
        std::cout << std::dec;
    }
    
    void find_strings() {
        std::cout << "\nSearching for strings:\n";
        std::string current;
        
        for (size_t i = 0; i < data.size(); i++) {
            if (data[i] >= 32 && data[i] <= 126) {
                current += static_cast<char>(data[i]);
            } else {
                if (current.length() > 4) {
                    std::cout << "0x" << std::hex << (i - current.length()) 
                             << std::dec << ": " << current << "\n";
                }
                current.clear();
            }
        }
    }
    
    void check_elf_header() {
        if (data.size() < 64) {
            std::cout << "File too small for ELF\n";
            return;
        }
        
        if (data[0] == 0x7f && data[1] == 'E' && data[2] == 'L' && data[3] == 'F') {
            std::cout << "\nThis is an ELF file\n";
            std::cout << "Architecture: " << (data[4] == 1 ? "32-bit" : "64-bit") << "\n";
            std::cout << "Endian: " << (data[5] == 1 ? "Little" : "Big") << "\n";
            
            uint64_t entry = 0;
            if (data[4] == 2) { // 64-bit
                for (int i = 0; i < 8; i++) {
                    entry |= static_cast<uint64_t>(data[24 + i]) << (i * 8);
                }
            }
            std::cout << "Entry point: 0x" << std::hex << entry << std::dec << "\n";
        } else {
            std::cout << "\nNot an ELF file\n";
            std::cout << "First 4 bytes: ";
            for (int i = 0; i < 4; i++) {
                std::cout << std::hex << static_cast<int>(data[i]) << " ";
            }
            std::cout << std::dec << "\n";
        }
    }
    
    void simple_disasm(size_t offset, size_t count) {
        std::cout << "\nSimple x64 disassembly at 0x" << std::hex << offset << ":\n";
        
        for (size_t i = offset; i < offset + count && i < data.size(); i++) {
            uint8_t byte = data[i];
            std::cout << std::hex << std::setw(8) << std::setfill('0') << i << ": "
                     << std::setw(2) << static_cast<int>(byte) << " ";
            
            // Very basic x64 instruction recognition
            switch (byte) {
                case 0x48: std::cout << "REX.W prefix"; break;
                case 0x50: case 0x51: case 0x52: case 0x53:
                case 0x54: case 0x55: case 0x56: case 0x57:
                    std::cout << "push r" << (byte - 0x50); break;
                case 0x58: case 0x59: case 0x5a: case 0x5b:
                case 0x5c: case 0x5d: case 0x5e: case 0x5f:
                    std::cout << "pop r" << (byte - 0x58); break;
                case 0xc3: std::cout << "ret"; break;
                case 0x90: std::cout << "nop"; break;
                case 0xcc: std::cout << "int3 (breakpoint)"; break;
                default: std::cout << "unknown"; break;
            }
            std::cout << "\n";
        }
        std::cout << std::dec;
    }
    
    void create_vulnerable_program() {
        std::ofstream vuln("vuln.c");
        vuln << "#include <stdio.h>\n";
        vuln << "#include <string.h>\n\n";
        vuln << "void vulnerable_function() {\n";
        vuln << "    char buffer[64];\n";
        vuln << "    printf(\"Enter data: \");\n";
        vuln << "    gets(buffer);  // VULNERABLE!\n";
        vuln << "    printf(\"You entered: %s\\n\", buffer);\n";
        vuln << "}\n\n";
        vuln << "int main() {\n";
        vuln << "    vulnerable_function();\n";
        vuln << "    return 0;\n";
        vuln << "}\n";
        vuln.close();
        
        std::cout << "\nCreated vuln.c - a program with buffer overflow\n";
        std::cout << "Compile with: gcc -fno-stack-protector -z execstack vuln.c -o vuln\n";
        std::cout << "Test with: python -c \"print('A' * 100)\" | ./vuln\n";
    }
    
    void teach_stack_layout() {
        std::cout << "\nStack Layout (grows downward):\n";
        std::cout << "High Address\n";
        std::cout << "+-----------------+\n";
        std::cout << "| Return Address  | <- We want to overwrite this\n";
        std::cout << "+-----------------+\n";
        std::cout << "| Saved Frame Ptr |\n";
        std::cout << "+-----------------+\n";
        std::cout << "| Local Variables |\n";
        std::cout << "| (our buffer)    | <- Overflow starts here\n";
        std::cout << "+-----------------+\n";
        std::cout << "Low Address\n\n";
        std::cout << "Buffer overflow overwrites return address\n";
        std::cout << "Control execution by pointing to shellcode\n";
    }
    
    void generate_simple_shellcode() {
        std::cout << "\nSimple x64 shellcode (execve /bin/sh):\n";
        std::cout << "\\x48\\x31\\xf6\\x56\\x48\\xbf\\x2f\\x62\\x69\\x6e\\x2f\\x2f\\x73\\x68\\x57\\x54\\x5f\\x6a\\x3b\\x58\\x99\\x0f\\x05\n\n";
        
        std::ofstream sc("shellcode.bin", std::ios::binary);
        uint8_t shellcode[] = {
            0x48, 0x31, 0xf6,           // xor rsi, rsi
            0x56,                       // push rsi
            0x48, 0xbf, 0x2f, 0x62,     // mov rdi, '/bin'
            0x69, 0x6e, 0x2f, 0x2f,     // '/sh'
            0x73, 0x68,
            0x57,                       // push rdi
            0x54,                       // push rsp
            0x5f,                       // pop rdi
            0x6a, 0x3b,                 // push 59 (execve)
            0x58,                       // pop rax
            0x99,                       // cdq
            0x0f, 0x05                  // syscall
        };
        
        sc.write(reinterpret_cast<char*>(shellcode), sizeof(shellcode));
        sc.close();
        
        std::cout << "Created shellcode.bin - " << sizeof(shellcode) << " bytes\n";
        std::cout << "Test with: cat shellcode.bin | ./vuln (if vuln is compiled)\n";
    }
};

int main(int argc, char* argv[]) {
    LearnRE re;
    
    if (argc > 1) {
        std::string cmd = argv[1];
        
        if (cmd == "load" && argc > 2) {
            re.load_binary(argv[2]);
        }
        else if (cmd == "vuln") {
            re.create_vulnerable_program();
        }
        else if (cmd == "stack") {
            re.teach_stack_layout();
        }
        else if (cmd == "shellcode") {
            re.generate_simple_shellcode();
        }
        else {
            std::cout << "Commands:\n";
            std::cout << "  load <file>  - Load and analyze binary\n";
            std::cout << "  vuln         - Create vulnerable C program\n";
            std::cout << "  stack        - Show stack layout\n";
            std::cout << "  shellcode    - Generate shellcode\n";
        }
    } else {
        std::cout << "LearnRE - Practical Reverse Engineering\n";
        std::cout << "Usage: " << argv[0] << " <command>\n\n";
        std::cout << "Start with: " << argv[0] << " vuln\n";
    }
    
    return 0;
}