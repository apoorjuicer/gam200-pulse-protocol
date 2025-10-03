/******************************************************************************/
/**
 * @file        ResourceManager.cpp
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Implementation of a basic text resource manager with caching
 *              capabilities. Handles text and configuration file loading,
 *              provides data-driven access, and supports resource lifecycle
 *              management including unloading and logging utilities.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without
 *              the prior written consent of DigiPen Institute of Technology
 *              is prohibited.
 */
 /******************************************************************************/

#include "ResourceManager.h"
#include <iostream>
#include <sstream>

// ============================= TEXT RESOURCES ============================= //
bool ResourceManager::LoadTextResource(const std::string& name, const std::string& filename) {
    std::string content = ReadFile(filename);
    if (content.empty()) {
        std::cout << "[ResourceManager] Failed to load text resource: " << filename << std::endl;
        return false;
    }

    // saves the text in memory under given name
    textResources[name] = content;
    std::cout << "[ResourceManager] Loaded text resource '" << name << "' from " << filename << std::endl;
    return true;
}

std::string ResourceManager::GetTextResource(const std::string& name) {
    // looks up resources by name
    auto it = textResources.find(name);
    if (it != textResources.end()) {
        return it->second; // returns content that is found
    }

    std::cout << "[ResourceManager] Warning: Text resource '" << name << "' not found" << std::endl;
    return "";
}

// ============================= CONFIG RESOURCES =========================== //
bool ResourceManager::LoadConfig(const std::string& configName, const std::string& filename) {
    std::string content = ReadFile(filename);
    if (content.empty()) {
        std::cout << "[ResourceManager] Failed to load config: " << filename << std::endl;
        return false;
    }

    configResources[configName] = ParseConfig(content);
    std::cout << "[ResourceManager] Loaded config '" << configName << "' from " << filename << std::endl;
    return true;
}

std::string ResourceManager::GetConfigValue(const std::string& configName, const std::string& key) {
    auto configIt = configResources.find(configName);
    if (configIt != configResources.end()) {
        auto valueIt = configIt->second.find(key);
        if (valueIt != configIt->second.end()) {
            return valueIt->second;
        }
    }

    std::cout << "[ResourceManager] Warning: Config value '" << key << "' not found in '" << configName << "'" << std::endl;
    return "";
}

// ============================= RESOURCE MANAGEMENT ======================== //
bool ResourceManager::IsResourceLoaded(const std::string& name) {
    return textResources.find(name) != textResources.end();
}

void ResourceManager::UnloadResource(const std::string& name) {
    textResources.erase(name);
    configResources.erase(name);
    std::cout << "[ResourceManager] Unloaded resource: " << name << std::endl;
}

void ResourceManager::UnloadAll() {
    textResources.clear();
    configResources.clear();
    std::cout << "[ResourceManager] All resources unloaded" << std::endl;
}

// ============================= FILE UTILITIES ============================= //
bool ResourceManager::WriteTextFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    file << content;
    file.close();
    return true;
}

bool ResourceManager::AppendToLog(const std::string& filename, const std::string& message) {
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        return false;
    }
    file << message << std::endl;
    file.close();
    return true;
}

// ============================= HELPER FUNCS ============================ //
std::string ResourceManager::ReadFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }

    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }

    file.close();
    return content;
}

std::unordered_map<std::string, std::string> ResourceManager::ParseConfig(const std::string& content) {
    std::unordered_map<std::string, std::string> config;
    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty() || line[0] == '#') continue;

        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            config[key] = value;
        }
    }

    return config;
}