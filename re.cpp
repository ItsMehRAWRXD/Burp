#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    
    std::ifstream file(argv[1], std::ios::binary);
    if (!file) return 1;
    
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0);
    
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    
    // Hex dump
    for (size_t i = 0; i < size; i += 16) {
        printf("%08zx: ", i);
        for (size_t j = 0; j < 16 && i + j < size; j++) {
            printf("%02x ", data[i + j]);
        }
        printf(" ");
        for (size_t j = 0; j < 16 && i + j < size; j++) {
            char c = data[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        printf("\n");
    }
    
    return 0;
}