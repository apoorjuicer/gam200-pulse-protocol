/******************************************************************************/
/**
 * @file        AssetRegistry.h
 * @project     Pulse Protocol
 * @author		Ban Kai Wei Benjamin
 * @author      Chloe Lau Rey En
 * @brief       Implementation of a basic asset manager
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without
 *              the prior written consent of DigiPen Institute of Technology
 *              is prohibited.
 */
 /******************************************************************************/


#pragma once

//#include <string>
//#include <unordered_map> 
//#include <fstream>
//#include <iostream>
#include "pch/pch_temp.h"
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

struct SceneResource {
	std::string scene_id = "";
	std::string path = "";

	void Deserialize(rapidjson::Value const& in);
};

class AssetRegistry {
private:
	//[key,value]
	std::unordered_map<std::string, AudioResource> audioResources;
	std::unordered_map<std::string, TextureResource> textureResources;
	std::unordered_map<std::string, FontResource> fontResources;
	std::unordered_map<std::string, SceneResource> sceneResources;

	//file paths
	std::string audiojsonfilepath = "../../MyEngine/JSON/audio.json";
	std::string texturejsonfilepath = "../../MyEngine/JSON/texture.json";
	std::string fontjsonfilepath = "../../MyEngine/JSON/font.json";
	std::string scenejsonfilepath = "../../MyEngine/JSON/scene.json";

public:
	//AssetRegistry(); for debugging shouldn't use. due to Rule of Zero
	std::unordered_map<std::string, AudioResource> const& getAudioContainer() const;
	std::unordered_map<std::string, TextureResource> const& getTexContainer() const;
	std::unordered_map<std::string, FontResource> const& getFontContainer() const;
	std::unordered_map<std::string, SceneResource> const& getSceneContainer() const;

	std::string const& GetAudioJSONfilepath() const;
	std::string const& GetTextureJSONfilepath() const;
	std::string const& GetFontJSONfilepath() const;
	std::string const& GetSceneJSONfilepath() const;

	//findXXX use like this rm.FindXXX(id), e.g. rm.FindAudio("bgm");
	AudioResource  const* FindAudio(std::string const& id)  const;
	TextureResource const* FindTexture(std::string const& id) const;
	FontResource   const* FindFont(std::string const& id)   const;
	SceneResource   const* FindScene(std::string const& id)   const;

	//A manifest describes
	//existing assets(id,path)
	//metadata(loop,volume)
	//but not actual sound data itself
	bool LoadAudioManifest( std::string const& jsonPath);
	bool LoadTextureManifest( std::string const& jsonPath);
	bool LoadFontManifest( std::string const& jsonPath);
	bool LoadSceneManifest(std::string const& jsonPath);

	//taking overloaded function!! DO NOT REMOVE
	bool LoadAudioManifest();
	bool LoadTextureManifest();
	bool LoadFontManifest();
	bool LoadSceneManifest();
};

//ODR
inline std::ostream& operator<<(std::ostream& os, AssetRegistry const& rm);

