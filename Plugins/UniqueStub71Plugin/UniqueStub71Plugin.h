#pragma once

#include "../../PluginFramework/IPlugin.h"
#include <string>
#include <vector>
#include <map>

namespace BenignPacker {
namespace PluginFramework {

class UniqueStub71Plugin : public IStubGenerator {
public:
    UniqueStub71Plugin() = default;
    ~UniqueStub71Plugin() override = default;

    PluginConfig GetConfig() const override;
    bool Initialize(const std::map<std::string, std::string>& settings) override;
    void Shutdown() override;
    bool SupportsCapability(PluginCapabilities capability) const override;
    std::vector<std::string> GetSupportedFileTypes() const override;
    PluginResult Execute(const ExecutionContext& context) override;

    std::vector<uint8_t> GenerateStub(const std::vector<uint8_t>& payload) override;
    bool SetStubTemplate(const std::string& template_path) override;
    std::vector<std::string> GetAvailableTemplates() const override;
    bool SetEncryptionMethod(const std::string& method) override;
    bool SetObfuscationLevel(int level) override;
    std::vector<std::string> GetSupportedEncryption() const override;
};

} // namespace PluginFramework
} // namespace BenignPacker