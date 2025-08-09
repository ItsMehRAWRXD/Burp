#include "../include/autopilot.h"
#include "../include/ide_integration.h"
#include "../include/task_bot.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

namespace TaskBot {

AutoPilotManager::AutoPilotManager() : isRecording_(false) {
    std::cout << "[AutoPilotManager] Initializing AutoPilot system..." << std::endl;
    
    // Initialize all components
    systemController_ = std::make_unique<SystemController>();
    inputSimulator_ = std::make_unique<InputSimulator>();
    windowManager_ = std::make_unique<WindowManager>();
    screenCapture_ = std::make_unique<ScreenCapture>();
    appController_ = std::make_unique<ApplicationController>();
    
    // Register default workflows
    registerDefaultWorkflows();
    
    std::cout << "[AutoPilotManager] AutoPilot system initialized successfully!" << std::endl;
}

AutoPilotManager::~AutoPilotManager() {
    std::cout << "[AutoPilotManager] Shutting down AutoPilot system..." << std::endl;
}

bool AutoPilotManager::executeWorkflow(const std::string& workflowName) {
    std::cout << "[AutoPilotManager] Executing workflow: " << workflowName << std::endl;
    
    auto it = workflows_.find(workflowName);
    if (it == workflows_.end()) {
        std::cerr << "[AutoPilotManager] Workflow not found: " << workflowName << std::endl;
        return false;
    }
    
    try {
        return it->second();
    } catch (const std::exception& e) {
        std::cerr << "[AutoPilotManager] Workflow execution failed: " << e.what() << std::endl;
        return false;
    }
}

bool AutoPilotManager::registerWorkflow(const std::string& name, std::function<bool()> workflow) {
    std::cout << "[AutoPilotManager] Registering workflow: " << name << std::endl;
    workflows_[name] = workflow;
    return true;
}

bool AutoPilotManager::autoCompleteCode(const std::string& partialCode) {
    std::cout << "[AutoPilotManager] Auto-completing code: " << partialCode << std::endl;
    
    // Get current window info
    auto activeWindow = windowManager_->getActiveWindow();
    std::cout << "[AutoPilotManager] Active window: " << activeWindow.title << std::endl;
    
    // Search for code completions
    auto searchEngine = std::make_shared<WebSearchEngine>();
    searchEngine->setMaxResults(5);
    
    std::string query = "code completion " + partialCode;
    auto results = searchEngine->searchCode(query);
    auto suggestions = searchEngine->extractCodeFromResults(results);
    
    if (!suggestions.empty()) {
        std::cout << "[AutoPilotManager] Found " << suggestions.size() << " code suggestions" << std::endl;
        
        // Type the first suggestion
        inputSimulator_->typeText(suggestions[0].code);
        return true;
    }
    
    std::cout << "[AutoPilotManager] No code suggestions found" << std::endl;
    return false;
}

bool AutoPilotManager::extractDataFromApplication(const std::string& appName, const std::string& dataPattern) {
    std::cout << "[AutoPilotManager] Extracting data from " << appName 
              << " with pattern: " << dataPattern << std::endl;
    
    // Find and focus the application window
    auto windowId = windowManager_->findWindowByTitle(appName);
    if (windowId == 0) {
        std::cerr << "[AutoPilotManager] Application window not found: " << appName << std::endl;
        return false;
    }
    
    windowManager_->focusWindow(windowId);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Select all and copy
    inputSimulator_->selectAll();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    inputSimulator_->copy();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Get clipboard content
    std::string data = AutoPilotHelpers::getClipboardContent();
    std::cout << "[AutoPilotManager] Extracted " << data.length() << " bytes of data" << std::endl;
    
    // Save to file
    std::ofstream outFile("extracted_data.txt");
    outFile << data;
    outFile.close();
    
    return true;
}

bool AutoPilotManager::synchronizeApplications(const std::vector<std::string>& apps) {
    std::cout << "[AutoPilotManager] Synchronizing " << apps.size() << " applications" << std::endl;
    
    if (apps.size() < 2) {
        std::cerr << "[AutoPilotManager] Need at least 2 applications to synchronize" << std::endl;
        return false;
    }
    
    // Get data from first application
    std::string sourceApp = apps[0];
    if (!extractDataFromApplication(sourceApp, "*")) {
        return false;
    }
    
    std::string data = AutoPilotHelpers::getClipboardContent();
    
    // Paste data to other applications
    for (size_t i = 1; i < apps.size(); i++) {
        auto windowId = windowManager_->findWindowByTitle(apps[i]);
        if (windowId == 0) {
            std::cerr << "[AutoPilotManager] Application not found: " << apps[i] << std::endl;
            continue;
        }
        
        windowManager_->focusWindow(windowId);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        inputSimulator_->paste();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "[AutoPilotManager] Synchronized data to: " << apps[i] << std::endl;
    }
    
    return true;
}

bool AutoPilotManager::automateDataEntry(const std::map<std::string, std::string>& data) {
    std::cout << "[AutoPilotManager] Automating data entry for " << data.size() << " fields" << std::endl;
    
    for (const auto& entry : data) {
        std::cout << "[AutoPilotManager] Entering data for field: " << entry.first << std::endl;
        
        // Try to find the field on screen
        ScreenRegion region;
        if (screenCapture_->waitForImage("field_" + entry.first + ".png", 5000, region)) {
            // Click on the field
            inputSimulator_->moveMouse(region.x + region.width/2, region.y + region.height/2);
            inputSimulator_->clickMouse();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Clear existing content
            inputSimulator_->selectAll();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            
            // Type new content
            inputSimulator_->typeText(entry.second);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else {
            // Try OCR to find the field
            auto ocrResults = screenCapture_->findTextOnScreen(entry.first);
            if (!ocrResults.empty()) {
                auto& result = ocrResults[0];
                inputSimulator_->moveMouse(result.boundingBox.x + result.boundingBox.width + 10,
                                         result.boundingBox.y + result.boundingBox.height/2);
                inputSimulator_->clickMouse();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                inputSimulator_->typeText(entry.second);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } else {
                std::cerr << "[AutoPilotManager] Could not find field: " << entry.first << std::endl;
            }
        }
    }
    
    return true;
}

bool AutoPilotManager::startRecording(const std::string& macroName) {
    std::cout << "[AutoPilotManager] Starting macro recording: " << macroName << std::endl;
    
    if (isRecording_) {
        std::cerr << "[AutoPilotManager] Already recording!" << std::endl;
        return false;
    }
    
    isRecording_ = true;
    recordedActions_.clear();
    
    // Start recording thread
    std::thread recordThread([this]() {
        while (isRecording_) {
            // Record mouse position
            auto mousePos = inputSimulator_->getMousePosition();
            std::string action = "MOUSE_POS," + std::to_string(mousePos.first) + "," + 
                               std::to_string(mousePos.second);
            recordedActions_.push_back(action);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    recordThread.detach();
    return true;
}

bool AutoPilotManager::stopRecording() {
    std::cout << "[AutoPilotManager] Stopping macro recording" << std::endl;
    
    if (!isRecording_) {
        std::cerr << "[AutoPilotManager] Not currently recording!" << std::endl;
        return false;
    }
    
    isRecording_ = false;
    std::cout << "[AutoPilotManager] Recorded " << recordedActions_.size() << " actions" << std::endl;
    
    return true;
}

bool AutoPilotManager::playMacro(const std::string& macroName, int speed) {
    std::cout << "[AutoPilotManager] Playing macro: " << macroName << " at speed " << speed << "x" << std::endl;
    
    // Load macro if needed
    std::string macroFile = macroName + ".macro";
    if (!loadMacro(macroFile)) {
        std::cerr << "[AutoPilotManager] Failed to load macro: " << macroName << std::endl;
        return false;
    }
    
    // Play recorded actions
    for (const auto& action : recordedActions_) {
        // Parse action
        size_t pos = action.find(',');
        if (pos == std::string::npos) continue;
        
        std::string actionType = action.substr(0, pos);
        std::string params = action.substr(pos + 1);
        
        if (actionType == "MOUSE_POS") {
            size_t commaPos = params.find(',');
            if (commaPos != std::string::npos) {
                int x = std::stoi(params.substr(0, commaPos));
                int y = std::stoi(params.substr(commaPos + 1));
                inputSimulator_->moveMouse(x, y);
            }
        } else if (actionType == "CLICK") {
            inputSimulator_->clickMouse();
        } else if (actionType == "TYPE") {
            inputSimulator_->typeText(params);
        } else if (actionType == "KEY") {
            inputSimulator_->pressKey(params);
        }
        
        // Adjust delay based on speed
        std::this_thread::sleep_for(std::chrono::milliseconds(100 / speed));
    }
    
    std::cout << "[AutoPilotManager] Macro playback completed" << std::endl;
    return true;
}

bool AutoPilotManager::saveMacro(const std::string& macroName, const std::string& filename) {
    std::cout << "[AutoPilotManager] Saving macro: " << macroName << " to " << filename << std::endl;
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[AutoPilotManager] Failed to open file: " << filename << std::endl;
        return false;
    }
    
    file << "# TaskBot Macro: " << macroName << "\n";
    file << "# Recorded at: " << Utils::getCurrentTimestamp() << "\n";
    file << "# Actions: " << recordedActions_.size() << "\n\n";
    
    for (const auto& action : recordedActions_) {
        file << action << "\n";
    }
    
    file.close();
    std::cout << "[AutoPilotManager] Macro saved successfully" << std::endl;
    return true;
}

bool AutoPilotManager::loadMacro(const std::string& filename) {
    std::cout << "[AutoPilotManager] Loading macro from: " << filename << std::endl;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[AutoPilotManager] Failed to open file: " << filename << std::endl;
        return false;
    }
    
    recordedActions_.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;
        
        recordedActions_.push_back(line);
    }
    
    file.close();
    std::cout << "[AutoPilotManager] Loaded " << recordedActions_.size() << " actions" << std::endl;
    return true;
}

bool AutoPilotManager::automateFromDescription(const std::string& taskDescription) {
    std::cout << "[AutoPilotManager] Automating from description: " << taskDescription << std::endl;
    
    // Use web search to find automation scripts
    auto searchEngine = std::make_shared<WebSearchEngine>();
    std::string query = "automation script " + taskDescription;
    auto results = searchEngine->search(query);
    
    if (!results.empty()) {
        std::cout << "[AutoPilotManager] Found " << results.size() << " automation examples" << std::endl;
        
        // Generate automation script based on search results
        std::string script = AutoPilotHelpers::generateAutomationScript(taskDescription);
        
        // Execute the generated script
        return systemController_->executeAutomationScript(script);
    }
    
    std::cerr << "[AutoPilotManager] No automation examples found" << std::endl;
    return false;
}

void AutoPilotManager::registerDefaultWorkflows() {
    // Register common automation workflows
    
    registerWorkflow("open_browser", [this]() {
        std::cout << "[Workflow] Opening web browser" << std::endl;
        return systemController_->launchApplication("firefox", {});
    });
    
    registerWorkflow("take_screenshot", [this]() {
        std::cout << "[Workflow] Taking screenshot" << std::endl;
        std::string filename = "screenshot_" + std::to_string(std::time(nullptr)) + ".png";
        return screenCapture_->captureScreen(filename);
    });
    
    registerWorkflow("switch_windows", [this]() {
        std::cout << "[Workflow] Switching between windows" << std::endl;
        inputSimulator_->executeShortcut("Alt+Tab");
        return true;
    });
    
    registerWorkflow("copy_paste_between_apps", [this]() {
        std::cout << "[Workflow] Copy-paste between applications" << std::endl;
        
        // Copy from current app
        inputSimulator_->selectAll();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        inputSimulator_->copy();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Switch to next app
        inputSimulator_->executeShortcut("Alt+Tab");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Paste
        inputSimulator_->paste();
        return true;
    });
    
    registerWorkflow("fill_web_form", [this]() {
        std::cout << "[Workflow] Filling web form" << std::endl;
        
        std::map<std::string, std::string> formData;
        formData["Name"] = "TaskBot User";
        formData["Email"] = "taskbot@example.com";
        formData["Message"] = "Automated form submission by TaskBot";
        
        return automateDataEntry(formData);
    });
    
    std::cout << "[AutoPilotManager] Registered " << workflows_.size() << " default workflows" << std::endl;
}

// AutoPilotTask implementation
AutoPilotTask::AutoPilotTask(const std::string& name, Operation op,
                           std::shared_ptr<AutoPilotManager> manager,
                           const std::map<std::string, std::string>& params)
    : Task(name, Priority::HIGH), operation_(op), manager_(manager), params_(params) {}

bool AutoPilotTask::execute() {
    std::cout << "[AutoPilotTask] Executing: " << getName() << std::endl;
    
    switch (operation_) {
        case Operation::EXECUTE_WORKFLOW:
            return manager_->executeWorkflow(params_.at("workflow"));
            
        case Operation::RECORD_MACRO:
            return manager_->startRecording(params_.at("macro_name"));
            
        case Operation::PLAY_MACRO:
            return manager_->playMacro(params_.at("macro_name"), 
                                     std::stoi(params_.count("speed") ? params_.at("speed") : "1"));
            
        case Operation::AUTOMATE_APP:
            return manager_->automateFromDescription(params_.at("description"));
            
        case Operation::EXTRACT_DATA:
            return manager_->extractDataFromApplication(params_.at("app_name"), 
                                                      params_.at("pattern"));
            
        case Operation::SYNC_APPS: {
            std::vector<std::string> apps;
            std::string appList = params_.at("apps");
            size_t pos = 0;
            while ((pos = appList.find(',')) != std::string::npos) {
                apps.push_back(appList.substr(0, pos));
                appList.erase(0, pos + 1);
            }
            if (!appList.empty()) apps.push_back(appList);
            return manager_->synchronizeApplications(apps);
        }
            
        case Operation::CUSTOM_AUTOMATION:
            return manager_->automateFromDescription(params_.at("task"));
            
        default:
            std::cerr << "[AutoPilotTask] Unknown operation" << std::endl;
            return false;
    }
}

std::string AutoPilotTask::getDescription() const {
    return "AutoPilot task: " + getName();
}

} // namespace TaskBot