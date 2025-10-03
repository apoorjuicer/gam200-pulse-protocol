/******************************************************************************/
/**
 * @file        ResourceManager.h
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Basic text resource manager for loading, caching, and managing
 *              text-based resources. Supports configuration files, game data,
 *              and provides the foundation for data-driven workflows. Includes
 *              utilities for reading, writing, and logging text resources.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without
 *              the prior written consent of DigiPen Institute of Technology
 *              is prohibited.
 */
 /******************************************************************************/


#pragma once

#include <string>
#include <unordered_map> // to fulfill the "Use STL in at least 2 code files.." req.
#include <fstream> // to fulfill the "Use STL in at least 2 code files.." req.

class ResourceManager {
private:
    // resource shelf
    std::unordered_map<std::string, std::string> textResources;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> configResources;

public:
    // load / get content from text file
    bool LoadTextResource(const std::string& name, const std::string& filename);
    std::string GetTextResource(const std::string& name);

    // load config file / get one value from config file
    bool LoadConfig(const std::string& configName, const std::string& filename);
    std::string GetConfigValue(const std::string& configName, const std::string& key);

    // resource loading / unloading (freeing)
    bool IsResourceLoaded(const std::string& name);
    void UnloadResource(const std::string& name);
    void UnloadAll();

    // write a string into file / add line or message to end of file
    static bool WriteTextFile(const std::string& filename, const std::string& content);
    static bool AppendToLog(const std::string& filename, const std::string& message);

private:
    // extra helper functions
    std::string ReadFile(const std::string& filename);
    std::unordered_map<std::string, std::string> ParseConfig(const std::string& content);
};