/******************************************************************************/
/**
 * @file        SnapShot.h
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin
 * @brief       contains EntitySnapshot to "snapshot" entity before deleting
 *
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#pragma once
#include "../MyEngine/Graphics/Transform.h"
#include "../MyEngine/Graphics/Renderable.h"
#include "../MyEngine/Graphics/Animation.h"

#include "../MyEngine/CoreEngine/Assets.h" //NPC
#include "../MyEngine/Components/AABB.h" //AABB
#include "../MyEngine/Components/Health.h" //Health
#include "../MyEngine/Components/DisplayName.h" //Name
#include "../MyEngine/Graphics/Layer.h" //LayerTag
#include "../MyEngine/Components/EntityType.h" //NamingComponent
#include "../MyEngine/GameLogic/GameLogic.h" //LogicComponent
#include "../MyEngine/Audio/AudioSource.h" //AudioSource
#include "../MyEngine/Graphics/Text.h" //TextComponent


namespace MyEditor
{
    //struct
    struct EntitySnapshot {
        Entity originalEntity = INVALID_ENTITY; // for debugging
        // Which components existed
        bool hasTransform = false;
        bool hasRenderable = false;
        bool hasAnimation = false;
        bool hasNPC = false;
        bool hasAABB = false;
        bool hasHealth = false;
        bool hasName = false;
        bool hasNameComponent = false;
        bool hasLogicComponent = false;
        bool hasLayerTag = false;
        bool hasAudioSource = false;
        bool hasTextComponent = false;


        // Actual component data (deep copy)
        Framework::Transform transform_snap{};
        Framework::Renderable           renderable_snap{};
        Framework::Animation            animation_snap{};

        NPC npc_snap{};
        AABB aabb_snap{};
        Health health_snap{};
        Name  name_snap{};
        LayerTag layertag_snap{};
        NamingComponent namingcomponent_snap{};
        LogicComponent logiccomponent_snap{};
        AudioSource audiosource_snap{};
        TextComponent text_snap;

    };
    //make a "snapshot" or "copy" of entity
    EntitySnapshot MakeEntitySnapshot(Entity e);
    Entity CreateEntityFromEntitySnapshot(EntitySnapshot const& snap);
   
}