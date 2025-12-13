/******************************************************************************/
/**
 * @file        SnapShot.h
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin
 * @brief       Saves an Entity before deleting it. Will be used to retrieve
 *              Entity's components
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#pragma once
#include "Graphics/Transform.h"
#include "Graphics/Renderable.h"
#include "Graphics/Animation.h"

#include "CoreEngine/Assets.h"                //NPC
#include "Components/AABB.h"                  //AABB
#include "Components/Health.h"                //Health
#include "Components/DisplayName.h"           //Name
#include "Graphics/Layer.h"                   //LayerTag
#include "Components/EntityType.h"            //NamingComponent
#include "GameLogic/GameLogic.h"              //LogicComponent
#include "Audio/AudioSource.h"                //AudioSource
#include "Graphics/Text.h"                    //TextComponent
#include "CoreEngine/Prefab/PrefabInstance.h" //prefab instance
#include "Editor.h"
namespace PulseEditor
{
    // struct
    struct EntitySnapshot
    {
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
        bool hasPrefabInstance = false;

        // Actual component data
        Framework::Transform transform_snap{};
        Framework::Renderable renderable_snap{};
        Framework::Animation animation_snap{};
        NPC npc_snap{};
        AABB aabb_snap{};
        Health health_snap{};
        Name name_snap{};
        LayerTag layertag_snap{};
        NamingComponent namingcomponent_snap{};
        LogicComponent logiccomponent_snap{};
        AudioSource audiosource_snap{};
        TextComponent text_snap{};
        PrefabInstance prefabinstance_snap{};
    };
    // make a "snapshot" or "copy" of entity
    EntitySnapshot MakeEntitySnapshot(Entity e);
    Entity CreateEntityFromEntitySnapshot(EntitySnapshot const &snap);
    // void printsnapshot(EntitySnapshot snap);
}