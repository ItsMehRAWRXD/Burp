#pragma once
#include <string>
#include <vector>
#include <map>
#include <fstream>

enum class AnalysisType { DISASM, STRINGS, SYMBOLS, HEADERS, CONTROL_FLOW, EXPLOIT };
enum class Arch { X86, X64, ARM };

struct Binary {
    std::string path;
    std::vector<uint8_t> data;
    Arch arch;
    bool is_elf;
    bool is_pe;
    uint64_t entry_point;
};

struct Function {
    std::string name;
    uint64_t address;
    uint64_t size;
    std::vector<std::string> instructions;
    std::vector<uint64_t> calls;
    bool is_vulnerable;
};

class ReverseBot {
private:
    Binary target;
    std::vector<Function> functions;
    std::map<uint64_t, std::string> strings;
    std::vector<std::string> imports;
    
    void load_binary(const std::string& path);
    void disassemble_at(uint64_t addr, size_t count);
    void find_functions();
    void extract_strings();
    void analyze_vulnerabilities();
    std::string decode_instruction(const uint8_t* bytes, Arch arch);
    
public:
    void analyze(const std::string& binary_path, AnalysisType type);
    void create_exploit(const std::string& vuln_type);
    void patch_binary(uint64_t addr, const std::vector<uint8_t>& patch);
    void generate_shellcode(Arch arch, const std::string& payload);
    void teach_concept(const std::string& concept);
    std::string get_analysis_report();
};