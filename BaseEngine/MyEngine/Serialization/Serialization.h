/******************************************************************************/
/**
 * @file        Serialization.h
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin
 * @brief       Declares serialization and deserialization interfaces for engine
 *              configuration (Config) and entities (e.g., NPC). Provides
 *              save/load utility function prototypes for writing and reading
 *              JSON-based game data files.
 *	            
 *			    c++ obj -> json text -> file
 * 
 * 
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without
 *              the prior written consent of DigiPen Institute of Technology is
 *              prohibited.
 */
 /******************************************************************************/

#pragma once
#//include <pch/pch.h>
//#include "../MyEngine/CoreEngine/ECS/Types.h" // ensures Entity/Signature are known  <-- (from Types.h)
#include <rapidjson/ostreamwrapper.h>
//#include <rapidjson/document.h>     // for Document, Value, AllocatorType in pch tmp
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h> // for StringBuffer
#include "pch/pch_temp.h"


class Config
{
private:
	int WindowWidth = 0;
	int WindowHeight = 0;
	std::string ConfigPath = "../../MyEngine/JSON/config.json";
public:
	int GetWindowWidth() const;
	int GetWindowHeight() const;
	std::string GetConfigPath() const;
	void SetWindowWidth(int w);
	void SetWindowHeight(int h);
	void SetConfigPath(std::string path);
	void Serialize(rapidjson::Value &out, rapidjson::Document::AllocatorType &alloc) const;
	void Deserialize(const rapidjson::Value &in);
};

bool SaveConfig(Config const &cfg, std::string const &path, bool pretty = true);

//bool SaveConfig(Config const& cfg) {
//	return SaveConfig(cfg, cfg.GetConfigPath());
//}
//bool SaveConfig(Config const& cfg, std::string const& path);
bool LoadConfig(Config &cfg, std::string const &path);

namespace JSONUtils {
	using namespace rapidjson;
    //using rapidjson::Document;
    //using rapidjson::Value;
    //using rapidjson::Writer;
    //using rapidjson::PrettyWriter;
    //using rapidjson::StringBuffer;
    //using rapidjson::SizeType;
    //using rapidjson::kObjectType;

    //bool pretty means use PrettyWriter or just Writer
	//PrettyWriter takes more file space but more readable
	//Writer makes everything one liner
	std::string JSONToString(Document const& d, bool pretty);

    template <typename T>
	Document MakeObjDocument(T const& object, std::string const& memberName) {
		Document d(kObjectType);
		auto& alloc = d.GetAllocator();

		Value Val(kObjectType);
		object.Serialize(Val, alloc);
		//void AddMember(Value& name, Value& value, Allocator& allocator);
		//instead of d.AddMember("transform", tVal, alloc);
		//we use like this

		//same thing more constructor call
	   //Value key(memberName.c_str(), static_cast<SizeType>(memberName.size()), alloc);
	   // d.AddMember(key,Val, alloc);

		d.AddMember(
			Value(memberName.c_str(), static_cast<SizeType>(memberName.size()), alloc),
			Val,
			alloc);

		return d; // NRVO (construct in caller) or move if optimization skipped
	}

    //bool pretty means use PrettyWriter or just Writer
    //PrettyWriter takes more file space but more readable
    //Writer makes everything one liner
    bool WriteJSONToFile(Document const& d, std::filesystem::path const& path, bool pretty);


}//end of namespace JSONUtils