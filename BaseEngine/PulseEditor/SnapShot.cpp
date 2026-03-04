/******************************************************************************/
/**
 * @file        SnapShot.cpp
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin
 * @brief       Implementation of entity snapshot / restore helpers.
 *
 * @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/
#include "SnapShot.h"
#include "CoreEngine/Factory/Factory.h"
#include "HierarchyHelpers.h"
//#include "EditorTypes.h"

namespace PulseEditor
{

    //// =============================================================================
    //// MakeEntitySnapshot
    //// Captures all components that currently exist on entity e.
    //// Parent / Children are stored by raw entity ID; callers that need
    //// multi-entity restore should use MakeSceneSnapshot instead.
    //// =============================================================================
    //EntitySnapshot MakeEntitySnapshot(Entity e)
    //{
    //    EntitySnapshot snap{};
    //    snap.originalEntity = e;

    //    auto* g = Coordinator::GetInstance();
    //    if (!g || e == INVALID_ENTITY)
    //        return snap;

    //    if (g->HasComponent<Framework::Transform>(e))
    //    {
    //        snap.hasTransform = true;
    //        snap.transform_snap = g->GetComponent<Framework::Transform>(e);
    //    }
    //    if (g->HasComponent<Framework::Renderable>(e))
    //    {
    //        snap.hasRenderable = true;
    //        snap.renderable_snap = g->GetComponent<Framework::Renderable>(e);
    //    }
    //    if (g->HasComponent<Framework::Animation>(e))
    //    {
    //        snap.hasAnimation = true;
    //        snap.animation_snap = g->GetComponent<Framework::Animation>(e);
    //    }
    //    if (g->HasComponent<NPC>(e))
    //    {
    //        snap.hasNPC = true;
    //        snap.npc_snap = g->GetComponent<NPC>(e);
    //    }
    //    if (g->HasComponent<AABB>(e))
    //    {
    //        snap.hasAABB = true;
    //        snap.aabb_snap = g->GetComponent<AABB>(e);
    //    }
    //    if (g->HasComponent<Health>(e))
    //    {
    //        snap.hasHealth = true;
    //        snap.health_snap = g->GetComponent<Health>(e);
    //    }
    //    if (g->HasComponent<Name>(e))
    //    {
    //        snap.hasName = true;
    //        snap.name_snap = g->GetComponent<Name>(e);
    //    }
    //    if (g->HasComponent<NamingComponent>(e))
    //    {
    //        snap.hasNameComponent = true;
    //        snap.namingcomponent_snap = g->GetComponent<NamingComponent>(e);
    //    }
    //    if (g->HasComponent<LogicComponent>(e))
    //    {
    //        snap.hasLogicComponent = true;
    //        snap.logiccomponent_snap = g->GetComponent<LogicComponent>(e);
    //    }
    //    if (g->HasComponent<LayerTag>(e))
    //    {
    //        snap.hasLayerTag = true;
    //        snap.layertag_snap = g->GetComponent<LayerTag>(e);
    //    }
    //    if (g->HasComponent<AudioSource>(e))
    //    {
    //        snap.hasAudioSource = true;
    //        snap.audiosource_snap = g->GetComponent<AudioSource>(e);
    //    }
    //    if (g->HasComponent<TextComponent>(e))
    //    {
    //        snap.hasTextComponent = true;
    //        snap.text_snap = g->GetComponent<TextComponent>(e);
    //    }
    //    if (g->HasComponent<PrefabInstance>(e))
    //    {
    //        snap.hasPrefabInstance = true;
    //        snap.prefabinstance_snap = g->GetComponent<PrefabInstance>(e);
    //    }
    //    if (g->HasComponent<Parent>(e))
    //    {
    //        snap.hasParent = true;
    //        snap.parent_snap = g->GetComponent<Parent>(e);
    //    }
    //    if (g->HasComponent<Children>(e))
    //    {
    //        snap.hasChildren = true;
    //        snap.children_snap = g->GetComponent<Children>(e);
    //    }

    //    return snap;
    //}

    //// =============================================================================
    //// ApplySnapshotToEntity
    //// Writes all captured component data onto an entity that was JUST created
    //// (blank slate).  Does NOT touch Parent/Children — SceneSnapshot handles that
    //// so IDs can be remapped correctly.
    //// =============================================================================
    //void ApplySnapshotToEntity(Entity e, const EntitySnapshot& snap, bool attachScripts)
    //{
    //    auto* g = Coordinator::GetInstance();
    //    if (!g || e == INVALID_ENTITY) return;

    //    if (snap.hasTransform)       g->AddComponent<Framework::Transform>(e, snap.transform_snap);
    //    if (snap.hasRenderable)      g->AddComponent<Framework::Renderable>(e, snap.renderable_snap);
    //    if (snap.hasAnimation)       g->AddComponent<Framework::Animation>(e, snap.animation_snap);
    //    if (snap.hasNPC)             g->AddComponent<NPC>(e, snap.npc_snap);
    //    if (snap.hasAABB)            g->AddComponent<AABB>(e, snap.aabb_snap);
    //    if (snap.hasHealth)          g->AddComponent<Health>(e, snap.health_snap);
    //    if (snap.hasName)            g->AddComponent<Name>(e, snap.name_snap);
    //    if (snap.hasNameComponent)   g->AddComponent<NamingComponent>(e, snap.namingcomponent_snap);
    //    if (snap.hasLayerTag)        g->AddComponent<LayerTag>(e, snap.layertag_snap);
    //    if (snap.hasAudioSource)     g->AddComponent<AudioSource>(e, snap.audiosource_snap);
    //    if (snap.hasTextComponent)   g->AddComponent<TextComponent>(e, snap.text_snap);
    //    if (snap.hasPrefabInstance)  g->AddComponent<PrefabInstance>(e, snap.prefabinstance_snap);

    //    // LogicComponent data (script names list etc.)
    //    if (snap.hasLogicComponent)
    //        g->AddComponent<LogicComponent>(e, snap.logiccomponent_snap);

    //    // Scene bookkeeping
    //    if (auto sceneManager = g->GetSystem<SceneManager>())
    //        sceneManager->IncrementEntityArray(e);

    //    // Re-attach scripts into ScriptAPI
    //    if (attachScripts && snap.hasLogicComponent)
    //    {
    //        auto addScriptFunc = Engine->GetFunctionPtr<bool(*)(int, const char*)>(
    //            "ScriptAPI", "ScriptAPI.EngineInterface", "AddScriptViaName");

    //        if (addScriptFunc)
    //        {
    //            for (const auto& scriptName : snap.logiccomponent_snap.ScriptName)
    //                if (!scriptName.empty())
    //                    addScriptFunc(static_cast<int>(e), scriptName.c_str());
    //        }
    //    }
    //}

    //// =============================================================================
    //// RestoreEntityFromSnapshot
    //// Overwrites components on an entity that is ALREADY alive.
    //// Used for undo of property edits (transform moved, health changed, etc.)
    //// without needing to delete and recreate the entity.
    //// =============================================================================
    //void RestoreEntityFromSnapshot(Entity e, const EntitySnapshot& snap)
    //{
    //    auto* g = Coordinator::GetInstance();
    //    if (!g || e == INVALID_ENTITY) return;

    //    if (snap.hasTransform && g->HasComponent<Framework::Transform>(e))
    //        g->GetComponent<Framework::Transform>(e) = snap.transform_snap;
    //    if (snap.hasRenderable && g->HasComponent<Framework::Renderable>(e))
    //        g->GetComponent<Framework::Renderable>(e) = snap.renderable_snap;
    //    if (snap.hasAnimation && g->HasComponent<Framework::Animation>(e))
    //        g->GetComponent<Framework::Animation>(e) = snap.animation_snap;
    //    if (snap.hasNPC && g->HasComponent<NPC>(e))
    //        g->GetComponent<NPC>(e) = snap.npc_snap;
    //    if (snap.hasAABB && g->HasComponent<AABB>(e))
    //        g->GetComponent<AABB>(e) = snap.aabb_snap;
    //    if (snap.hasHealth && g->HasComponent<Health>(e))
    //        g->GetComponent<Health>(e) = snap.health_snap;
    //    if (snap.hasName && g->HasComponent<Name>(e))
    //        g->GetComponent<Name>(e) = snap.name_snap;
    //    if (snap.hasNameComponent && g->HasComponent<NamingComponent>(e))
    //        g->GetComponent<NamingComponent>(e) = snap.namingcomponent_snap;
    //    if (snap.hasLogicComponent && g->HasComponent<LogicComponent>(e))
    //        g->GetComponent<LogicComponent>(e) = snap.logiccomponent_snap;
    //    if (snap.hasLayerTag && g->HasComponent<LayerTag>(e))
    //        g->GetComponent<LayerTag>(e) = snap.layertag_snap;
    //    if (snap.hasAudioSource && g->HasComponent<AudioSource>(e))
    //        g->GetComponent<AudioSource>(e) = snap.audiosource_snap;
    //    if (snap.hasTextComponent && g->HasComponent<TextComponent>(e))
    //        g->GetComponent<TextComponent>(e) = snap.text_snap;
    //    if (snap.hasPrefabInstance && g->HasComponent<PrefabInstance>(e))
    //        g->GetComponent<PrefabInstance>(e) = snap.prefabinstance_snap;
    //}

    //// =============================================================================
    //// MakeSceneSnapshot  (private helper: depth-first walk)
    //// =============================================================================
    //static void WalkSubtree(Entity e, SceneSnapshot& out)
    //{
    //    out.nodes.push_back(MakeEntitySnapshot(e));

    //    auto* g = Coordinator::GetInstance();
    //    if (!g) return;

    //    if (g->HasComponent<Children>(e))
    //    {
    //        for (Entity child : g->GetComponent<Children>(e).children)
    //            WalkSubtree(child, out);
    //    }
    //}

    //SceneSnapshot MakeSceneSnapshot(Entity root)
    //{
    //    SceneSnapshot scene{};
    //    WalkSubtree(root, scene);
    //    return scene;
    //}

    //// =============================================================================
    //// RestoreSceneSnapshot
    //// 1. Create a fresh entity for every node (IDs are whatever ECS gives us).
    //// 2. Apply all non-hierarchy components.
    //// 3. Build an oldID -> newID remap table.
    //// 4. Fix up Parent / Children on every new entity using the remap table.
    //// Returns the new root entity (nodes[0]).
    //// =============================================================================
    //Entity RestoreSceneSnapshot(SceneSnapshot& scene, bool attachScripts)
    //{
    //    if (scene.nodes.empty()) return INVALID_ENTITY;

    //    auto* g = Coordinator::GetInstance();
    //    if (!g) return INVALID_ENTITY;

    //    scene.oldToNew.clear();

    //    // --- pass 1: create entities and apply component data (no parenting yet) ---
    //    std::vector<Entity> newEntities;
    //    newEntities.reserve(scene.nodes.size());

    //    for (const EntitySnapshot& snap : scene.nodes)
    //    {
    //        Factory factory;
    //        Entity newE = factory.DefaultObj();
    //        if (newE == INVALID_ENTITY)
    //        {
    //            // Clean up anything we already created and bail
    //            for (Entity created : newEntities)
    //            {
    //                Factory f;
    //                f.DeleteEntity(created);
    //            }
    //            return INVALID_ENTITY;
    //        }

    //        ApplySnapshotToEntity(newE, snap, attachScripts);

    //        scene.oldToNew[snap.originalEntity] = newE;
    //        newEntities.push_back(newE);
    //    }

    //    // --- pass 2: fix up Parent / Children using the remap table ---
    //    // Helper: translate an old entity id to its new counterpart.
    //    // If not found in the remap (e.g. parent lives outside the snapshot)
    //    // keep the original id — it is still alive in the scene.
    //    auto remap = [&](Entity old) -> Entity
    //        {
    //            auto it = scene.oldToNew.find(old);
    //            return (it != scene.oldToNew.end()) ? it->second : old;
    //        };

    //    for (size_t i = 0; i < scene.nodes.size(); ++i)
    //    {
    //        const EntitySnapshot& snap = scene.nodes[i];
    //        Entity newE = newEntities[i];

    //        if (snap.hasParent)
    //        {
    //            Parent remappedParent;
    //            remappedParent.parent = remap(snap.parent_snap.parent);
    //            g->AddComponent<Parent>(newE, remappedParent);

    //            // Register this entity in the parent's Children list
    //            Entity newParent = remappedParent.parent;
    //            if (newParent != INVALID_ENTITY && newParent != static_cast<Entity>(-1))
    //            {
    //                if (!g->HasComponent<Children>(newParent))
    //                    g->AddComponent<Children>(newParent, Children{});

    //                auto& siblings = g->GetComponent<Children>(newParent).children;
    //                if (std::find(siblings.begin(), siblings.end(), newE) == siblings.end())
    //                    siblings.push_back(newE);
    //            }
    //        }

    //        if (snap.hasChildren)
    //        {
    //            Children remappedChildren;
    //            for (Entity oldChild : snap.children_snap.children)
    //                remappedChildren.children.push_back(remap(oldChild));

    //            // Guard against double-add: the hasParent block above may have
    //            // already added a Children component to this entity when registering
    //            // it as a parent of its own children.
    //            if (g->HasComponent<Children>(newE))
    //                g->GetComponent<Children>(newE) = remappedChildren;
    //            else
    //                g->AddComponent<Children>(newE, remappedChildren);
    //        }
    //    }

    //    return newEntities[0]; // root
    //}
    
    Entity CreateFromSnapshot(EntitySnapshot const& snap,Factory f)
    {
        // Create new entity and apply snapshot data
        // Return new entity id

        auto* g = Coordinator::GetInstance();
        //Entity e = f.DefaultObj();// create a new entity default obj
        Entity e = g->CreateEntity();

        if (snap.hasName) {
            g->AddComponent<Name>(e, snap.name_snap);
        }
            

        if (snap.hasTransform)
            g->AddComponent<Framework::Transform>(e, snap.transform_snap);

        if (snap.hasRenderable)
            g->AddComponent<Framework::Renderable>(e, snap.renderable_snap);

        if (snap.hasAnimation)
            g->AddComponent<Framework::Animation>(e, snap.animation_snap);

        if (snap.hasLogicComponent)
            g->AddComponent<LogicComponent>(e, snap.logiccomponent_snap);

        if (snap.hasAudioSource)
            g->AddComponent<AudioSource>(e, snap.audiosource_snap);

        if (snap.hasAABB)
            g->AddComponent<AABB>(e, snap.aabb_snap);

        if (snap.hasHealth)
            g->AddComponent<Health>(e, snap.health_snap);

        if (snap.hasTextComponent)
            g->AddComponent<TextComponent>(e, snap.text_snap);

        if (snap.hasLayerTag)
            g->AddComponent<LayerTag>(e, snap.layertag_snap);

        if (snap.hasPrefabInstance)
            g->AddComponent<PrefabInstance>(e, snap.prefabinstance_snap);

        return e; // placeholder
    }

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

} // namespace PulseEditor