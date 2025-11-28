/******************************************************************************/
/**
 * @file        SnapShot.cpp
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin
 * @brief       contains function to "snapshot" save an entity before deleting
 *              so we can restore the entity when we undo
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#include "SnapShot.h"
#include "CoreEngine/Factory/Factory.h"

namespace MyEditor
{
    EntitySnapshot MakeEntitySnapshot(Entity e)
    {
        EntitySnapshot snap{};
        snap.originalEntity = e;

        auto* g = Coordinator::GetInstance();
        if (!g || e == INVALID_ENTITY)
            return snap;
        if (g->HasComponent<Framework::Transform>(e))
        {
            
            snap.hasTransform = true;
            snap.transform_snap = g->GetComponent<Framework::Transform>(e);
        }

        if (g->HasComponent<Framework::Renderable>(e))
        {
            snap.hasRenderable = true;
            snap.renderable_snap = g->GetComponent<Framework::Renderable>(e);
        }

        if (g->HasComponent<Framework::Animation>(e))
        {
            snap.hasAnimation = true;
            snap.animation_snap = g->GetComponent<Framework::Animation>(e);
        }

        if (g->HasComponent<NPC>(e))
        {
            snap.hasNPC = true;
            snap.npc_snap = g->GetComponent<NPC>(e);
        }

        if (g->HasComponent<AABB>(e))
        {
            snap.hasAABB = true;
            snap.aabb_snap = g->GetComponent<AABB>(e);
        }

        if (g->HasComponent<Health>(e))
        {
            snap.hasHealth = true;
            snap.health_snap = g->GetComponent<Health>(e);
        }

        if (g->HasComponent<Name>(e))
        {
            snap.hasName = true;
            snap.name_snap = g->GetComponent<Name>(e);
        }

        if (g->HasComponent<NamingComponent>(e))
        {
            snap.hasNameComponent = true;
            snap.namingcomponent_snap = g->GetComponent<NamingComponent>(e);
        }

        if (g->HasComponent<LogicComponent>(e))
        {
            snap.hasLogicComponent = true;
            snap.logiccomponent_snap = g->GetComponent<LogicComponent>(e);
        }

        if (g->HasComponent<LayerTag>(e))
        {
            snap.hasLayerTag = true;
            snap.layertag_snap = g->GetComponent<LayerTag>(e);
        }

        if (g->HasComponent<AudioSource>(e))
        {
            snap.hasAudioSource = true;
            snap.audiosource_snap = g->GetComponent<AudioSource>(e);
        }

        if (g->HasComponent<TextComponent>(e))
        {
            snap.hasTextComponent = true;
            snap.text_snap = g->GetComponent<TextComponent>(e);
        }

        if (g->HasComponent<PrefabInstance>(e))
        {
            snap.hasPrefabInstance = true;
            snap.prefabinstance_snap = g->GetComponent<PrefabInstance>(e);
        }

        return snap;
    }
    Entity CreateEntityFromEntitySnapshot(EntitySnapshot const& snap)
    {
        auto* g = Coordinator::GetInstance();
        if (!g)
            return INVALID_ENTITY;

        // Restore using ORIGINAL ID
        Entity e = g->CreateEntityWithID(snap.originalEntity);

        // If the ID cannot be revived, bail out
        if (e >= MaxEntity)      // ECS-side "invalid"
            return INVALID_ENTITY; // editor-side "invalid"

        // Now overwrite / add components using the snapshot
        if (snap.hasTransform)
        {
            if (g->HasComponent<Framework::Transform>(e))
                g->GetComponent<Framework::Transform>(e) = snap.transform_snap;
            else
                g->AddComponent<Framework::Transform>(e, snap.transform_snap);
        }

        if (snap.hasRenderable)
        {
            if (g->HasComponent<Framework::Renderable>(e))
                g->GetComponent<Framework::Renderable>(e) = snap.renderable_snap;
            else
                g->AddComponent<Framework::Renderable>(e, snap.renderable_snap);
        }

        if (snap.hasAnimation)
        {
            if (g->HasComponent<Framework::Animation>(e))
                g->GetComponent<Framework::Animation>(e) = snap.animation_snap;
            else
                g->AddComponent<Framework::Animation>(e, snap.animation_snap);
        }

        if (snap.hasNPC)
        {
            if (g->HasComponent<NPC>(e))
                g->GetComponent<NPC>(e) = snap.npc_snap;
            else
                g->AddComponent<NPC>(e, snap.npc_snap);
        }

        if (snap.hasAABB)
        {
            if (g->HasComponent<AABB>(e))
                g->GetComponent<AABB>(e) = snap.aabb_snap;
            else
                g->AddComponent<AABB>(e, snap.aabb_snap);
        }

        if (snap.hasHealth)
        {
            if (g->HasComponent<Health>(e))
                g->GetComponent<Health>(e) = snap.health_snap;
            else
                g->AddComponent<Health>(e, snap.health_snap);
        }

        if (snap.hasName)
        {
            if (g->HasComponent<Name>(e))
                g->GetComponent<Name>(e) = snap.name_snap;
            else
                g->AddComponent<Name>(e, snap.name_snap);
        }

        if (snap.hasNameComponent)
        {
            if (g->HasComponent<NamingComponent>(e))
                g->GetComponent<NamingComponent>(e) = snap.namingcomponent_snap;
            else
                g->AddComponent<NamingComponent>(e, snap.namingcomponent_snap);
        }

        if (snap.hasLogicComponent)
        {
            if (g->HasComponent<LogicComponent>(e))
                g->GetComponent<LogicComponent>(e) = snap.logiccomponent_snap;
            else
                g->AddComponent<LogicComponent>(e, snap.logiccomponent_snap);
        }

        if (snap.hasLayerTag)
        {
            if (g->HasComponent<LayerTag>(e))
                g->GetComponent<LayerTag>(e) = snap.layertag_snap;
            else
                g->AddComponent<LayerTag>(e, snap.layertag_snap);
        }

        if (snap.hasAudioSource)
        {
            if (g->HasComponent<AudioSource>(e))
                g->GetComponent<AudioSource>(e) = snap.audiosource_snap;
            else
                g->AddComponent<AudioSource>(e, snap.audiosource_snap);
        }

        if (snap.hasTextComponent)
        {
            if (g->HasComponent<TextComponent>(e))
                g->GetComponent<TextComponent>(e) = snap.text_snap;
            else
                g->AddComponent<TextComponent>(e, snap.text_snap);
        }

        if (snap.hasPrefabInstance)
        {
            if (g->HasComponent<PrefabInstance>(e))
                g->GetComponent<PrefabInstance>(e) = snap.prefabinstance_snap;
            else
                g->AddComponent<PrefabInstance>(e, snap.prefabinstance_snap);
        }

        // At end of CreateEntityFromEntitySnapshot(), before return e;
        auto sceneManager = g->GetSystem<SceneManager>();
        if (sceneManager) {
            sceneManager->IncrementEntityArray(e);
        }

        return e;
    }

    //void printsnapshot(EntitySnapshot snap) {
    //    std::cout << "snap hasTransform?" << snap.hasTransform << "\n";
    //    std::cout << "snap hasRenderable?" << snap.hasRenderable << "\n";
    //    std::cout << "snap hasAnimation?" << snap.hasAnimation << "\n";
    //    std::cout << "snap hasNPC?" << snap.hasNPC << "\n";
    //    std::cout << "snap hasAABB?" << snap.hasAABB << "\n";
    //}
}

