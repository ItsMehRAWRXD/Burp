#include "MultiLanguageBot.h"
#include <curl/curl.h>
#include <json/json.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

// ========== LanguageCodeGenerator Implementation ==========
LanguageCodeGenerator::LanguageCodeGenerator(Language lang, std::shared_ptr<Logger> log)
    : language(lang), logger(log), gen(rd()) {
    
    // Initialize language configuration
    switch (language) {
        case Language::CPP:
            config = {"C++", ".cpp", "g++", "", "g++ -std=c++17 -O2 {source} -o {output}", "", 
                     {"#include <iostream>", "#include <vector>", "#include <string>"}, {}, true, false};
            break;
        case Language::PYTHON:
            config = {"Python", ".py", "", "python3", "python3 {source}", "pip", 
                     {"import os", "import sys", "import json"}, {}, false, true};
            break;
        case Language::JAVASCRIPT:
            config = {"JavaScript", ".js", "", "node", "node {source}", "npm", 
                     {"const fs = require('fs');", "const path = require('path');"}, {}, false, true};
            break;
        case Language::RUST:
            config = {"Rust", ".rs", "rustc", "", "rustc -O {source} -o {output}", "cargo", 
                     {"use std::io;", "use std::collections::HashMap;"}, {}, true, false};
            break;
        case Language::GO:
            config = {"Go", ".go", "go", "", "go run {source}", "go get", 
                     {"package main", "import \"fmt\"", "import \"os\""}, {}, false, false};
            break;
        case Language::JAVA:
            config = {"Java", ".java", "javac", "java", "javac {source} && java {class}", "maven", 
                     {"import java.util.*;", "import java.io.*;"}, {}, true, false};
            break;
        case Language::TYPESCRIPT:
            config = {"TypeScript", ".ts", "tsc", "node", "tsc {source} && node {output}", "npm", 
                     {"import * as fs from 'fs';", "import * as path from 'path';"}, {}, true, false};
            break;
        case Language::PHP:
            config = {"PHP", ".php", "", "php", "php {source}", "composer", 
                     {"<?php"}, {}, false, true};
            break;
        case Language::RUBY:
            config = {"Ruby", ".rb", "", "ruby", "ruby {source}", "gem", 
                     {"require 'json'", "require 'net/http'"}, {}, false, true};
            break;
        case Language::SHELL:
            config = {"Shell", ".sh", "", "bash", "bash {source}", "", 
                     {"#!/bin/bash"}, {}, false, true};
            break;
        default:
            config = {"Unknown", ".txt", "", "", "", "", {}, {}, false, false};
    }
    
    initializeTemplates();
    logger->info("Initialized " + config.name + " code generator");
}

void LanguageCodeGenerator::initializeTemplates() {
    switch (language) {
        case Language::CPP:
            classTemplates = {
                "class {NAME} {\npublic:\n    {NAME}();\n    ~{NAME}();\n    void process();\nprivate:\n    std::string data_;\n};",
                "template<typename T>\nclass {NAME} {\npublic:\n    explicit {NAME}(T value) : value_(value) {}\n    T getValue() const { return value_; }\nprivate:\n    T value_;\n};",
                "class {NAME} : public std::enable_shared_from_this<{NAME}> {\npublic:\n    static std::shared_ptr<{NAME}> create();\n    void run();\nprivate:\n    {NAME}() = default;\n};"
            };
            functionTemplates = {
                "auto {NAME}() -> void {\n    std::cout << \"Function {NAME} executed\\n\";\n}",
                "template<typename T>\nconstexpr T {NAME}(T a, T b) {\n    return a + b;\n}",
                "[[nodiscard]] std::string {NAME}(std::string_view input) {\n    return std::string(input) + \"_processed\";\n}"
            };
            break;
            
        case Language::PYTHON:
            classTemplates = {
                "class {NAME}:\n    def __init__(self):\n        self.data = []\n    \n    def process(self):\n        return f'Processing {len(self.data)} items'",
                "from abc import ABC, abstractmethod\n\nclass {NAME}(ABC):\n    @abstractmethod\n    def execute(self):\n        pass\n    \n    def common_method(self):\n        return 'Common functionality'",
                "@dataclass\nclass {NAME}:\n    name: str\n    value: int = 0\n    \n    def __post_init__(self):\n        self.processed = True"
            };
            functionTemplates = {
                "def {NAME}(*args, **kwargs):\n    \"\"\"Generated function {NAME}\"\"\"\n    return f'Function {NAME} called with args: {args}'",
                "async def {NAME}(data: list) -> dict:\n    \"\"\"Async function {NAME}\"\"\"\n    await asyncio.sleep(0.1)\n    return {'processed': len(data)}",
                "@functools.lru_cache(maxsize=128)\ndef {NAME}(n: int) -> int:\n    \"\"\"Cached function {NAME}\"\"\"\n    return n * 2 if n > 0 else 0"
            };
            break;
            
        case Language::JAVASCRIPT:
            classTemplates = {
                "class {NAME} {\n    constructor() {\n        this.data = [];\n    }\n    \n    process() {\n        return `Processing ${this.data.length} items`;\n    }\n}",
                "class {NAME} extends EventEmitter {\n    constructor(options = {}) {\n        super();\n        this.options = options;\n    }\n    \n    async execute() {\n        this.emit('started');\n        // Implementation here\n        this.emit('completed');\n    }\n}"
            };
            functionTemplates = {
                "function {NAME}(...args) {\n    console.log(`Function {NAME} called with:`, args);\n    return args.length;\n}",
                "const {NAME} = async (data) => {\n    const result = await processData(data);\n    return result;\n};",
                "const {NAME} = (function() {\n    let cache = new Map();\n    return function(key) {\n        if (!cache.has(key)) {\n            cache.set(key, expensiveOperation(key));\n        }\n        return cache.get(key);\n    };\n})();"
            };
            break;
            
        case Language::RUST:
            classTemplates = {
                "#[derive(Debug, Clone)]\npub struct {NAME} {\n    data: Vec<String>,\n}\n\nimpl {NAME} {\n    pub fn new() -> Self {\n        Self { data: Vec::new() }\n    }\n    \n    pub fn process(&mut self) -> Result<usize, Box<dyn std::error::Error>> {\n        Ok(self.data.len())\n    }\n}",
                "pub trait {NAME} {\n    type Output;\n    fn execute(&self) -> Self::Output;\n}\n\n#[derive(Default)]\npub struct {NAME}Impl;\n\nimpl {NAME} for {NAME}Impl {\n    type Output = String;\n    fn execute(&self) -> Self::Output {\n        String::from(\"Executed\")\n    }\n}"
            };
            functionTemplates = {
                "pub fn {NAME}() -> Result<(), Box<dyn std::error::Error>> {\n    println!(\"Function {NAME} executed\");\n    Ok(())\n}",
                "pub async fn {NAME}<T>(data: Vec<T>) -> Result<usize, tokio::task::JoinError> \nwhere T: Send + 'static {\n    let len = data.len();\n    tokio::spawn(async move { len }).await\n}"
            };
            break;
            
        case Language::GO:
            classTemplates = {
                "type {NAME} struct {\n    data []string\n}\n\nfunc New{NAME}() *{NAME} {\n    return &{NAME}{data: make([]string, 0)}\n}\n\nfunc (s *{NAME}) Process() int {\n    return len(s.data)\n}",
                "type {NAME} interface {\n    Execute() error\n    GetStatus() string\n}\n\ntype {NAME}Impl struct {\n    status string\n}\n\nfunc (impl *{NAME}Impl) Execute() error {\n    impl.status = \"executed\"\n    return nil\n}\n\nfunc (impl *{NAME}Impl) GetStatus() string {\n    return impl.status\n}"
            };
            functionTemplates = {
                "func {NAME}() error {\n    fmt.Println(\"Function {NAME} executed\")\n    return nil\n}",
                "func {NAME}[T any](data []T) (int, error) {\n    if data == nil {\n        return 0, errors.New(\"data is nil\")\n    }\n    return len(data), nil\n}"
            };
            break;
            
        case Language::JAVA:
            classTemplates = {
                "public class {NAME} {\n    private List<String> data;\n    \n    public {NAME}() {\n        this.data = new ArrayList<>();\n    }\n    \n    public int process() {\n        return data.size();\n    }\n}",
                "public interface {NAME} {\n    void execute();\n    String getStatus();\n}\n\npublic class {NAME}Impl implements {NAME} {\n    private String status = \"ready\";\n    \n    @Override\n    public void execute() {\n        this.status = \"executed\";\n    }\n    \n    @Override\n    public String getStatus() {\n        return status;\n    }\n}"
            };
            functionTemplates = {
                "public static void {NAME}() {\n    System.out.println(\"Function {NAME} executed\");\n}",
                "public static <T> CompletableFuture<Integer> {NAME}(List<T> data) {\n    return CompletableFuture.supplyAsync(() -> data.size());\n}"
            };
            break;
            
        default:
            // Default templates for other languages
            classTemplates = {"// {NAME} class placeholder"};
            functionTemplates = {"// {NAME} function placeholder"};
    }
}

std::string LanguageCodeGenerator::generateClass(const std::string& className) {
    std::string name = className.empty() ? generateRandomName("Class") : className;
    if (classTemplates.empty()) return "// No class templates available";
    
    std::uniform_int_distribution<> dis(0, classTemplates.size() - 1);
    std::string tmpl = classTemplates[dis(gen)];
    
    std::map<std::string, std::string> tokens = {{"NAME", name}};
    return replaceTokens(tmpl, tokens);
}

std::string LanguageCodeGenerator::generateFunction(const std::string& functionName) {
    std::string name = functionName.empty() ? generateRandomName("function") : functionName;
    if (functionTemplates.empty()) return "// No function templates available";
    
    std::uniform_int_distribution<> dis(0, functionTemplates.size() - 1);
    std::string tmpl = functionTemplates[dis(gen)];
    
    std::map<std::string, std::string> tokens = {{"NAME", name}};
    return replaceTokens(tmpl, tokens);
}

std::string LanguageCodeGenerator::generateProject(const std::string& projectType) {
    std::stringstream project;
    
    // Add imports/includes
    for (const auto& import : config.commonImports) {
        project << import << "\n";
    }
    project << "\n";
    
    if (projectType == "web_app") {
        return generateWebApp();
    } else if (projectType == "api") {
        return generateAPI();
    } else if (projectType == "script") {
        return generateScript();
    }
    
    // Default project structure
    std::string className = generateClass();
    std::string functionName = generateFunction();
    
    project << className << "\n\n";
    project << functionName << "\n\n";
    
    // Add main/entry point based on language
    switch (language) {
        case Language::CPP:
            project << "int main() {\n    std::cout << \"Generated project executing...\\n\";\n    return 0;\n}\n";
            break;
        case Language::PYTHON:
            project << "if __name__ == '__main__':\n    print('Generated project executing...')\n";
            break;
        case Language::JAVASCRIPT:
            project << "console.log('Generated project executing...');\n";
            break;
        case Language::JAVA:
            project << "public class Main {\n    public static void main(String[] args) {\n        System.out.println(\"Generated project executing...\");\n    }\n}\n";
            break;
        case Language::GO:
            project << "func main() {\n    fmt.Println(\"Generated project executing...\")\n}\n";
            break;
        case Language::RUST:
            project << "fn main() {\n    println!(\"Generated project executing...\");\n}\n";
            break;
        default:
            project << "// Main entry point\n";
    }
    
    return project.str();
}

std::string LanguageCodeGenerator::generateWebApp() {
    switch (language) {
        case Language::JAVASCRIPT:
            return R"(const express = require('express');
const app = express();
const port = 3000;

app.use(express.json());
app.use(express.static('public'));

app.get('/', (req, res) => {
    res.send('<h1>Generated Web App</h1><p>Hello from auto-generated Express app!</p>');
});

app.get('/api/status', (req, res) => {
    res.json({ status: 'running', timestamp: new Date().toISOString() });
});

app.listen(port, () => {
    console.log(`Generated web app listening at http://localhost:${port}`);
});)";
            
        case Language::PYTHON:
            return R"(from flask import Flask, jsonify, render_template_string
from datetime import datetime

app = Flask(__name__)

@app.route('/')
def home():
    return '''
    <h1>Generated Web App</h1>
    <p>Hello from auto-generated Flask app!</p>
    <a href="/api/status">Check Status</a>
    '''

@app.route('/api/status')
def status():
    return jsonify({
        'status': 'running',
        'timestamp': datetime.now().isoformat()
    })

if __name__ == '__main__':
    print('Generated web app starting...')
    app.run(debug=True, port=5000))";
            
        case Language::GO:
            return R"(package main

import (
    "encoding/json"
    "fmt"
    "log"
    "net/http"
    "time"
)

type Status struct {
    Status    string `json:"status"`
    Timestamp string `json:"timestamp"`
}

func homeHandler(w http.ResponseWriter, r *http.Request) {
    html := `<h1>Generated Web App</h1><p>Hello from auto-generated Go app!</p><a href="/api/status">Check Status</a>`
    w.Header().Set("Content-Type", "text/html")
    fmt.Fprint(w, html)
}

func statusHandler(w http.ResponseWriter, r *http.Request) {
    status := Status{
        Status:    "running",
        Timestamp: time.Now().Format(time.RFC3339),
    }
    w.Header().Set("Content-Type", "application/json")
    json.NewEncoder(w).Encode(status)
}

func main() {
    http.HandleFunc("/", homeHandler)
    http.HandleFunc("/api/status", statusHandler)
    
    fmt.Println("Generated web app listening at http://localhost:8080")
    log.Fatal(http.ListenAndServe(":8080", nil))
})";
            
        default:
            return "// Web app template not available for " + config.name;
    }
}

std::string LanguageCodeGenerator::generateAPI() {
    switch (language) {
        case Language::PYTHON:
            return R"(from fastapi import FastAPI
from pydantic import BaseModel
from typing import List, Optional
import uvicorn

app = FastAPI(title="Generated API", version="1.0.0")

class Item(BaseModel):
    id: Optional[int] = None
    name: str
    description: Optional[str] = None

items_db = []

@app.get("/")
def read_root():
    return {"message": "Generated API is running"}

@app.get("/items", response_model=List[Item])
def get_items():
    return items_db

@app.post("/items", response_model=Item)
def create_item(item: Item):
    item.id = len(items_db) + 1
    items_db.append(item)
    return item

@app.get("/items/{item_id}", response_model=Item)
def get_item(item_id: int):
    for item in items_db:
        if item.id == item_id:
            return item
    return {"error": "Item not found"}

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000))";
            
        case Language::JAVASCRIPT:
            return R"(const express = require('express');
const app = express();
const port = 3000;

app.use(express.json());

let items = [];
let nextId = 1;

// API Routes
app.get('/api', (req, res) => {
    res.json({ message: 'Generated API is running', version: '1.0.0' });
});

app.get('/api/items', (req, res) => {
    res.json(items);
});

app.post('/api/items', (req, res) => {
    const item = {
        id: nextId++,
        ...req.body,
        createdAt: new Date().toISOString()
    };
    items.push(item);
    res.status(201).json(item);
});

app.get('/api/items/:id', (req, res) => {
    const item = items.find(i => i.id === parseInt(req.params.id));
    if (!item) {
        return res.status(404).json({ error: 'Item not found' });
    }
    res.json(item);
});

app.delete('/api/items/:id', (req, res) => {
    const index = items.findIndex(i => i.id === parseInt(req.params.id));
    if (index === -1) {
        return res.status(404).json({ error: 'Item not found' });
    }
    items.splice(index, 1);
    res.status(204).send();
});

app.listen(port, () => {
    console.log(`Generated API listening at http://localhost:${port}`);
});)";
            
        default:
            return "// API template not available for " + config.name;
    }
}

std::string LanguageCodeGenerator::generateScript(const std::string& purpose) {
    std::string scriptPurpose = purpose.empty() ? "automation" : purpose;
    
    switch (language) {
        case Language::PYTHON:
            return R"(#!/usr/bin/env python3
"""
Generated automation script
Purpose: Data processing and file operations
"""

import os
import sys
import json
import argparse
from pathlib import Path

def process_files(directory):
    """Process files in the given directory"""
    path = Path(directory)
    if not path.exists():
        print(f"Directory {directory} does not exist")
        return
    
    files_processed = 0
    for file_path in path.rglob("*"):
        if file_path.is_file():
            print(f"Processing: {file_path}")
            files_processed += 1
    
    print(f"Processed {files_processed} files")

def main():
    parser = argparse.ArgumentParser(description="Generated automation script")
    parser.add_argument("directory", help="Directory to process")
    parser.add_argument("--verbose", action="store_true", help="Verbose output")
    
    args = parser.parse_args()
    
    if args.verbose:
        print("Running in verbose mode")
    
    process_files(args.directory)

if __name__ == "__main__":
    main())";
            
        case Language::SHELL:
            return R"(#!/bin/bash
# Generated automation script
# Purpose: System monitoring and maintenance

set -euo pipefail

# Configuration
LOG_FILE="/var/log/generated_script.log"
TEMP_DIR="/tmp/generated_script"

# Functions
log_message() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

check_system() {
    log_message "Checking system status..."
    
    # Check disk space
    df -h | head -n 1
    df -h | grep -E '^/dev/' | awk '{print $1 ": " $5 " used"}'
    
    # Check memory
    free -h | grep Mem | awk '{print "Memory: " $3 "/" $2 " used"}'
    
    # Check load average
    uptime | awk '{print "Load average: " $(NF-2) $(NF-1) $NF}'
}

cleanup() {
    log_message "Performing cleanup..."
    [ -d "$TEMP_DIR" ] && rm -rf "$TEMP_DIR"
    log_message "Cleanup completed"
}

main() {
    log_message "Generated script starting..."
    
    mkdir -p "$TEMP_DIR"
    check_system
    cleanup
    
    log_message "Generated script completed"
}

# Trap cleanup on exit
trap cleanup EXIT

main "$@")";
            
        default:
            return "// Script template not available for " + config.name;
    }
}

std::string LanguageCodeGenerator::replaceTokens(const std::string& tmpl, const std::map<std::string, std::string>& tokens) {
    std::string result = tmpl;
    for (const auto& token : tokens) {
        std::string placeholder = "{" + token.first + "}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), token.second);
            pos += token.second.length();
        }
    }
    return result;
}

std::string LanguageCodeGenerator::generateRandomName(const std::string& prefix) {
    std::uniform_int_distribution<> dis(1000, 9999);
    return prefix + std::to_string(dis(gen));
}

// ========== MultiLanguageExecutor Implementation ==========
MultiLanguageExecutor::MultiLanguageExecutor(std::shared_ptr<Logger> log, std::shared_ptr<ConfigManager> cfg)
    : logger(log), config(cfg) {
    workingDirectory = config->get("working_directory", "bot_workspace");
    initializeLanguageConfigs();
    logger->info("MultiLanguageExecutor initialized");
}

void MultiLanguageExecutor::initializeLanguageConfigs() {
    // Initialize configurations for all supported languages
    languageConfigs[Language::CPP] = {"C++", ".cpp", "g++", "", "g++ -std=c++17 -O2 {source} -o {output}", "", {}, {}, true, false};
    languageConfigs[Language::PYTHON] = {"Python", ".py", "", "python3", "python3 {source}", "pip", {}, {}, false, true};
    languageConfigs[Language::JAVASCRIPT] = {"JavaScript", ".js", "", "node", "node {source}", "npm", {}, {}, false, true};
    languageConfigs[Language::RUST] = {"Rust", ".rs", "rustc", "", "rustc -O {source} -o {output}", "cargo", {}, {}, true, false};
    languageConfigs[Language::GO] = {"Go", ".go", "go", "", "go run {source}", "go get", {}, {}, false, false};
    languageConfigs[Language::JAVA] = {"Java", ".java", "javac", "java", "javac {source}", "maven", {}, {}, true, false};
    languageConfigs[Language::PHP] = {"PHP", ".php", "", "php", "php {source}", "composer", {}, {}, false, true};
    languageConfigs[Language::RUBY] = {"Ruby", ".rb", "", "ruby", "ruby {source}", "gem", {}, {}, false, true};
    languageConfigs[Language::TYPESCRIPT] = {"TypeScript", ".ts", "tsc", "node", "tsc {source}", "npm", {}, {}, true, false};
    languageConfigs[Language::SHELL] = {"Shell", ".sh", "", "bash", "bash {source}", "", {}, {}, false, true};
}

bool MultiLanguageExecutor::compileCode(const std::string& sourceFile, Language lang, const std::string& outputFile) {
    auto it = languageConfigs.find(lang);
    if (it == languageConfigs.end()) {
        logger->error("Language not supported for compilation");
        return false;
    }
    
    const LanguageConfig& langConfig = it->second;
    if (!langConfig.needsCompilation) {
        logger->info("Language " + langConfig.name + " does not require compilation");
        return true;
    }
    
    std::string command = langConfig.runCommand;
    std::string output = outputFile.empty() ? sourceFile.substr(0, sourceFile.find_last_of('.')) : outputFile;
    
    // Replace placeholders
    size_t pos = 0;
    while ((pos = command.find("{source}", pos)) != std::string::npos) {
        command.replace(pos, 8, sourceFile);
        pos += sourceFile.length();
    }
    pos = 0;
    while ((pos = command.find("{output}", pos)) != std::string::npos) {
        command.replace(pos, 8, output);
        pos += output.length();
    }
    
    logger->info("Compiling " + langConfig.name + ": " + command);
    
    std::string cmdOutput;
    bool success = executeCommand(command, cmdOutput);
    
    if (success) {
        logger->info("Compilation successful");
    } else {
        logger->error("Compilation failed: " + cmdOutput);
    }
    
    return success;
}

std::string MultiLanguageExecutor::runCode(const std::string& code, Language lang, bool keepFiles) {
    std::string sourceFile = generateTempFilename(lang);
    
    // Write code to file
    std::ofstream file(sourceFile);
    if (!file.is_open()) {
        logger->error("Failed to create source file: " + sourceFile);
        return "FAILED";
    }
    file << code;
    file.close();
    
    auto it = languageConfigs.find(lang);
    if (it == languageConfigs.end()) {
        logger->error("Language not supported");
        return "FAILED";
    }
    
    const LanguageConfig& langConfig = it->second;
    
    // Compile if necessary
    if (langConfig.needsCompilation) {
        if (!compileCode(sourceFile, lang)) {
            if (!keepFiles) {
                std::filesystem::remove(sourceFile);
            }
            return "COMPILATION_FAILED";
        }
    }
    
    // Execute
    std::string command = langConfig.runCommand;
    size_t pos = 0;
    while ((pos = command.find("{source}", pos)) != std::string::npos) {
        command.replace(pos, 8, sourceFile);
        pos += sourceFile.length();
    }
    
    // For compiled languages, use the output file
    if (langConfig.needsCompilation && lang != Language::GO) {
        std::string outputFile = sourceFile.substr(0, sourceFile.find_last_of('.'));
        command = outputFile;
    }
    
    logger->info("Executing " + langConfig.name + ": " + command);
    
    std::string output;
    bool success = executeCommand(command, output);
    
    // Cleanup
    if (!keepFiles) {
        std::filesystem::remove(sourceFile);
        if (langConfig.needsCompilation) {
            std::string outputFile = sourceFile.substr(0, sourceFile.find_last_of('.'));
            std::filesystem::remove(outputFile);
        }
    }
    
    if (success) {
        logger->info("Execution successful");
        return "SUCCESS";
    } else {
        logger->error("Execution failed: " + output);
        return "EXECUTION_FAILED";
    }
}

std::vector<Language> MultiLanguageExecutor::getAvailableLanguages() {
    std::vector<Language> available;
    
    for (const auto& pair : languageConfigs) {
        if (checkLanguageSupport(pair.first)) {
            available.push_back(pair.first);
        }
    }
    
    return available;
}

bool MultiLanguageExecutor::checkLanguageSupport(Language lang) {
    auto it = languageConfigs.find(lang);
    if (it == languageConfigs.end()) return false;
    
    const LanguageConfig& langConfig = it->second;
    std::string checkCmd;
    
    if (!langConfig.compiler.empty()) {
        checkCmd = langConfig.compiler + " --version";
    } else if (!langConfig.interpreter.empty()) {
        checkCmd = langConfig.interpreter + " --version";
    } else {
        return false;
    }
    
    std::string output;
    return executeCommand(checkCmd + " > /dev/null 2>&1", output);
}

bool MultiLanguageExecutor::executeCommand(const std::string& command, std::string& output) {
    std::string fullCommand = "cd \"" + workingDirectory + "\" && " + command + " 2>&1";
    
    FILE* pipe = popen(fullCommand.c_str(), "r");
    if (!pipe) {
        logger->error("Failed to execute command: " + command);
        return false;
    }
    
    char buffer[128];
    output.clear();
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    
    int exitCode = pclose(pipe);
    return exitCode == 0;
}

std::string MultiLanguageExecutor::generateTempFilename(Language lang) {
    static int counter = 0;
    auto it = languageConfigs.find(lang);
    std::string extension = (it != languageConfigs.end()) ? it->second.extension : ".txt";
    return workingDirectory + "/temp_" + std::to_string(++counter) + extension;
}

// ========== WebConnector Implementation ==========
WebConnector::WebConnector(std::shared_ptr<Logger> log) 
    : logger(log), curl(nullptr), userAgent("MultiLanguageBot/1.0"), maxRetries(3), timeoutSeconds(30) {
    initializeCurl();
}

WebConnector::~WebConnector() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void WebConnector::initializeCurl() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSeconds);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        logger->info("WebConnector initialized with curl");
    } else {
        logger->error("Failed to initialize curl");
    }
}

size_t WebConnector::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

HttpResponse WebConnector::httpGet(const std::string& url, const std::map<std::string, std::string>& headers) {
    HttpResponse response;
    response.success = false;
    response.responseCode = 0;
    
    if (!curl) {
        logger->error("Curl not initialized");
        return response;
    }
    
    std::string responseData;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    // Set headers
    struct curl_slist* headerList = nullptr;
    for (const auto& header : headers) {
        std::string headerStr = header.first + ": " + header.second;
        headerList = curl_slist_append(headerList, headerStr.c_str());
    }
    if (headerList) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }
    
    CURLcode res = curl_easy_perform(curl);
    
    if (headerList) {
        curl_slist_free_all(headerList);
    }
    
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.responseCode);
        response.data = responseData;
        response.success = (response.responseCode == 200);
        
        if (response.success) {
            logger->debug("HTTP GET successful: " + url);
        } else {
            logger->warning("HTTP GET failed with code " + std::to_string(response.responseCode) + ": " + url);
        }
    } else {
        logger->error("HTTP GET error: " + std::string(curl_easy_strerror(res)));
    }
    
    return response;
}

std::vector<WebResource> WebConnector::searchGitHub(const std::string& query, Language lang) {
    std::vector<WebResource> resources;
    
    std::string apiUrl = buildGitHubApiUrl(query, lang);
    HttpResponse response = httpGet(apiUrl);
    
    if (response.success) {
        try {
            Json::Value root;
            Json::Reader reader;
            
            if (reader.parse(response.data, root)) {
                const Json::Value& items = root["items"];
                
                for (const auto& item : items) {
                    WebResource resource;
                    resource.type = ResourceType::GITHUB_REPO;
                    resource.url = item["html_url"].asString();
                    resource.language = lang;
                    resource.relevanceScore = item["score"].asDouble();
                    resource.fetchedAt = std::chrono::system_clock::now();
                    
                    // Extract description and tags
                    resource.content = item["description"].asString();
                    if (item.isMember("topics")) {
                        for (const auto& topic : item["topics"]) {
                            resource.tags.push_back(topic.asString());
                        }
                    }
                    
                    resources.push_back(resource);
                }
                
                logger->info("Found " + std::to_string(resources.size()) + " GitHub repositories");
            }
        } catch (const std::exception& e) {
            logger->error("Failed to parse GitHub response: " + std::string(e.what()));
        }
    }
    
    return resources;
}

std::string WebConnector::buildGitHubApiUrl(const std::string& query, Language lang) {
    std::string baseUrl = "https://api.github.com/search/repositories";
    std::string langQuery = query + " language:" + languageToString(lang);
    
    // URL encode the query
    CURL* curl_handle = curl_easy_init();
    char* encoded = curl_easy_escape(curl_handle, langQuery.c_str(), langQuery.length());
    std::string encodedQuery(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl_handle);
    
    return baseUrl + "?q=" + encodedQuery + "&sort=stars&order=desc&per_page=10";
}

std::string WebConnector::languageToString(Language lang) {
    switch (lang) {
        case Language::CPP: return "cpp";
        case Language::PYTHON: return "python";
        case Language::JAVASCRIPT: return "javascript";
        case Language::RUST: return "rust";
        case Language::GO: return "go";
        case Language::JAVA: return "java";
        case Language::TYPESCRIPT: return "typescript";
        case Language::PHP: return "php";
        case Language::RUBY: return "ruby";
        case Language::SHELL: return "shell";
        default: return "unknown";
    }
}

// ========== MultiLanguageBot Implementation ==========
MultiLanguageBot::MultiLanguageBot() : currentLanguage(Language::CPP), internetEnabled(true), autoFetchResources(true) {
    // Initialize with basic languages
    enabledLanguages = {Language::CPP, Language::PYTHON, Language::JAVASCRIPT};
}

MultiLanguageBot::~MultiLanguageBot() {
    // Cleanup will be handled by base class and smart pointers
}

void MultiLanguageBot::initialize() {
    CodeGeneratorBot::initialize();
    
    logger->info("Initializing MultiLanguageBot...");
    
    initializeLanguageGenerators();
    setupInternetConnectivity();
    
    // Initialize multi-language executor
    multiExecutor = std::make_shared<MultiLanguageExecutor>(logger, config);
    
    // Check available languages
    auto availableLanguages = multiExecutor->getAvailableLanguages();
    logger->info("Available languages: " + std::to_string(availableLanguages.size()));
    
    for (Language lang : availableLanguages) {
        logger->info("  - " + getLanguageName(lang));
    }
    
    logger->info("MultiLanguageBot initialization complete");
}

void MultiLanguageBot::initializeLanguageGenerators() {
    for (Language lang : enabledLanguages) {
        generators[lang] = std::make_shared<LanguageCodeGenerator>(lang, logger);
    }
}

void MultiLanguageBot::setupInternetConnectivity() {
    if (internetEnabled) {
        webConnector = std::make_shared<WebConnector>(logger);
        packageManager = std::make_shared<PackageManager>(logger, webConnector);
        logger->info("Internet connectivity enabled");
    }
}

std::string MultiLanguageBot::generateCodeForLanguage(Language lang, const std::string& type, const std::string& name) {
    auto it = generators.find(lang);
    if (it == generators.end()) {
        logger->error("Language generator not found for " + getLanguageName(lang));
        return "";
    }
    
    auto generator = it->second;
    
    if (type == "class") {
        return generator->generateClass(name);
    } else if (type == "function") {
        return generator->generateFunction(name);
    } else if (type == "project") {
        return generator->generateProject(name);
    } else if (type == "webapp") {
        return generator->generateWebApp();
    } else if (type == "api") {
        return generator->generateAPI();
    } else if (type == "script") {
        return generator->generateScript(name);
    }
    
    return generator->generateProject();
}

void MultiLanguageBot::run() {
    initialize();
    start();
    
    std::cout << "\n🚀 Multi-Language CodeGeneratorBot is now running!\n";
    std::cout << "Supports " << enabledLanguages.size() << " programming languages with internet connectivity!\n\n";
    
    std::cout << "Enhanced Commands:\n";
    std::cout << "  lang <language>     - Switch to language (cpp/python/js/rust/go/java/etc.)\n";
    std::cout << "  langs               - Show available languages\n";
    std::cout << "  demo <language>     - Demonstrate language capabilities\n";
    std::cout << "  web <language>      - Generate web application\n";
    std::cout << "  api <language>      - Generate API server\n";
    std::cout << "  fetch <query>       - Fetch code examples from internet\n";
    std::cout << "  github <query>      - Search GitHub repositories\n";
    std::cout << "  project <type>      - Generate full project\n";
    std::cout << "  status              - Show bot status\n";
    std::cout << "  stop                - Shutdown bot\n\n";
    
    std::string command;
    while (running && std::getline(std::cin, command)) {
        if (command == "stop") {
            break;
        } else if (command == "status") {
            showLanguageStatus();
        } else if (command == "langs") {
            std::cout << "\nAvailable languages:\n";
            for (Language lang : enabledLanguages) {
                std::string marker = (lang == currentLanguage) ? " [CURRENT]" : "";
                std::cout << "  - " << getLanguageName(lang) << marker << "\n";
            }
            std::cout << "\n";
        } else if (command.find("lang ") == 0) {
            std::string langName = command.substr(5);
            // Parse language and switch
            if (langName == "cpp" || langName == "c++") {
                setCurrentLanguage(Language::CPP);
            } else if (langName == "python" || langName == "py") {
                setCurrentLanguage(Language::PYTHON);
            } else if (langName == "javascript" || langName == "js") {
                setCurrentLanguage(Language::JAVASCRIPT);
            } else if (langName == "rust") {
                setCurrentLanguage(Language::RUST);
            } else if (langName == "go") {
                setCurrentLanguage(Language::GO);
            } else if (langName == "java") {
                setCurrentLanguage(Language::JAVA);
            } else {
                std::cout << "Language not supported: " << langName << "\n";
            }
        } else if (command.find("demo ") == 0) {
            std::string langName = command.substr(5);
            // Demo language capabilities
            std::cout << "\n=== " << langName << " Demo ===\n";
            std::string code = generateCodeForLanguage(currentLanguage, "project");
            std::cout << code << "\n";
            std::cout << "=== End Demo ===\n\n";
        } else if (command.find("web ") == 0) {
            std::cout << "\n=== Generated Web Application ===\n";
            std::string code = generateCodeForLanguage(currentLanguage, "webapp");
            std::cout << code << "\n";
            std::cout << "=== End Web App ===\n\n";
        } else if (command.find("api ") == 0) {
            std::cout << "\n=== Generated API Server ===\n";
            std::string code = generateCodeForLanguage(currentLanguage, "api");
            std::cout << code << "\n";
            std::cout << "=== End API ===\n\n";
        } else {
            processMultiLanguageCommand(command);
        }
    }
    
    stop();
}

void MultiLanguageBot::setCurrentLanguage(Language lang) {
    currentLanguage = lang;
    
    // Enable language if not already enabled
    if (std::find(enabledLanguages.begin(), enabledLanguages.end(), lang) == enabledLanguages.end()) {
        enableLanguage(lang);
    }
    
    logger->info("Switched to " + getLanguageName(lang));
    std::cout << "Current language: " << getLanguageName(lang) << "\n";
}

void MultiLanguageBot::enableLanguage(Language lang) {
    if (std::find(enabledLanguages.begin(), enabledLanguages.end(), lang) == enabledLanguages.end()) {
        enabledLanguages.push_back(lang);
        generators[lang] = std::make_shared<LanguageCodeGenerator>(lang, logger);
        logger->info("Enabled " + getLanguageName(lang));
    }
}

void MultiLanguageBot::showLanguageStatus() {
    auto now = std::chrono::system_clock::now();
    auto runtime = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
    
    std::cout << "\n=== Multi-Language Bot Status ===\n";
    std::cout << "Running: " << (running ? "Yes" : "No") << "\n";
    std::cout << "Runtime: " << runtime << " seconds\n";
    std::cout << "Current Language: " << getLanguageName(currentLanguage) << "\n";
    std::cout << "Enabled Languages: " << enabledLanguages.size() << "\n";
    
    for (Language lang : enabledLanguages) {
        std::string status = multiExecutor->checkLanguageSupport(lang) ? "✓" : "✗";
        std::cout << "  " << status << " " << getLanguageName(lang) << "\n";
    }
    
    std::cout << "Internet: " << (internetEnabled ? "Enabled" : "Disabled") << "\n";
    std::cout << "Tasks completed: " << tasksCompleted << "\n";
    std::cout << "Tasks successful: " << tasksSuccessful << "\n";
    std::cout << "Success rate: " << (tasksCompleted > 0 ? (tasksSuccessful * 100.0 / tasksCompleted) : 0) << "%\n";
    std::cout << "Working directory: " << codeExecutor->getWorkingDirectory() << "\n";
    std::cout << "===============================\n\n";
}

void MultiLanguageBot::processMultiLanguageCommand(const std::string& command) {
    // Process other commands using the base class
    processUserCommand(command);
}

std::string MultiLanguageBot::getLanguageName(Language lang) {
    switch (lang) {
        case Language::CPP: return "C++";
        case Language::PYTHON: return "Python";
        case Language::JAVASCRIPT: return "JavaScript";
        case Language::RUST: return "Rust";
        case Language::GO: return "Go";
        case Language::JAVA: return "Java";
        case Language::TYPESCRIPT: return "TypeScript";
        case Language::PHP: return "PHP";
        case Language::RUBY: return "Ruby";
        case Language::SHELL: return "Shell";
        case Language::SQL: return "SQL";
        case Language::HTML_CSS: return "HTML/CSS";
        case Language::R: return "R";
        case Language::SCALA: return "Scala";
        case Language::PERL: return "Perl";
        case Language::LUA: return "Lua";
        case Language::DART: return "Dart";
        case Language::SWIFT: return "Swift";
        case Language::KOTLIN: return "Kotlin";
        case Language::CSHARP: return "C#";
        default: return "Unknown";
    }
}

// ========== PackageManager Implementation ==========
PackageManager::PackageManager(std::shared_ptr<Logger> log, std::shared_ptr<WebConnector> web)
    : logger(log), webConnector(web) {
    initializePopularPackages();
    logger->info("PackageManager initialized");
}

void PackageManager::initializePopularPackages() {
    popularPackages[Language::PYTHON] = {"requests", "numpy", "pandas", "flask", "django", "fastapi"};
    popularPackages[Language::JAVASCRIPT] = {"express", "react", "lodash", "axios", "moment", "socket.io"};
    popularPackages[Language::RUST] = {"serde", "tokio", "clap", "reqwest", "diesel", "actix-web"};
    popularPackages[Language::GO] = {"gin", "echo", "viper", "logrus", "testify", "gorm"};
    popularPackages[Language::JAVA] = {"spring-boot", "jackson", "junit", "slf4j", "guava", "commons-lang3"};
}

std::vector<std::string> PackageManager::getPopularPackages(Language lang) {
    auto it = popularPackages.find(lang);
    return (it != popularPackages.end()) ? it->second : std::vector<std::string>{};
}