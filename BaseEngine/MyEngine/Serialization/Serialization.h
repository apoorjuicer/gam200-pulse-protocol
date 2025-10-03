/******************************************************************************/
/**
 * @file        Serialization.h
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin
 * @brief       Declares serialization and deserialization interfaces for engine
 *              configuration (Config) and entities (e.g., NPC). Provides
 *              save/load utility function prototypes for writing and reading
 *              JSON-based game data files.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without
 *              the prior written consent of DigiPen Institute of Technology is
 *              prohibited.
 */
 /******************************************************************************/

#pragma once
#include <pch/pch.h>
#include <string>
#include "../MyEngine/Core/Types.h" // ensures Entity/Signature are known  <-- (from Types.h)

class Coordinator; // forward-declare (global namespace)

class Config
{
private:
	float WindowWidth = 0;
	float WindowHeight = 0;
	std::string ConfigPath = "../../MyEngine/JSON/config.json";

public:
	float GetWindowWidth() const
	{
		return WindowWidth;
	}
	float GetWindowHeight() const
	{
		return WindowHeight;
	}
	std::string GetConfigPath() const
	{
		return ConfigPath;
	}

	void SetWindowWidth(float w) { WindowWidth = w; }
	void SetWindowHeight(float h) { WindowHeight = h; }
	void SetConfigPath(std::string path) { ConfigPath = std::move(path); }

	void Serialize(rapidjson::Value &out, rapidjson::Document::AllocatorType &alloc) const;
	void Deserialize(const rapidjson::Value &in);
};

bool SaveConfig(Config const &cfg, std::string const &path);
bool LoadConfig(Config &cfg, std::string const &path);

bool SaveEntity(Entity e, Coordinator *g, std::string const &path);
bool LoadEntity(Entity e, Coordinator *g, std::string const &path);

// bool SaveScene(const std::vector<Entity>& entities, Coordinator* g, const std::string& path);
// bool LoadScene(std::vector<Entity>& entities, Coordinator* g, const std::string& path);
