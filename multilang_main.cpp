#include "MultiLanguageBot.h"
#include <signal.h>
#include <atomic>

// Global bot instance and signal handling
std::atomic<bool> shouldShutdown(false);
MultiLanguageBot* globalBot = nullptr;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down multi-language bot gracefully...\n";
    shouldShutdown = true;
    if (globalBot) {
        globalBot->stop();
    }
}

void printWelcomeMessage() {
    std::cout << R"(
╔═════════════════════════════════════════════════════════════════════════════════╗
║                    Multi-Language CodeGeneratorBot v2.0                        ║
║                                                                                 ║
║              🌐 Internet-Connected Multi-Language AI Bot 🤖                     ║
║                                                                                 ║
║  📋 Supported Languages (20+):                                                  ║
║     C++ • Python • JavaScript • Rust • Go • Java • TypeScript                 ║
║     PHP • Ruby • Shell • C# • Swift • Kotlin • Dart • Scala                   ║
║     Perl • Lua • R • SQL • HTML/CSS                                            ║
║                                                                                 ║
║  🚀 Enhanced Features:                                                           ║
║     • Multi-language code generation                                           ║
║     • Internet connectivity for templates and examples                         ║
║     • GitHub repository integration                                            ║
║     • StackOverflow code examples                                              ║
║     • Package manager integration (npm, pip, cargo, etc.)                      ║
║     • Web application generation                                               ║
║     • API server generation                                                    ║
║     • Cross-language project templates                                         ║
║     • Community-driven learning                                                ║
║     • Autonomous self-improvement                                              ║
║                                                                                 ║
║  🔧 Commands:                                                                    ║
║     lang <language>   - Switch programming language                            ║
║     langs            - Show available languages                                ║
║     web              - Generate web application                                ║
║     api              - Generate API server                                     ║
║     github <query>   - Search GitHub for code examples                        ║
║     fetch <topic>    - Fetch internet resources                               ║
║     project <type>   - Generate full project                                  ║
║     demo <lang>      - Demonstrate language capabilities                       ║
║                                                                                 ║
╚═════════════════════════════════════════════════════════════════════════════════╝
)" << std::endl;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help, -h              Show this help message\n";
    std::cout << "  --language <lang>       Set default language (cpp/python/js/rust/go/java/etc.)\n";
    std::cout << "  --config <file>         Use custom config file\n";
    std::cout << "  --working-dir <dir>     Set working directory\n";
    std::cout << "  --auto-start            Start bot immediately\n";
    std::cout << "  --internet              Enable internet connectivity (default)\n";
    std::cout << "  --no-internet           Disable internet connectivity\n";
    std::cout << "  --enable-lang <langs>   Enable specific languages (comma-separated)\n";
    std::cout << "  --demo <language>       Run demo for specific language\n";
    std::cout << "  --list-languages        List all supported languages\n";
    std::cout << "  --generate <type>       Generate code (class/function/webapp/api/project)\n";
    std::cout << "  --output <file>         Output generated code to file\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << programName << " --language python --auto-start\n";
    std::cout << "  " << programName << " --demo javascript\n";
    std::cout << "  " << programName << " --generate webapp --language python --output app.py\n";
    std::cout << "  " << programName << " --enable-lang cpp,python,rust --auto-start\n";
    std::cout << "  " << programName << " --no-internet --working-dir /tmp/bot\n";
    std::cout << std::endl;
}

void listSupportedLanguages() {
    std::cout << "\n🌐 Supported Programming Languages:\n\n";
    
    struct LangInfo {
        std::string name;
        std::string alias;
        std::string description;
        bool requiresCompiler;
    };
    
    std::vector<LangInfo> languages = {
        {"C++", "cpp", "Systems programming, performance-critical applications", true},
        {"Python", "python/py", "Data science, web development, automation", false},
        {"JavaScript", "javascript/js", "Web development, Node.js applications", false},
        {"Rust", "rust", "Systems programming, memory safety", true},
        {"Go", "go", "Cloud services, microservices", false},
        {"Java", "java", "Enterprise applications, Android development", true},
        {"TypeScript", "typescript/ts", "Type-safe JavaScript development", true},
        {"PHP", "php", "Web development, server-side scripting", false},
        {"Ruby", "ruby/rb", "Web development, scripting", false},
        {"Shell", "shell/bash", "System administration, automation", false},
        {"C#", "csharp/cs", ".NET development, Windows applications", true},
        {"Swift", "swift", "iOS/macOS development", true},
        {"Kotlin", "kotlin/kt", "Android development, JVM applications", true},
        {"Dart", "dart", "Flutter mobile development", false},
        {"Scala", "scala", "JVM functional programming", true},
        {"Perl", "perl", "Text processing, legacy systems", false},
        {"Lua", "lua", "Embedded scripting, game development", false},
        {"R", "r", "Statistical computing, data analysis", false},
        {"SQL", "sql", "Database queries and management", false},
        {"HTML/CSS", "html", "Web markup and styling", false}
    };
    
    for (const auto& lang : languages) {
        std::string compiler = lang.requiresCompiler ? " [Compiled]" : " [Interpreted]";
        std::cout << "  📌 " << lang.name << " (" << lang.alias << ")" << compiler << "\n";
        std::cout << "     " << lang.description << "\n\n";
    }
    
    std::cout << "Use --language <alias> to set default language\n";
    std::cout << "Use 'lang <alias>' command while running to switch languages\n\n";
}

std::string generateCodeStandalone(const std::string& language, const std::string& type, const std::string& name = "") {
    // Create a minimal generator for standalone use
    Language lang = Language::CPP; // Default
    
    if (language == "python" || language == "py") lang = Language::PYTHON;
    else if (language == "javascript" || language == "js") lang = Language::JAVASCRIPT;
    else if (language == "rust") lang = Language::RUST;
    else if (language == "go") lang = Language::GO;
    else if (language == "java") lang = Language::JAVA;
    else if (language == "typescript" || language == "ts") lang = Language::TYPESCRIPT;
    else if (language == "php") lang = Language::PHP;
    else if (language == "ruby" || language == "rb") lang = Language::RUBY;
    else if (language == "shell" || language == "bash") lang = Language::SHELL;
    
    auto logger = std::make_shared<Logger>("standalone.log");
    LanguageCodeGenerator generator(lang, logger);
    
    if (type == "class") {
        return generator.generateClass(name);
    } else if (type == "function") {
        return generator.generateFunction(name);
    } else if (type == "webapp") {
        return generator.generateWebApp();
    } else if (type == "api") {
        return generator.generateAPI();
    } else if (type == "script") {
        return generator.generateScript(name);
    } else {
        return generator.generateProject(type);
    }
}

void runDemo(const std::string& language) {
    std::cout << "\n🎬 " << language << " Demo\n";
    std::cout << "=" << std::string(50, '=') << "\n\n";
    
    std::cout << "1. Class Generation:\n";
    std::cout << generateCodeStandalone(language, "class") << "\n\n";
    
    std::cout << "2. Function Generation:\n";
    std::cout << generateCodeStandalone(language, "function") << "\n\n";
    
    if (language == "python" || language == "javascript" || language == "go") {
        std::cout << "3. Web Application:\n";
        std::cout << generateCodeStandalone(language, "webapp") << "\n\n";
        
        std::cout << "4. API Server:\n";
        std::cout << generateCodeStandalone(language, "api") << "\n\n";
    }
    
    std::cout << "5. Script Generation:\n";
    std::cout << generateCodeStandalone(language, "script") << "\n\n";
    
    std::cout << "Demo completed for " << language << "!\n";
    std::cout << "Run without --demo to start the full interactive bot.\n\n";
}

Language parseLanguage(const std::string& langStr) {
    std::string lang = langStr;
    std::transform(lang.begin(), lang.end(), lang.begin(), ::tolower);
    
    if (lang == "cpp" || lang == "c++") return Language::CPP;
    if (lang == "python" || lang == "py") return Language::PYTHON;
    if (lang == "javascript" || lang == "js") return Language::JAVASCRIPT;
    if (lang == "rust") return Language::RUST;
    if (lang == "go") return Language::GO;
    if (lang == "java") return Language::JAVA;
    if (lang == "typescript" || lang == "ts") return Language::TYPESCRIPT;
    if (lang == "php") return Language::PHP;
    if (lang == "ruby" || lang == "rb") return Language::RUBY;
    if (lang == "shell" || lang == "bash") return Language::SHELL;
    if (lang == "csharp" || lang == "cs") return Language::CSHARP;
    if (lang == "swift") return Language::SWIFT;
    if (lang == "kotlin" || lang == "kt") return Language::KOTLIN;
    if (lang == "dart") return Language::DART;
    if (lang == "scala") return Language::SCALA;
    if (lang == "perl") return Language::PERL;
    if (lang == "lua") return Language::LUA;
    if (lang == "r") return Language::R;
    if (lang == "sql") return Language::SQL;
    if (lang == "html") return Language::HTML_CSS;
    
    return Language::CPP; // Default
}

std::vector<Language> parseLanguageList(const std::string& langList) {
    std::vector<Language> languages;
    std::stringstream ss(langList);
    std::string lang;
    
    while (std::getline(ss, lang, ',')) {
        // Trim whitespace
        lang.erase(0, lang.find_first_not_of(" \t"));
        lang.erase(lang.find_last_not_of(" \t") + 1);
        
        if (!lang.empty()) {
            languages.push_back(parseLanguage(lang));
        }
    }
    
    return languages;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string defaultLanguage = "cpp";
    std::string configFile = "multilang_bot_config.txt";
    std::string workingDir = "multilang_workspace";
    std::string outputFile = "";
    std::string generateType = "";
    std::string demoLanguage = "";
    std::string enabledLanguages = "";
    bool autoStart = false;
    bool showHelp = false;
    bool listLanguages = false;
    bool internetEnabled = true;
    bool demoMode = false;
    bool generateMode = false;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            showHelp = true;
        } else if (arg == "--language" && i + 1 < argc) {
            defaultLanguage = argv[++i];
        } else if (arg == "--config" && i + 1 < argc) {
            configFile = argv[++i];
        } else if (arg == "--working-dir" && i + 1 < argc) {
            workingDir = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (arg == "--generate" && i + 1 < argc) {
            generateType = argv[++i];
            generateMode = true;
        } else if (arg == "--demo" && i + 1 < argc) {
            demoLanguage = argv[++i];
            demoMode = true;
        } else if (arg == "--enable-lang" && i + 1 < argc) {
            enabledLanguages = argv[++i];
        } else if (arg == "--auto-start") {
            autoStart = true;
        } else if (arg == "--internet") {
            internetEnabled = true;
        } else if (arg == "--no-internet") {
            internetEnabled = false;
        } else if (arg == "--list-languages") {
            listLanguages = true;
        } else {
            std::cout << "Unknown option: " << arg << "\n";
            showHelp = true;
        }
    }
    
    if (showHelp) {
        printUsage(argv[0]);
        return 0;
    }
    
    if (listLanguages) {
        listSupportedLanguages();
        return 0;
    }
    
    if (demoMode) {
        runDemo(demoLanguage);
        return 0;
    }
    
    if (generateMode) {
        std::string code = generateCodeStandalone(defaultLanguage, generateType);
        
        if (!outputFile.empty()) {
            std::ofstream file(outputFile);
            if (file.is_open()) {
                file << code;
                std::cout << "Generated " << generateType << " code saved to: " << outputFile << "\n";
            } else {
                std::cerr << "Error: Could not write to file: " << outputFile << "\n";
                return 1;
            }
        } else {
            std::cout << "\n=== Generated " << generateType << " (" << defaultLanguage << ") ===\n";
            std::cout << code << "\n";
            std::cout << "=== End Generated Code ===\n";
        }
        return 0;
    }
    
    // Set up signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        printWelcomeMessage();
        
        // Check basic dependencies
        if (internetEnabled) {
            std::cout << "🌐 Internet connectivity: ENABLED\n";
            std::cout << "  - GitHub integration available\n";
            std::cout << "  - Package manager support available\n";
            std::cout << "  - Community templates available\n";
        } else {
            std::cout << "🔒 Internet connectivity: DISABLED\n";
            std::cout << "  - Operating in offline mode\n";
        }
        
        std::cout << "🔧 Default language: " << defaultLanguage << "\n";
        std::cout << "📁 Working directory: " << workingDir << "\n\n";
        
        // Create bot instance
        MultiLanguageBot bot;
        globalBot = &bot;
        
        // Set default language
        bot.setCurrentLanguage(parseLanguage(defaultLanguage));
        
        // Enable specific languages if specified
        if (!enabledLanguages.empty()) {
            auto languages = parseLanguageList(enabledLanguages);
            for (Language lang : languages) {
                bot.enableLanguage(lang);
            }
            std::cout << "Enabled " << languages.size() << " languages\n";
        }
        
        if (!autoStart) {
            std::cout << "🤖 Multi-Language Bot is ready to start!\n\n";
            std::cout << "The bot will:\n";
            std::cout << "  • Generate code in 20+ programming languages\n";
            std::cout << "  • Connect to internet for templates and examples\n";
            std::cout << "  • Create web applications and APIs\n";
            std::cout << "  • Learn from GitHub and community resources\n";
            std::cout << "  • Improve autonomously over time\n\n";
            
            char choice;
            std::cout << "Start the Multi-Language Bot? (y/n): ";
            std::cin >> choice;
            std::cin.ignore(); // Clear input buffer
            
            if (choice != 'y' && choice != 'Y') {
                std::cout << "Bot startup cancelled.\n";
                return 0;
            }
        }
        
        std::cout << "\n🚀 Starting Multi-Language CodeGeneratorBot...\n";
        
        // Run the bot
        bot.run();
        
        std::cout << "\n✅ Multi-Language Bot shutdown completed.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Unknown fatal error occurred." << std::endl;
        return 1;
    }
    
    return 0;
}

// Original generateCode function for backward compatibility
void originalGenerateCode(const std::string& type) {
    if (type == "class") {
        std::cout << "class MyClass {\npublic:\n    MyClass();\n};\n";
    } else if (type == "function") {
        std::cout << "void myFunction() {\n    // code here\n}\n";
    } else {
        std::cout << "Unknown type.\n";
    }
}

// Demonstrate evolution from original to multi-language
void showEvolution() {
    std::cout << "\n🔄 Evolution from Simple to Multi-Language Bot:\n\n";
    
    std::cout << "ORIGINAL (Simple C++ only):\n";
    std::cout << "─────────────────────────────\n";
    originalGenerateCode("class");
    
    std::cout << "\nENHANCED (Multi-Language with Internet):\n";
    std::cout << "────────────────────────────────────────\n";
    std::cout << "✓ 20+ programming languages\n";
    std::cout << "✓ Internet connectivity for resources\n";
    std::cout << "✓ GitHub integration\n";
    std::cout << "✓ Package manager support\n";
    std::cout << "✓ Web app and API generation\n";
    std::cout << "✓ Self-improving AI\n";
    std::cout << "✓ Community-driven learning\n\n";
    
    std::cout << "Example Python web app:\n";
    std::cout << generateCodeStandalone("python", "webapp") << "\n";
}