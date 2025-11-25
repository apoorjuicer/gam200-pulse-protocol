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

//for .NET
#include "CoreEngine/Core/ImportExport.h" // for DLL_API
// stupid warning just saying to make sure we are using same compilier type and runtime MDd. WHICH WE ARE SO WHY WARNING STILL??????????
#pragma warning(push)
#pragma warning(disable: 4251)

class Factory;

struct PrefabData {
    std::string prefabName;
    std::string filePath;
};

class DLL_API PrefabManager
{
public:
    PrefabManager();   // Constructor
    ~PrefabManager();  // Destructor

    // save entity as prefab
    bool SaveEntityAsPrefab(Entity entity, std::string const& prefabName);

    // load prefab and create entity
    std::optional<Entity> InstantiatePrefab(std::string const& prefabName);

    // delete prefab from disk
    bool DeletePrefab(std::string const& prefabName);

    // get all available prefabs for the editor
    std::vector<PrefabData> GetAvailablePrefabs();

    // get prefab path by name
    std::string GetPrefabPath(std::string const& prefabName);

    // updates all entities that were instantiated from this prefab
    void UpdateAllInstancesOfPrefab(std::string const& prefabName);

private:
    std::string prefabDirectory = "../../MyEngine/Assets/Prefabs/";

    // updates a single entity from a prefab file (private helper)
    bool UpdateEntityFromPrefab(Entity entity, std::string const& prefabPath);

    // using Factory internally
    Factory* factory;
};

#pragma warning(pop)