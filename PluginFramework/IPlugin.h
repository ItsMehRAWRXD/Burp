#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <cstdint>

namespace BenignPacker {
    namespace PluginFramework {

        // Plugin API version for compatibility checking
        #define BENIGN_PACKER_PLUGIN_API_VERSION 1

        // Plugin types
        enum class PluginType {
            STUB_GENERATOR,
            MASM_ASSEMBLER,
            PACKER,
            ENCODER,
            OBFUSCATOR,
            LOADER
        };

        // Plugin capabilities flags
        enum class PluginCapabilities : uint32_t {
            NONE = 0x00000000,
            MUTEX_MANAGEMENT = 0x00000001,
            CERTIFICATE_SPOOFING = 0x00000002,
            EXPLOIT_METHODS = 0x00000004,
            ANTI_ANALYSIS = 0x00000008,
            PROCESS_INJECTION = 0x00000010,
            UAC_BYPASS = 0x00000020,
            PERSISTENCE = 0x00000040,
            NETWORK_EXPLOITS = 0x00000080,
            MASM_INTEGRATION = 0x00000100,
            POLYMORPHIC_CODE = 0x00000200,
            COMPANY_PROFILES = 0x00000400,
            RING0_RING3 = 0x00000800,
            ALL_CAPABILITIES = 0xFFFFFFFF
        };

        // Plugin configuration structure
        struct PluginConfig {
            std::string name;
            std::string version;
            std::string author;
            std::string description;
            PluginType type;
            PluginCapabilities capabilities;
            uint32_t api_version;
            std::map<std::string, std::string> settings;
        };

        // Plugin execution context
        struct ExecutionContext {
            std::string input_file;
            std::string output_file;
            std::vector<uint8_t> payload_data;
            std::map<std::string, std::string> parameters;
            bool verbose_mode;
            bool debug_mode;
        };

        // Plugin result structure
        struct PluginResult {
            bool success;
            std::string message;
            std::vector<uint8_t> output_data;
            std::map<std::string, std::string> metadata;
            uint32_t execution_time_ms;
        };

        // Main plugin interface
        class IPlugin {
        public:
            virtual ~IPlugin() = default;

            // Plugin identification and configuration
            virtual PluginConfig GetConfig() const = 0;
            virtual bool Initialize(const std::map<std::string, std::string>& settings) = 0;
            virtual void Shutdown() = 0;

            // Plugin capabilities
            virtual bool SupportsCapability(PluginCapabilities capability) const = 0;
            virtual std::vector<std::string> GetSupportedFileTypes() const = 0;

            // Main execution method
            virtual PluginResult Execute(const ExecutionContext& context) = 0;

            // Optional methods for advanced plugins
            virtual bool ValidateInput(const ExecutionContext& context) { return true; }
            virtual std::string GetLastError() const { return ""; }
            virtual bool CanChainWith(const IPlugin* other_plugin) const { return false; }
        };

        // Stub generator specific interface
        class IStubGenerator : public IPlugin {
        public:
            virtual ~IStubGenerator() = default;

            // Stub generation methods
            virtual std::vector<uint8_t> GenerateStub(const std::vector<uint8_t>& payload) = 0;
            virtual bool SetStubTemplate(const std::string& template_path) = 0;
            virtual std::vector<std::string> GetAvailableTemplates() const = 0;

            // Encryption and obfuscation
            virtual bool SetEncryptionMethod(const std::string& method) = 0;
            virtual bool SetObfuscationLevel(int level) = 0;
            virtual std::vector<std::string> GetSupportedEncryption() const = 0;
        };

        // MASM assembler specific interface
        class IMASMAssembler : public IPlugin {
        public:
            virtual ~IMASMAssembler() = default;

            // Assembly methods
            virtual bool AssembleFile(const std::string& asm_file, const std::string& output_file) = 0;
            virtual bool AssembleSource(const std::string& asm_source, std::vector<uint8_t>& output) = 0;
            virtual std::vector<std::string> GetLastAssemblyErrors() const = 0;

            // MASM configuration
            virtual bool SetMASMPath(const std::string& masm_path) = 0;
            virtual bool SetIncludePaths(const std::vector<std::string>& include_paths) = 0;
            virtual bool SetAssemblyOptions(const std::map<std::string, std::string>& options) = 0;
        };

        // Plugin factory function type
        typedef std::unique_ptr<IPlugin>(*CreatePluginFunc)();
        typedef void(*DestroyPluginFunc)(IPlugin*);
        typedef uint32_t(*GetApiVersionFunc)();

        // Plugin export macros
        #define EXPORT_PLUGIN_FUNC extern "C" __declspec(dllexport)

        #define DECLARE_PLUGIN_EXPORTS(PluginClassName) \
            EXPORT_PLUGIN_FUNC std::unique_ptr<IPlugin> CreatePlugin() { \
                return std::make_unique<PluginClassName>(); \
            } \
            EXPORT_PLUGIN_FUNC void DestroyPlugin(IPlugin* plugin) { \
                delete plugin; \
            } \
            EXPORT_PLUGIN_FUNC uint32_t GetApiVersion() { \
                return BENIGN_PACKER_PLUGIN_API_VERSION; \
            }

        // Capability helper functions
        inline PluginCapabilities operator|(PluginCapabilities a, PluginCapabilities b) {
            return static_cast<PluginCapabilities>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
        }

        inline PluginCapabilities operator&(PluginCapabilities a, PluginCapabilities b) {
            return static_cast<PluginCapabilities>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
        }

        inline bool HasCapability(PluginCapabilities caps, PluginCapabilities check) {
            return (caps & check) == check;
        }

    } // namespace PluginFramework
} // namespace BenignPacker