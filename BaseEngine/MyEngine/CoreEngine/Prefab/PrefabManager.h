/******************************************************************************/
/**
 * @file        PrefabManager.h
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Declares the PrefabManager class for managing reusable entity templates.
 *              Handles saving entities as prefabs, instantiating prefabs into scenes,
 *              and tracking available prefabs in the asset directory.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#pragma once
#include "pch/pch_temp.h"
#include "../ECS/Types.h"

class Factory;

struct PrefabData {
    std::string prefabName;
    std::string filePath;
};

class PrefabManager
{
public:
    PrefabManager();   // Constructor
    ~PrefabManager();  // Destructor

    // save entity as prefab
    bool SaveEntityAsPrefab(Entity entity, std::string const& prefabName);

    // load prefab and create entity
    std::optional<Entity> InstantiatePrefab(std::string const& prefabName);

    // get all available prefabs for the editor
    std::vector<PrefabData> GetAvailablePrefabs();

    // get prefab path by name
    std::string GetPrefabPath(std::string const& prefabName);

private:
    std::string prefabDirectory = "../../MyEngine/Assets/Prefabs/";
    
    // using Factory internally
    Factory* factory;
};