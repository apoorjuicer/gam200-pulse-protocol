/******************************************************************************/
/**
 * @file            Serialization.cpp
 * @project         Pulse Protocol
 * @author          Ban Kai Wei Benjamin
 * @brief               This file manages serialization and deserialization of config
 *                          and entity
 * @copyright     Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#include <pch/pch.h>

void Config::Serialize(rapidjson::Value &out, rapidjson::Document::AllocatorType &alloc) const
{
    using namespace rapidjson;
    out.SetObject();

    out.AddMember("WindowWidth", GetWindowWidth(), alloc);
    out.AddMember("WindowHeight", GetWindowHeight(), alloc);
    Value pathVal;
    pathVal.SetString(GetConfigPath().c_str(),
                      static_cast<SizeType>(GetConfigPath().length()),
                      alloc);
    out.AddMember("ConfigPath", pathVal, alloc);
}
void Config::Deserialize(const rapidjson::Value &in)
{
    if (!in.IsObject())
    {
        throw std::runtime_error("Invalid JSON for Config");
    }

    if (in.HasMember("WindowWidth") && in["WindowWidth"].IsNumber())
    {
        SetWindowWidth(in["WindowWidth"].GetFloat());
    }
    if (in.HasMember("WindowHeight") && in["WindowHeight"].IsNumber())
    {
        SetWindowHeight(in["WindowHeight"].GetFloat());
    }
    if (in.HasMember("ConfigPath") && in["ConfigPath"].IsString())
    {
        SetConfigPath(in["ConfigPath"].GetString());
    }
}

bool SaveConfig(Config const &config, std::string const &path)
{
    rapidjson::Document d;
    d.SetObject();
    auto &alloc = d.GetAllocator();

    rapidjson::Value configVal;
    config.Serialize(configVal, alloc);
    d.AddMember("config", configVal, alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);

    // Ensure directory exists
    namespace fs = std::filesystem;
    fs::create_directories("../../MyEngine/JSON");
    // fs::create_directories("../JSON");

    std::ofstream ofs(path);
    if (!ofs.is_open())
    {
        std::cerr << "Error: Cannot open file for writing: " << path << "\n";
        return false;
    }
    ofs << buffer.GetString();
    if (!ofs.good())
    {
        std::cerr << "Error: Failed to write JSON to file: " << path << "\n";
        return false;
    }
    return true;
}
bool LoadConfig(Config &config, std::string const &path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open())
    {
        std::cerr << "Error: Cannot open file for reading: " << path << "\n";
        return false;
    }
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document d;
    d.ParseStream(isw);

    if (d.HasParseError())
    {
        std::cerr << "Error: Failed to parse JSON in file: " << path << "\n";
        return false;
    }

    if (!d.HasMember("config"))
    {
        std::cerr << "Error: No -config- field in file : " << path << "\n";
        return false;
    }

    config.Deserialize(d["config"]);
    return true;
}

bool SaveEntity(Entity e, Coordinator *g, std::string const &path)
{
    using namespace rapidjson;
    Document d;
    d.SetObject();
    auto &alloc = d.GetAllocator();

    // NPC
    if (g->HasComponent<NPC>(e))
    {
        // Value npcVal;
        Value npcVal(kObjectType);
        g->GetComponent<NPC>(e).Serialize(npcVal, alloc);
        d.AddMember("NPC", npcVal, alloc);
    }

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    d.Accept(writer);
    namespace fs = std::filesystem;
    fs::create_directories("../../MyEngine/JSON");
    // fs::create_directories("../JSON");

    std::ofstream ofs(path);
    if (!ofs.is_open())
    {
        std::cerr << "Error: Cannot open file for writing: " << path << "\n";
        return false;
    }
    ofs << buffer.GetString();

    if (!ofs.good())
    {
        std::cerr << "Error: Failed to write JSON to file: " << path << "\n";
        return false;
    }

    return true;
}

bool LoadEntity(Entity e, Coordinator *g, std::string const &path)
{
    using namespace rapidjson;
    std::ifstream ifs(path);
    if (!ifs.is_open())
        return false;

    IStreamWrapper isw(ifs);
    Document d;
    d.ParseStream(isw);

    if (!d.IsObject())
        return false;

    if (d.HasMember("NPC"))
    {
        auto &npc = g->GetComponent<NPC>(e);
        npc.Deserialize(d["NPC"]);
    }
    /*    if (d.HasMember("AABB")) {
            auto& aabb = g->GetComponent<AABB>(e);
            aabb.Deserialize(d["AABB"]);
        }*/

    return true;
}
