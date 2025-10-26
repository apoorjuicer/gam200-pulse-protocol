/******************************************************************************/
/**
 * @file        ResourceManager.cpp
 * @project     Pulse Protocol
 * @author		Ban Kai Wei Benjamin
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

namespace {
	void printSeparator(std::ostream& os) {
		os << "=============================\n";
	}
}

/*Audio Resource*/

void AudioResource::Deserialize(const rapidjson::Value& in_obj) {
	if (!in_obj.IsObject()) {
		std::cerr << "[AudioResource] Invalid JSON format\n";
		return;
	}

	//***Essential metadata, must exist and validated 
	if (in_obj.HasMember("id") && in_obj["id"].IsString()) { audio_id = in_obj["id"].GetString(); }
	if (in_obj.HasMember("path") && in_obj["path"].IsString()){ path = in_obj["path"].GetString(); }

	//***optional metadata, 
	if (in_obj.HasMember("loop") && in_obj["loop"].IsBool()) { 
		if (in_obj["loop"].IsBool())
			loop = in_obj["loop"].GetBool();
		else
			std::cerr << "[AudioResource] 'loop' has wrong type for " << audio_id << "\n";
	}
	else {
		std::cerr << "[AudioResource] Missing 'loop' field, using default (false) for " << audio_id << "\n";
	}
	if (in_obj.HasMember("volume") && in_obj["volume"].IsNumber()) { 
		if (in_obj["volume"].IsNumber())
			volume = static_cast<float>(in_obj["volume"].GetDouble());
		else
			std::cerr << "[AudioResource] 'volume' has wrong type for " << audio_id << "\n";
	}
	else {
		std::cerr << "[AudioResource] Missing 'volume' field, using default value (1.0) for " 
			<< audio_id << "\n";
	}
}
/*Texture Resource*/
void TextureResource::Deserialize(const rapidjson::Value& in_obj) {
	if (!in_obj.IsObject()) {
		std::cerr << "[TextureResource] Invalid JSON format\n";
		return;
	}

	//***Essential metadata, must exist and validated 
	if (in_obj.HasMember("id") && in_obj["id"].IsString()) { tex_id = in_obj["id"].GetString(); }
	if (in_obj.HasMember("path") && in_obj["path"].IsString()) { path = in_obj["path"].GetString(); }
}
/*Font Resource*/
void FontResource::Deserialize(const rapidjson::Value& in_obj) {
	if (!in_obj.IsObject()) {
		std::cerr << "[FontResource] Invalid JSON format\n";
		return;
	}

	//***Essential metadata, must exist and validated 
	if (in_obj.HasMember("id") && in_obj["id"].IsString()) { font_id = in_obj["id"].GetString(); }
	if (in_obj.HasMember("path") && in_obj["path"].IsString()) { path = in_obj["path"].GetString(); }
}

/*Resource Manager*/

/*accessor*/
std::unordered_map<std::string, FontResource> const& ResourceManager::getFontContainer() const {
	return fontResources;
}
std::unordered_map<std::string, AudioResource> const& ResourceManager::getAudioContainer() const {
	return audioResources;
}
std::unordered_map<std::string, TextureResource> const& ResourceManager::getTexContainer() const {
	return textureResources;
}

std::string const& ResourceManager::GetAudioJSONfilepath() const {
	return audiojsonfilepath;
}

std::string const& ResourceManager::GetTextureJSONfilepath() const {
	return texturejsonfilepath;
}

std::string const& ResourceManager::GetFontJSONfilepath() const {
	return fontjsonfilepath;
}

/*default cons for debug*/
//ResourceManager::ResourceManager() {
//	std::cout << "Resource Manager Initialised" << "\n";
//}

bool ResourceManager::LoadAudioManifest( std::string const& jsonPath) {
	std::ifstream ifs(jsonPath);
	if (!ifs.is_open()) {
		std::cerr << "Error: Cannot open file for reading: " << jsonPath << "\n";
		return false;
	}
	rapidjson::IStreamWrapper isw(ifs); 
	rapidjson::Document d;                  
	d.ParseStream(isw);                       
	if (d.HasParseError()) {
		std::cerr << "Error: Failed to parse JSON in file: " << jsonPath << "\n";
		return false;
	}

	if (!d.HasMember("audioFilePaths")) {
		std::cerr << "Error: No 'audioFilePaths' field in file: " << jsonPath << "\n";
		return false;
	}

	//get the array of json file
	auto const& arr = d["audioFilePaths"].GetArray();
	//variables to count successful parsing and failed parsing
	size_t added = 0, skipped = 0;

	for (const auto& entry : arr) {
		
		AudioResource aud_res;
		aud_res.Deserialize(entry);

	/*	if (!aud_res.audio_id.empty()) {
			audioResources.emplace(aud_res.audio_id, std::move(aud_res));
		}*/

		if (aud_res.audio_id.empty() || aud_res.path.empty()) {
			++skipped;
			std::cerr << "[Audio] Skipping entry with missing id/path\n";
			continue;
		}
		//insert into map assign into variable type
		//std::pair<std::unordered_map<std::string, AudioResource>::iterator, bool>
		//ok refers to successful flag
		auto [it, ok] = audioResources.emplace(aud_res.audio_id, std::move(aud_res));
		if (!ok) {
			++skipped;
			std::cerr << "[Audio] Duplicate id =" << it->first << "= error skipping\n";
		}
		else {
			++added;
		}
	}// end of for-range to load data into container


	std::cout << "No. of [Audio] Loaded :" << added << "\n"
			  << "No. item(s), skipped " << skipped << "\n"
		      << "Loaded from :" << jsonPath << "\n";

	return true;
}

bool ResourceManager::LoadTextureManifest( std::string const& jsonPath) {
	std::ifstream ifs(jsonPath);
	if (!ifs.is_open()) {
		std::cerr << "Error: Cannot open file for reading: " << jsonPath << "\n";
		return false;
	}
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document d;
	d.ParseStream(isw);
	if (d.HasParseError()) {
		std::cerr << "Error: Failed to parse JSON in file: " << jsonPath << "\n";
		return false;
	}

	if (!d.HasMember("textureFilePaths")) {
		std::cerr << "Error: No 'textureFilePaths' field in file: " << jsonPath << "\n";
		return false;
	}

	//get the array of json file
	auto const& arr = d["textureFilePaths"].GetArray();
	//variables to count successful parsing and failed parsing
	size_t added = 0, skipped = 0;

	for (const auto& entry : arr) {

		TextureResource tex_res;
		tex_res.Deserialize(entry);

		if (tex_res.tex_id.empty() || tex_res.path.empty()) {
			++skipped;
			std::cerr << "[Texture] Skipping entry with missing id/path\n";
			continue;
		}
		auto [it, ok] = textureResources.emplace(tex_res.tex_id, std::move(tex_res));
		if (!ok) {
			++skipped;
			std::cerr << "[Texture] Duplicate id =" << it->first << "= error skipping\n";
		}
		else {
			++added;
		}
	}// end of for-range to load data into container

	std::cout << "No. of [Texture] Loaded :" << added << "\n"
	<< "No. item(s), skipped " << skipped << "\n"
	<< "Loaded from :" << jsonPath << "\n";

	return true;
}



bool ResourceManager::LoadFontManifest(std::string const& jsonPath) {
	std::ifstream ifs(jsonPath);
	if (!ifs.is_open()) {
		std::cerr << "Error: Cannot open file for reading: " << jsonPath << "\n";
		return false;
	}
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::Document d;
	d.ParseStream(isw);
	if (d.HasParseError()) {
		std::cerr << "Error: Failed to parse JSON in file: " << jsonPath << "\n";
		return false;
	}

	if (!d.HasMember("fontFilePaths")) {
		std::cerr << "Error: No 'fontFilePaths' field in file: " << jsonPath << "\n";
		return false;
	}

	//get the array of json file
	auto const& arr = d["fontFilePaths"].GetArray();
	//variables to count successful parsing and failed parsing
	size_t added = 0, skipped = 0;

	for (const auto& entry : arr) {

		FontResource font_res;
		font_res.Deserialize(entry);

		if (font_res.font_id.empty() || font_res.path.empty()) {
			++skipped;
			std::cerr << "[Font] Skipping entry with missing id/path\n";
			continue;
		}
		auto [it, ok] = fontResources.emplace(font_res.font_id, std::move(font_res));
		if (!ok) {
			++skipped;
			std::cerr << "[Font] Duplicate id =" << it->first << "= error skipping\n";
		}
		else {
			++added;
		}
	}// end of for-range to load data into container

	std::cout << "No. of [Font] Loaded :" << added << "\n"
		<< "No. item(s), skipped " << skipped << "\n"
		<< "Loaded from :" << jsonPath << "\n";

	return true;
}

bool ResourceManager::LoadAudioManifest() {
	return LoadAudioManifest(audiojsonfilepath);
}
bool ResourceManager::LoadTextureManifest() {
	return LoadTextureManifest(texturejsonfilepath);
}
bool ResourceManager::LoadFontManifest() {
	return LoadFontManifest(fontjsonfilepath);
}


/*For Debug and Console Init print all container element*/
std::ostream& operator<<(std::ostream& os, ResourceManager const& rm) {

	os << "===Resource Manager Loaded: ===\n";
	os << "Audio:  \n";
	printSeparator(os);
	for (auto const& [id, aud] : rm.getAudioContainer()) {
		os << id << "->" << aud.path << "\n";
		os << "loop: " << aud.loop << "\n";
		os << "volume: " << aud.volume << "\n";
	}
	printSeparator(os);
	os << "Texture:  \n";
	printSeparator(os);
	for (auto const& [id, tex] : rm.getTexContainer()) {
		os << id << "->" << tex.path << "\n";
	}
	printSeparator(os);
	os << "Fonts: \n";
	printSeparator(os);
	for (auto const& [id, font] : rm.getFontContainer()) {
		os << id << "->" << font.path << "\n";
	}
	printSeparator(os);
	return os;
}