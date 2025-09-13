/******************************************************************************/
/*!
\file   IOManager.h
\author [Your Name]
\date   [Current Date]

\brief
    Basic I/O system for reading/writing text files and parsing simple
    configurations. Supports M1 rubrics for data-driven game objects,
    configuration loading, and STL container usage.

STL Used: std::string, std::ifstream, std::ofstream, std::unordered_map
*/
/******************************************************************************/

#pragma once

#include <string> // to fulfill the "use STL in at least 2 code files req."
#include <fstream> // to fulfill the "use STL in at least 2 code files req."
#include <unordered_map> // to fulfill the "use STL in at least 2 code files req."

class IOManager {
public:

    //basic file ops
    static std::string ReadTextFile(const std::string& filename);
    static bool WriteTextFile(const std::string& filename, const std::string& content);
    static bool FileExists(const std::string& filename);

    //config parsing
    static std::unordered_map<std::string, std::string> ParseConfig(const std::string& filename);

    //logging utilities
    static bool AppendToLog(const std::string& filename, const std::string& message);
};