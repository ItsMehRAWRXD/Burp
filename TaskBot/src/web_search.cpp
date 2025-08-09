#include "../include/ide_integration.h"
#include "../include/task_bot.h"
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <json/json.h>
#include <regex>

namespace TaskBot {

// Callback for CURL to write response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

WebSearchEngine::WebSearchEngine() : maxResults_(10) {
    std::cout << "[WebSearchEngine] Initializing web search engine..." << std::endl;
    userAgent_ = "TaskBot/1.0 (Linux; AutoPilot Integration)";
    
    // Initialize CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

WebSearchEngine::~WebSearchEngine() {
    curl_global_cleanup();
}

void WebSearchEngine::setApiKey(const std::string& apiKey) {
    apiKey_ = apiKey;
    std::cout << "[WebSearchEngine] API key set" << std::endl;
}

void WebSearchEngine::setSearchEngineId(const std::string& engineId) {
    searchEngineId_ = engineId;
    std::cout << "[WebSearchEngine] Search engine ID set: " << engineId << std::endl;
}

void WebSearchEngine::setMaxResults(int maxResults) {
    maxResults_ = maxResults;
    std::cout << "[WebSearchEngine] Max results set to: " << maxResults << std::endl;
}

std::vector<SearchResult> WebSearchEngine::search(const std::string& query) {
    std::cout << "[WebSearchEngine] Searching for: \"" << query << "\"" << std::endl;
    
    std::map<std::string, std::string> params;
    params["q"] = query;
    params["num"] = std::to_string(maxResults_);
    
    std::string url = buildSearchUrl(query, params);
    std::string response = performHttpRequest(url);
    
    return parseSearchResponse(response);
}

std::vector<SearchResult> WebSearchEngine::searchCode(const std::string& query, const std::string& language) {
    std::cout << "[WebSearchEngine] Searching for code: \"" << query << "\" in " << language << std::endl;
    
    std::string enhancedQuery = query;
    if (!language.empty()) {
        enhancedQuery += " language:" + language;
    }
    enhancedQuery += " site:stackoverflow.com OR site:github.com OR site:gist.github.com";
    
    return search(enhancedQuery);
}

std::vector<SearchResult> WebSearchEngine::searchDocumentation(const std::string& query, const std::string& framework) {
    std::cout << "[WebSearchEngine] Searching documentation for: \"" << query << "\"" << std::endl;
    
    std::string enhancedQuery = query + " documentation reference guide";
    if (!framework.empty()) {
        enhancedQuery += " " + framework;
    }
    
    return search(enhancedQuery);
}

std::vector<SearchResult> WebSearchEngine::searchStackOverflow(const std::string& query) {
    std::cout << "[WebSearchEngine] Searching StackOverflow for: \"" << query << "\"" << std::endl;
    
    std::string enhancedQuery = query + " site:stackoverflow.com";
    return search(enhancedQuery);
}

std::vector<SearchResult> WebSearchEngine::searchGitHub(const std::string& query, const std::string& language) {
    std::cout << "[WebSearchEngine] Searching GitHub for: \"" << query << "\"" << std::endl;
    
    std::string enhancedQuery = query + " site:github.com";
    if (!language.empty()) {
        enhancedQuery += " language:" + language;
    }
    
    return search(enhancedQuery);
}

std::vector<CodeSuggestion> WebSearchEngine::extractCodeFromResults(const std::vector<SearchResult>& results) {
    std::cout << "[WebSearchEngine] Extracting code from " << results.size() << " search results" << std::endl;
    
    std::vector<CodeSuggestion> suggestions;
    
    for (const auto& result : results) {
        // Simple code extraction - look for code blocks in snippets
        std::regex codeRegex("```([^`]*)```|<code>([^<]*)</code>|<pre>([^<]*)</pre>");
        std::smatch matches;
        std::string content = result.snippet;
        
        while (std::regex_search(content, matches, codeRegex)) {
            CodeSuggestion suggestion;
            suggestion.code = matches[1].str();
            if (suggestion.code.empty()) suggestion.code = matches[2].str();
            if (suggestion.code.empty()) suggestion.code = matches[3].str();
            
            suggestion.sourceUrl = result.url;
            suggestion.explanation = result.title;
            suggestion.confidence = result.relevance * 0.8; // Adjust confidence based on source
            
            // Try to detect language
            if (result.url.find("github.com") != std::string::npos) {
                suggestion.language = "various";
                suggestion.confidence *= 1.1;
            } else if (result.url.find("stackoverflow.com") != std::string::npos) {
                suggestion.confidence *= 1.2;
            }
            
            suggestions.push_back(suggestion);
            content = matches.suffix().str();
        }
    }
    
    std::cout << "[WebSearchEngine] Extracted " << suggestions.size() << " code suggestions" << std::endl;
    return suggestions;
}

std::string WebSearchEngine::buildSearchUrl(const std::string& query, const std::map<std::string, std::string>& params) {
    // Using Google Custom Search API
    std::string baseUrl = "https://www.googleapis.com/customsearch/v1";
    
    std::stringstream url;
    url << baseUrl << "?";
    
    // Add API key and search engine ID
    url << "key=" << apiKey_ << "&";
    url << "cx=" << searchEngineId_ << "&";
    
    // Add query parameters
    for (const auto& param : params) {
        CURL* curl = curl_easy_init();
        if (curl) {
            char* encoded = curl_easy_escape(curl, param.second.c_str(), param.second.length());
            url << param.first << "=" << encoded << "&";
            curl_free(encoded);
            curl_easy_cleanup(curl);
        }
    }
    
    std::string finalUrl = url.str();
    if (!finalUrl.empty() && finalUrl.back() == '&') {
        finalUrl.pop_back();
    }
    
    return finalUrl;
}

std::string WebSearchEngine::performHttpRequest(const std::string& url) {
    std::cout << "[WebSearchEngine] Performing HTTP request..." << std::endl;
    
    CURL* curl = curl_easy_init();
    std::string response;
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent_.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            std::cerr << "[WebSearchEngine] CURL error: " << curl_easy_strerror(res) << std::endl;
        } else {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            std::cout << "[WebSearchEngine] HTTP response code: " << http_code << std::endl;
        }
        
        curl_easy_cleanup(curl);
    }
    
    return response;
}

std::vector<SearchResult> WebSearchEngine::parseSearchResponse(const std::string& response) {
    std::cout << "[WebSearchEngine] Parsing search response..." << std::endl;
    
    std::vector<SearchResult> results;
    
    // Parse JSON response
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    std::istringstream stream(response);
    
    if (!Json::parseFromStream(builder, stream, &root, &errors)) {
        std::cerr << "[WebSearchEngine] JSON parse error: " << errors << std::endl;
        return results;
    }
    
    // Extract search results
    if (root.isMember("items") && root["items"].isArray()) {
        const Json::Value& items = root["items"];
        
        for (const auto& item : items) {
            SearchResult result;
            
            if (item.isMember("title")) {
                result.title = item["title"].asString();
            }
            
            if (item.isMember("link")) {
                result.url = item["link"].asString();
            }
            
            if (item.isMember("snippet")) {
                result.snippet = item["snippet"].asString();
            }
            
            if (item.isMember("displayLink")) {
                result.source = item["displayLink"].asString();
            }
            
            // Calculate relevance (simple scoring based on position)
            result.relevance = 1.0 - (results.size() * 0.1);
            
            results.push_back(result);
            
            std::cout << "[WebSearchEngine] Found result: " << result.title << std::endl;
        }
    }
    
    std::cout << "[WebSearchEngine] Parsed " << results.size() << " results" << std::endl;
    return results;
}

} // namespace TaskBot