#ifndef MULTI_LANGUAGE_BOT_H
#define MULTI_LANGUAGE_BOT_H

#include "CodeGeneratorBot.h"
#include <curl/curl.h>
#include <json/json.h>
#include <regex>
#include <unordered_map>
#include <set>

// Supported programming languages
enum class Language {
    CPP,
    PYTHON,
    JAVASCRIPT,
    RUST,
    GO,
    JAVA,
    CSHARP,
    PHP,
    RUBY,
    SWIFT,
    KOTLIN,
    TYPESCRIPT,
    SHELL,
    SQL,
    HTML_CSS,
    R,
    SCALA,
    PERL,
    LUA,
    DART
};

// Language configuration structure
struct LanguageConfig {
    std::string name;
    std::string extension;
    std::string compiler;
    std::string interpreter;
    std::string runCommand;
    std::string packageManager;
    std::vector<std::string> commonImports;
    std::vector<std::string> templateUrls;
    bool needsCompilation;
    bool supportsInteractive;
};

// Internet resource types
enum class ResourceType {
    GITHUB_REPO,
    STACKOVERFLOW,
    DOCUMENTATION,
    PACKAGE_REGISTRY,
    CODE_EXAMPLE,
    TUTORIAL,
    API_REFERENCE
};

// Web resource structure
struct WebResource {
    ResourceType type;
    std::string url;
    std::string content;
    Language language;
    std::vector<std::string> tags;
    double relevanceScore;
    std::chrono::system_clock::time_point fetchedAt;
};

// HTTP response structure
struct HttpResponse {
    std::string data;
    long responseCode;
    bool success;
};

// Language-specific code generator
class LanguageCodeGenerator {
private:
    Language language;
    LanguageConfig config;
    std::shared_ptr<Logger> logger;
    std::vector<std::string> classTemplates;
    std::vector<std::string> functionTemplates;
    std::vector<std::string> projectTemplates;
    std::random_device rd;
    std::mt19937 gen;

public:
    LanguageCodeGenerator(Language lang, std::shared_ptr<Logger> log);
    
    std::string generateClass(const std::string& className = "");
    std::string generateFunction(const std::string& functionName = "");
    std::string generateProject(const std::string& projectType = "");
    std::string generateWebApp();
    std::string generateAPI();
    std::string generateScript(const std::string& purpose = "");
    
    void addTemplate(const std::string& tmpl, const std::string& type);
    void loadTemplatesFromWeb();
    LanguageConfig getConfig() const { return config; }
    
private:
    void initializeTemplates();
    std::string replaceTokens(const std::string& tmpl, const std::map<std::string, std::string>& tokens);
    std::string generateRandomName(const std::string& prefix);
};

// Multi-language code executor
class MultiLanguageExecutor {
private:
    std::shared_ptr<Logger> logger;
    std::shared_ptr<ConfigManager> config;
    std::string workingDirectory;
    std::unordered_map<Language, LanguageConfig> languageConfigs;

public:
    MultiLanguageExecutor(std::shared_ptr<Logger> log, std::shared_ptr<ConfigManager> cfg);
    
    bool compileCode(const std::string& sourceFile, Language lang, const std::string& outputFile = "");
    bool executeCode(const std::string& file, Language lang, const std::string& args = "");
    std::string runCode(const std::string& code, Language lang, bool keepFiles = false);
    bool installDependencies(Language lang, const std::vector<std::string>& packages);
    bool checkLanguageSupport(Language lang);
    
    void setupLanguageEnvironment(Language lang);
    std::vector<Language> getAvailableLanguages();
    
private:
    void initializeLanguageConfigs();
    bool executeCommand(const std::string& command, std::string& output);
    std::string generateTempFilename(Language lang);
    bool installPackageManager(Language lang);
};

// Internet connectivity and web scraping
class WebConnector {
private:
    std::shared_ptr<Logger> logger;
    CURL* curl;
    std::vector<WebResource> cachedResources;
    std::string userAgent;
    int maxRetries;
    int timeoutSeconds;

public:
    WebConnector(std::shared_ptr<Logger> log);
    ~WebConnector();
    
    HttpResponse httpGet(const std::string& url, const std::map<std::string, std::string>& headers = {});
    HttpResponse httpPost(const std::string& url, const std::string& data, 
                         const std::map<std::string, std::string>& headers = {});
    
    std::vector<WebResource> searchGitHub(const std::string& query, Language lang);
    std::vector<WebResource> searchStackOverflow(const std::string& query, Language lang);
    std::vector<WebResource> fetchDocumentation(Language lang, const std::string& topic = "");
    std::vector<WebResource> getCodeExamples(Language lang, const std::string& feature);
    
    std::string downloadFile(const std::string& url, const std::string& localPath);
    bool cloneGitRepository(const std::string& repoUrl, const std::string& localPath);
    
    std::vector<std::string> extractCodeBlocks(const std::string& html, Language lang);
    std::vector<std::string> parseJsonResponse(const std::string& json, const std::string& field);
    
private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    void initializeCurl();
    std::string buildGitHubApiUrl(const std::string& query, Language lang);
    std::string buildStackOverflowApiUrl(const std::string& query, Language lang);
    std::string languageToString(Language lang);
    Language stringToLanguage(const std::string& langStr);
};

// Package and dependency manager
class PackageManager {
private:
    std::shared_ptr<Logger> logger;
    std::shared_ptr<WebConnector> webConnector;
    std::unordered_map<Language, std::vector<std::string>> popularPackages;

public:
    PackageManager(std::shared_ptr<Logger> log, std::shared_ptr<WebConnector> web);
    
    bool installPackage(Language lang, const std::string& packageName);
    bool uninstallPackage(Language lang, const std::string& packageName);
    std::vector<std::string> searchPackages(Language lang, const std::string& query);
    std::vector<std::string> getPopularPackages(Language lang);
    
    bool createProject(Language lang, const std::string& projectName, const std::string& template = "");
    bool addDependency(Language lang, const std::string& projectPath, const std::string& dependency);
    
    std::string generatePackageFile(Language lang, const std::string& projectName, 
                                  const std::vector<std::string>& dependencies);
    
private:
    void initializePopularPackages();
    std::string getPackageManagerCommand(Language lang, const std::string& action);
};

// Enhanced multi-language bot
class MultiLanguageBot : public CodeGeneratorBot {
private:
    std::unordered_map<Language, std::shared_ptr<LanguageCodeGenerator>> generators;
    std::shared_ptr<MultiLanguageExecutor> multiExecutor;
    std::shared_ptr<WebConnector> webConnector;
    std::shared_ptr<PackageManager> packageManager;
    
    std::vector<Language> enabledLanguages;
    Language currentLanguage;
    bool internetEnabled;
    bool autoFetchResources;

public:
    MultiLanguageBot();
    ~MultiLanguageBot();
    
    void initialize() override;
    void run() override;
    
    // Multi-language specific methods
    void setCurrentLanguage(Language lang);
    Language getCurrentLanguage() const { return currentLanguage; }
    void enableLanguage(Language lang);
    void disableLanguage(Language lang);
    std::vector<Language> getEnabledLanguages() const { return enabledLanguages; }
    
    // Enhanced code generation
    std::string generateCodeForLanguage(Language lang, const std::string& type, const std::string& name = "");
    std::string generateFullProject(Language lang, const std::string& projectType);
    std::string generateFromTemplate(Language lang, const std::string& templateUrl);
    
    // Internet-enhanced features
    void fetchLatestTemplates();
    void updateLanguageSupport();
    std::string generateCodeFromStackOverflow(Language lang, const std::string& problem);
    std::string adaptCodeFromGitHub(const std::string& repoUrl, Language targetLang);
    
    // Interactive features
    void processMultiLanguageCommand(const std::string& command);
    void showLanguageStatus();
    void demonstrateLanguage(Language lang);
    
    // Auto-learning from internet
    void learnFromSuccessfulProjects();
    void discoverNewPatterns();
    void updateTemplatesFromCommunity();
    
protected:
    void executeMultiLanguageTask(const Task& task);
    void initializeLanguageGenerators();
    void setupInternetConnectivity();
    std::string getLanguageName(Language lang);
};

// Task types for multi-language operations
enum class MultiLangTaskType {
    GENERATE_MULTI_LANG_PROJECT,
    CROSS_LANGUAGE_TRANSLATION,
    FETCH_WEB_RESOURCES,
    INSTALL_DEPENDENCIES,
    CLONE_REPOSITORY,
    GENERATE_FROM_TEMPLATE,
    LANGUAGE_BENCHMARK,
    COMMUNITY_LEARNING
};

// Extended task structure for multi-language operations
class MultiLanguageTask : public Task {
public:
    Language primaryLanguage;
    Language secondaryLanguage;
    std::vector<std::string> requiredPackages;
    std::string repositoryUrl;
    std::string templateUrl;
    MultiLangTaskType multiLangType;
    
    MultiLanguageTask(MultiLangTaskType mlType, Language lang, Priority p, const std::string& desc)
        : Task(static_cast<TaskType>(100 + static_cast<int>(mlType)), p, desc),
          primaryLanguage(lang), secondaryLanguage(Language::CPP), multiLangType(mlType) {}
};

#endif // MULTI_LANGUAGE_BOT_H