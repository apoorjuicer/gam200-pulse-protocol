/******************************************************************************/
/*!
\file   IOManager.cpp
\author [Your Name]
\date   [Current Date]

\brief
    Implementation of basic file I/O operations. Provides text file reading,
    writing, and simple key=value config parsing for game engine systems.

Functions:
    - ReadTextFile: Load entire file as string
    - WriteTextFile: Save string to file
    - ParseSimpleConfig: Load key=value pairs from config files
*/
/******************************************************************************/

#include "pch/pch.h"
#include <ostream>
#include <sstream>

//basic file ops
std::string IOManager::ReadTextFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error: Could not open file " << filename << std::endl;
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

bool IOManager::WriteTextFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error: Could not create file " << filename << std::endl;
        return false;
    }

    file << content;
    file.close();
    return true;
}

bool IOManager::FileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good(); // .good() checks whether the state of stream is good or not
}

//config parsing
std::unordered_map<std::string, std::string> IOManager::ParseConfig(const std::string& filename) {
    std::unordered_map<std::string, std::string> config;
    std::string content = ReadTextFile(filename);

    if (content.empty()) {
        return config;
    }

    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line)) {
        // if line is empty or is a comment, it gets skipped
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            config[key] = value;
        }
    }

    return config;
}

//logging utilities
bool IOManager::AppendToLog(const std::string& filename, const std::string& message) {
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        return false;
    }

    file << message << std::endl;
    file.close();
    return true;
}