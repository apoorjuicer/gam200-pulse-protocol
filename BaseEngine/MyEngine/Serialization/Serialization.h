
#pragma once
#include <pch/pch.h>
#include <string>
#include "../MyEngine/Core/Types.h"        // ensures Entity/Signature are known  <-- (from Types.h)

class Coordinator;          // forward-declare (global namespace)


    bool SaveEntity(Entity e, Coordinator* g, std::string const& path);
    bool LoadEntity(Entity e, Coordinator* g, std::string const& path);

   // bool SaveScene(const std::vector<Entity>& entities, Coordinator* g, const std::string& path);
   // bool LoadScene(std::vector<Entity>& entities, Coordinator* g, const std::string& path);
