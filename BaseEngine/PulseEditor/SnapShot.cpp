/******************************************************************************/
/**
 * @file        SnapShot.cpp
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin
 * @brief       contains function to "snapshot" save an entity before deleting
 *              so we can restore the entity when we undo
 *
 * @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#include "SnapShot.h"
#include "CoreEngine/Factory/Factory.h"

namespace PulseEditor
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

        if (g->HasComponent<Parent>(e))
        {
            snap.hasParent = true;
            snap.parent_snap = g->GetComponent<Parent>(e);
        }

        if (g->HasComponent<Children>(e))
        {
            snap.hasChildren = true;
            snap.children_snap = g->GetComponent<Children>(e);
        }

        return snap;
    }
    Entity CreateEntityFromEntitySnapshot(EntitySnapshot const& snap,
        bool restoreParenting,
        bool attachScripts)
    {
        auto* g = Coordinator::GetInstance();
        if (!g)
            return INVALID_ENTITY;

        Entity e = g->CreateEntity();
        if (e == INVALID_ENTITY || e >= MaxEntity)
            return INVALID_ENTITY;

        // ---- restore components ----
        if (snap.hasTransform)       g->AddComponent<Framework::Transform>(e, snap.transform_snap);
        if (snap.hasRenderable)      g->AddComponent<Framework::Renderable>(e, snap.renderable_snap);
        if (snap.hasAnimation)       g->AddComponent<Framework::Animation>(e, snap.animation_snap);
        if (snap.hasNPC)             g->AddComponent<NPC>(e, snap.npc_snap);
        if (snap.hasAABB)            g->AddComponent<AABB>(e, snap.aabb_snap);
        if (snap.hasHealth)          g->AddComponent<Health>(e, snap.health_snap);
        if (snap.hasName)            g->AddComponent<Name>(e, snap.name_snap);
        if (snap.hasNameComponent)   g->AddComponent<NamingComponent>(e, snap.namingcomponent_snap);
        if (snap.hasLayerTag)        g->AddComponent<LayerTag>(e, snap.layertag_snap);
        if (snap.hasAudioSource)     g->AddComponent<AudioSource>(e, snap.audiosource_snap);
        if (snap.hasTextComponent)   g->AddComponent<TextComponent>(e, snap.text_snap);
        if (snap.hasPrefabInstance)  g->AddComponent<PrefabInstance>(e, snap.prefabinstance_snap);

        // restore LogicComponent DATA (not just scripts)
        if (snap.hasLogicComponent)
            g->AddComponent<LogicComponent>(e, snap.logiccomponent_snap);

        // Scene bookkeeping (matches Factory::DefaultObj / CreateEntityFromJson)
        if (auto sceneManager = g->GetSystem<SceneManager>())
            sceneManager->IncrementEntityArray(e);

        // Optional: reattach scripts into ScriptAPI container
        if (attachScripts && snap.hasLogicComponent)
        {
            auto addScriptFunc = Engine->GetFunctionPtr<bool(*)(int, const char*)>(
                "ScriptAPI",
                "ScriptAPI.EngineInterface",
                "AddScriptViaName"
            );

            if (addScriptFunc)
            {
                for (const auto& scriptName : snap.logiccomponent_snap.ScriptName)
                {
                    if (!scriptName.empty())
                        addScriptFunc((int)e, scriptName.c_str());
                }
            }
        }

        // Parent/Children: only if caller wants it (multi-entity restore will remap!)
        if (restoreParenting)
        {
            if (snap.hasParent)   g->AddComponent<Parent>(e, snap.parent_snap);
            if (snap.hasChildren) g->AddComponent<Children>(e, snap.children_snap);
        }

        return e;
    }

    void RestoreEntityFromSnapshot(Entity e, const EntitySnapshot& snap)
    {
        auto* g = Coordinator::GetInstance();
        if (!g || e == INVALID_ENTITY) return;

        if (snap.hasTransform && g->HasComponent<Framework::Transform>(e))
            g->GetComponent<Framework::Transform>(e) = snap.transform_snap;

        if (snap.hasRenderable && g->HasComponent<Framework::Renderable>(e))
            g->GetComponent<Framework::Renderable>(e) = snap.renderable_snap;

        if (snap.hasAnimation && g->HasComponent<Framework::Animation>(e))
            g->GetComponent<Framework::Animation>(e) = snap.animation_snap;

        if (snap.hasNPC && g->HasComponent<NPC>(e))
            g->GetComponent<NPC>(e) = snap.npc_snap;

        if (snap.hasAABB && g->HasComponent<AABB>(e))
            g->GetComponent<AABB>(e) = snap.aabb_snap;

        if (snap.hasHealth && g->HasComponent<Health>(e))
            g->GetComponent<Health>(e) = snap.health_snap;

        if (snap.hasName && g->HasComponent<Name>(e))
            g->GetComponent<Name>(e) = snap.name_snap;

        if (snap.hasNameComponent && g->HasComponent<NamingComponent>(e))
            g->GetComponent<NamingComponent>(e) = snap.namingcomponent_snap;

        if (snap.hasLogicComponent && g->HasComponent<LogicComponent>(e))
            g->GetComponent<LogicComponent>(e) = snap.logiccomponent_snap;

        if (snap.hasLayerTag && g->HasComponent<LayerTag>(e))
            g->GetComponent<LayerTag>(e) = snap.layertag_snap;

        if (snap.hasAudioSource && g->HasComponent<AudioSource>(e))
            g->GetComponent<AudioSource>(e) = snap.audiosource_snap;

        if (snap.hasTextComponent && g->HasComponent<TextComponent>(e))
            g->GetComponent<TextComponent>(e) = snap.text_snap;

        if (snap.hasPrefabInstance && g->HasComponent<PrefabInstance>(e))
            g->GetComponent<PrefabInstance>(e) = snap.prefabinstance_snap;
    }

    Entity CreateEntityFromEntitySnapshot(EntitySnapshot const& snap)
    {
        // default behavior: restore parenting=false, attachScripts=true/false up to you
        return CreateEntityFromEntitySnapshot(snap, /*restoreParenting*/true, /*attachScripts*/true);
    }

    //void printsnapshot(EntitySnapshot snap) {
    //    std::cout << "snap hasTransform?" << snap.hasTransform << "\n";
    //    std::cout << "snap hasRenderable?" << snap.hasRenderable << "\n";
    //    std::cout << "snap hasAnimation?" << snap.hasAnimation << "\n";
    //    std::cout << "snap hasNPC?" << snap.hasNPC << "\n";
    //    std::cout << "snap hasAABB?" << snap.hasAABB << "\n";
    //}
}

