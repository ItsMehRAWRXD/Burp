#ifndef IDE_INTEGRATION_H
#define IDE_INTEGRATION_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <json/json.h> // For JSON-RPC communication

namespace TaskBot {

// Forward declarations
class IDEConnector;
class WebSearchEngine;
class CodeGenerator;

// IDE Types
enum class IDEType {
    VSCODE,
    INTELLIJ,
    SUBLIME,
    VIM,
    EMACS,
    VISUAL_STUDIO,
    CUSTOM
};

// Search result structure
struct SearchResult {
    std::string title;
    std::string url;
    std::string snippet;
    std::string source;
    double relevance;
};

// Code suggestion from search
struct CodeSuggestion {
    std::string language;
    std::string code;
    std::string explanation;
    std::string sourceUrl;
    double confidence;
};

// IDE Command structure
struct IDECommand {
    std::string command;
    std::map<std::string, std::string> parameters;
    std::string workspace;
};

// Base IDE Connector class
class IDEConnector {
public:
    IDEConnector(IDEType type) : ideType_(type) {}
    virtual ~IDEConnector() = default;
    
    // Core IDE operations
    virtual bool connect(const std::string& connectionString) = 0;
    virtual bool disconnect() = 0;
    virtual bool isConnected() const = 0;
    
    // File operations
    virtual bool openFile(const std::string& filePath) = 0;
    virtual bool saveFile(const std::string& filePath) = 0;
    virtual bool createFile(const std::string& filePath, const std::string& content) = 0;
    
    // Editor operations
    virtual bool insertText(const std::string& text, int line = -1, int column = -1) = 0;
    virtual bool replaceText(const std::string& oldText, const std::string& newText) = 0;
    virtual bool executeCommand(const IDECommand& command) = 0;
    
    // Navigation
    virtual bool goToLine(int line) = 0;
    virtual bool goToDefinition(const std::string& symbol) = 0;
    virtual bool findUsages(const std::string& symbol) = 0;
    
    // Project operations
    virtual std::vector<std::string> getOpenFiles() const = 0;
    virtual std::string getCurrentFile() const = 0;
    virtual std::string getSelectedText() const = 0;
    virtual std::string getWorkspacePath() const = 0;
    
    IDEType getType() const { return ideType_; }
    
protected:
    IDEType ideType_;
    bool connected_ = false;
};

// VSCode connector implementation
class VSCodeConnector : public IDEConnector {
public:
    VSCodeConnector();
    
    bool connect(const std::string& connectionString) override;
    bool disconnect() override;
    bool isConnected() const override { return connected_; }
    
    bool openFile(const std::string& filePath) override;
    bool saveFile(const std::string& filePath) override;
    bool createFile(const std::string& filePath, const std::string& content) override;
    
    bool insertText(const std::string& text, int line = -1, int column = -1) override;
    bool replaceText(const std::string& oldText, const std::string& newText) override;
    bool executeCommand(const IDECommand& command) override;
    
    bool goToLine(int line) override;
    bool goToDefinition(const std::string& symbol) override;
    bool findUsages(const std::string& symbol) override;
    
    std::vector<std::string> getOpenFiles() const override;
    std::string getCurrentFile() const override;
    std::string getSelectedText() const override;
    std::string getWorkspacePath() const override;
    
private:
    bool sendCommand(const std::string& method, const Json::Value& params);
    Json::Value receiveResponse();
    
    int socketFd_;
    std::string extensionPath_;
    int requestId_;
};

// Web Search Engine
class WebSearchEngine {
public:
    WebSearchEngine();
    ~WebSearchEngine();
    
    // Search configuration
    void setApiKey(const std::string& apiKey);
    void setSearchEngineId(const std::string& engineId);
    void setMaxResults(int maxResults);
    
    // Search operations
    std::vector<SearchResult> search(const std::string& query);
    std::vector<SearchResult> searchCode(const std::string& query, const std::string& language = "");
    std::vector<SearchResult> searchDocumentation(const std::string& query, const std::string& framework = "");
    std::vector<SearchResult> searchStackOverflow(const std::string& query);
    std::vector<SearchResult> searchGitHub(const std::string& query, const std::string& language = "");
    
    // Advanced search with filters
    std::vector<SearchResult> advancedSearch(const std::string& query, 
                                           const std::map<std::string, std::string>& filters);
    
    // Parse and extract code from search results
    std::vector<CodeSuggestion> extractCodeFromResults(const std::vector<SearchResult>& results);
    
private:
    std::string buildSearchUrl(const std::string& query, const std::map<std::string, std::string>& params);
    std::string performHttpRequest(const std::string& url);
    std::vector<SearchResult> parseSearchResponse(const std::string& response);
    
    std::string apiKey_;
    std::string searchEngineId_;
    int maxResults_;
    std::string userAgent_;
};

// Code Generator from search results
class CodeGenerator {
public:
    struct GenerationOptions {
        std::string language;
        std::string style;
        bool includeComments;
        bool includeImports;
        bool formatCode;
        int maxLineLength;
    };
    
    CodeGenerator();
    
    // Generate code from search results
    std::string generateFromSearchResults(const std::vector<CodeSuggestion>& suggestions,
                                        const GenerationOptions& options);
    
    // Generate specific code patterns
    std::string generateFunction(const std::string& name, const std::string& description,
                               const std::string& language);
    std::string generateClass(const std::string& name, const std::vector<std::string>& methods,
                            const std::string& language);
    std::string generateTest(const std::string& codeToTest, const std::string& framework);
    
    // Code transformation
    std::string refactorCode(const std::string& code, const std::string& pattern);
    std::string optimizeCode(const std::string& code, const std::string& language);
    std::string documentCode(const std::string& code, const std::string& language);
    
    // Template-based generation
    void loadTemplate(const std::string& name, const std::string& templateCode);
    std::string generateFromTemplate(const std::string& templateName, 
                                   const std::map<std::string, std::string>& variables);
    
private:
    std::map<std::string, std::string> templates_;
    std::map<std::string, std::string> languagePatterns_;
};

// IDE Integration Manager
class IDEIntegrationManager {
public:
    IDEIntegrationManager();
    ~IDEIntegrationManager();
    
    // IDE Management
    bool registerIDE(const std::string& name, std::shared_ptr<IDEConnector> connector);
    bool connectToIDE(const std::string& name, const std::string& connectionString);
    bool disconnectFromIDE(const std::string& name);
    std::shared_ptr<IDEConnector> getIDE(const std::string& name);
    
    // Search and code generation
    void setSearchEngine(std::shared_ptr<WebSearchEngine> searchEngine);
    void setCodeGenerator(std::shared_ptr<CodeGenerator> codeGenerator);
    
    // High-level operations
    bool searchAndInsertCode(const std::string& ideName, const std::string& query);
    bool searchAndReplaceCode(const std::string& ideName, const std::string& oldCode, 
                            const std::string& searchQuery);
    bool generateCodeFromDescription(const std::string& ideName, const std::string& description);
    
    // Automated workflows
    bool implementFunction(const std::string& ideName, const std::string& functionSignature);
    bool fixError(const std::string& ideName, const std::string& errorMessage);
    bool addImports(const std::string& ideName, const std::vector<std::string>& symbols);
    
    // Batch operations
    bool processSearchQueries(const std::string& ideName, 
                            const std::vector<std::string>& queries);
    
private:
    std::map<std::string, std::shared_ptr<IDEConnector>> ides_;
    std::shared_ptr<WebSearchEngine> searchEngine_;
    std::shared_ptr<CodeGenerator> codeGenerator_;
};

// Task for IDE operations
class IDETask : public Task {
public:
    enum class Operation {
        SEARCH_AND_INSERT,
        SEARCH_AND_REPLACE,
        GENERATE_CODE,
        IMPLEMENT_FUNCTION,
        FIX_ERROR,
        REFACTOR,
        DOCUMENT_CODE
    };
    
    IDETask(const std::string& name, Operation op, 
            std::shared_ptr<IDEIntegrationManager> manager,
            const std::string& ideName,
            const std::map<std::string, std::string>& params);
    
    bool execute() override;
    std::string getDescription() const override;
    
private:
    Operation operation_;
    std::shared_ptr<IDEIntegrationManager> manager_;
    std::string ideName_;
    std::map<std::string, std::string> params_;
};

// Helper functions for IDE integration
namespace IDEHelpers {
    std::string detectIDEType();
    std::string getIDEConnectionString(IDEType type);
    std::vector<std::string> getAvailableIDEs();
    bool isIDERunning(IDEType type);
    std::string formatCodeForIDE(const std::string& code, IDEType type);
}

} // namespace TaskBot

#endif // IDE_INTEGRATION_H