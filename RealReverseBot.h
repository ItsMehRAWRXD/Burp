#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

class RealReverseBot {
private:
    std::vector<uint8_t> binary_data;
    std::string file_path;
    
    struct ELF_Header {
        uint32_t magic;
        uint8_t class_type;
        uint8_t endian;
        uint64_t entry_point;
        uint64_t section_offset;
        uint16_t section_count;
    };
    
    ELF_Header elf_header;
    bool is_valid_elf;
    
public:
    bool load_file(const std::string& path);
    void disassemble_x64(uint64_t offset, size_t count);
    std::vector<std::string> extract_strings();
    void analyze_functions();
    void find_vulnerabilities();
    std::string generate_exploit_template(const std::string& vuln_type);
    std::vector<uint8_t> generate_shellcode_x64();
    void patch_bytes(uint64_t offset, const std::vector<uint8_t>& new_bytes);
    void save_patched(const std::string& output_path);
};