/******************************************************************************/
/**
 * @file        ResourceManager.h
 * @project     Pulse Protocol
 * @author		Ban Kai Wei Benjamin
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
#include <unordered_map> 
#include <fstream>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

struct AudioResource {
	std::string audio_id = "";
	std::string path = "";
	bool loop = false;
	float volume = 1.0;

	void Deserialize( rapidjson::Value const& in);
};

struct TextureResource {
	std::string tex_id = "";
	std::string path = "";
	/*GLuint id = 0; int w = 0; int h = 0;*/

	void Deserialize( rapidjson::Value const& in);
};

struct FontResource {
	std::string font_id = "";
	std::string path = "";
	//may want font size

	void Deserialize( rapidjson::Value const& in);
};

class ResourceManager {
private:
	//[key,value]
	std::unordered_map<std::string, AudioResource> audioResources;
	std::unordered_map<std::string, TextureResource> textureResources;
	std::unordered_map<std::string, FontResource> fontResources;

	//file paths
	std::string audiojsonfilepath = "../../MyEngine/JSON/audio.json";
	std::string texturejsonfilepath = "../../MyEngine/JSON/texture.json";
	std::string fontjsonfilepath = "../../MyEngine/JSON/font.json";

public:
	//ResourceManager(); for debugging shouldn't use. due to Rule of Zero
	std::unordered_map<std::string, AudioResource> const& getAudioContainer() const;
	std::unordered_map<std::string, TextureResource> const& getTexContainer() const;
	std::unordered_map<std::string, FontResource> const& getFontContainer() const;

	std::string const& GetAudioJSONfilepath() const;
	std::string const& GetTextureJSONfilepath() const;
	std::string const& GetFontJSONfilepath() const;


	//A manifest describes
	//existing assets(id,path)
	//metadata(loop,volume)
	//but not actual sound data itself
	bool LoadAudioManifest( std::string const& jsonPath);
	bool LoadTextureManifest( std::string const& jsonPath);
	bool LoadFontManifest( std::string const& jsonPath);

	//taking overloaded function!! DO NOT REMOVE
	bool LoadAudioManifest();
	bool LoadTextureManifest();
	bool LoadFontManifest();
};

std::ostream& operator<<(std::ostream& os, ResourceManager const& rm);

namespace {
	void printSeparator(std::ostream& os); 
}