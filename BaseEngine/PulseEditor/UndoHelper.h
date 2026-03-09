/******************************************************************************/
/**
 * @file        UndoHelper.h
 * @project     Pulse Protocol
 * @author      Ban Kai Wei Benjamin
 * @brief       Functions that implement undo logic using undomanager,
 *              snapshot, hierarchyhelpers 
 *               and editortypes with engine's factory functions
 *
 * @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#pragma once
#include <vector>
#include <algorithm>
#include "UndoManager.h"
#include <memory>
#include "CoreEngine/Factory/Factory.h" 
#include "SnapShot.h"
#include "EditorTypes.h"
#include "HierarchyHelpers.h"
#include <iostream>
#include <unordered_set>
//using Entity = std::uint32_t; lazy put header
//std::uint32_t = Entity  


namespace PulseEditor
{

    struct DeletedNode
    {
        Entity oldEntity = INVALID_ENTITY;
        EntitySnapshot snapshot{};
        int displayIndex = -1;
        std::shared_ptr<EntityHandle> handle;
    };

    //not needed for now
    //inline std::shared_ptr<EntityHandle> EnsureHandle(Entity e)
    //{
    //    if (e == INVALID_ENTITY) return nullptr;

    //    auto it = handleOf.find(e);
    //    if (it != handleOf.end() && it->second)
    //        return it->second;

    //    auto h = std::make_shared<EntityHandle>();
    //    h->id = e;
    //    handleOf[e] = h;
    //    return h;
    //}

    // Helper: remove entity from display order if present
    //find entity e and remove it from the vector of entities
    inline void RemoveFromDisplayOrder(std::vector<std::uint32_t>& order, std::uint32_t e)
    {
        auto it = std::find(order.begin(), order.end(), e);
        if (it != order.end())
            order.erase(it);
    }
    //
    // Helper: insert entity back into display order at index (clamped)
    /* index needed for order of when entity ID is added to
    * order = [10, 42, 77, 99] //entity IDs
          0   1   2   3   // indices
    */
    inline void InsertIntoDisplayOrder(std::vector<std::uint32_t>& order, std::uint32_t e, int index)
    {
        if (index < 0) index = 0;
        if (index > (int)order.size()) index = (int)order.size();
        order.insert(order.begin() + index, e);
    }

    // Helper: find index of entity in display order
    // -1 means not found (INVALID ENTITY), 
    inline int FindIndexInDisplayOrder(std::vector<std::uint32_t> const& order, std::uint32_t e)
    {
        auto it = std::find(order.begin(), order.end(), e);
        if (it == order.end()) return INVALID_ENTITY;
        return static_cast<int>(it - order.begin());
    }

    inline void RebindHandle(std::unordered_map<Entity, std::shared_ptr<EntityHandle>>& hof,
        std::shared_ptr<EntityHandle> const& h,
        Entity oldId,
        Entity newId)
    {
        if (!h) return;

        if (oldId != INVALID_ENTITY)
            hof.erase(oldId);

        if (newId != INVALID_ENTITY)
            hof[newId] = h;
    }

    inline std::shared_ptr<EntityHandle>
        PushCreateDefault(UndoManager& undo, Factory& factory, std::vector<Entity>& displayOrder
        , std::unordered_map<Entity, std::shared_ptr<EntityHandle>>& hof)
    {
        // Shared stable handle for this conceptual object
        auto h = std::make_shared<EntityHandle>();
        h->id = INVALID_ENTITY;

        {
            Entity created = factory.DefaultObj();
            h->id = created;
            hof[created] = h;
            // Put into display list (optional)
            displayOrder.push_back(created);
        }

        // Build undo/redo lambdas that refer ONLY to stable data:
        // - factory reference
        // - displayOrder reference
        // - shared handle h (stable)
        UndoManager::Action a;

        a.undo = [h, &factory, &displayOrder, &hof]()
            {
                // Undo create = delete the currently live entity
                if (h->id == INVALID_ENTITY) return;

                Entity old = h->id;

                // Delete from ECS
                factory.DeleteEntity(old);
                
                // Remove from UI list first
                RemoveFromDisplayOrder(displayOrder, old);

                // unbind map
                hof.erase(old);
                // Mark as not alive
                h->id = INVALID_ENTITY;
            };

        a.redo = [h, &factory, &displayOrder, &hof]()
            {
                // Redo create = create a NEW entity (new id), then update the handle
                if (h->id != INVALID_ENTITY)
                {
                    // Already alive somehow; avoid double-create
                    return;
                }

                Entity createdAGN = factory.DefaultObj();
                h->id = createdAGN;
                hof[createdAGN] = h;

                // Add back into UI list
                displayOrder.push_back(createdAGN);
            };

        undo.Push(std::move(a));
        return h; // return handle so caller can keep selection tracking
    }

    inline void
        PushDeleteEntity(UndoManager& undo,
            Factory& factory,
            std::vector<Entity>& displayOrder,
            std::shared_ptr<EntityHandle> h
        , std::unordered_map<Entity, std::shared_ptr<EntityHandle>>& hof)
    {
        if (!h || h->id == INVALID_ENTITY)
            return; // nothing to delete

        // Capture snapshot BEFORE deleting
        EntitySnapshot snap = MakeEntitySnapshot(h->id);

        // Capture UI position BEFORE removing
        int index = FindIndexInDisplayOrder(displayOrder, h->id);

        {
            Entity old = h->id;
            RemoveFromDisplayOrder(displayOrder, old);
            factory.DeleteEntity(old);
            hof.erase(old);
            h->id = INVALID_ENTITY; //assign to invalid
        }

        UndoManager::Action a;
        //the mutable keyword in a lambda expression allows the body of the lambda 
        // to modify variables that were captured by value
        a.undo = [h, snap, index, &factory, &displayOrder, &hof]() mutable
            {
                // Undo delete = recreate entity from snapshot
                if (h->id != INVALID_ENTITY)
                {
                    // Already alive; avoid double-spawn
                    return;
                }

                std::cout << "[CreateFromSnapshot] UNDOING..\n";

                Entity recreated = CreateFromSnapshot(snap);
                if (recreated == INVALID_ENTITY) {
                    std::cout << "[CreateFromSnapshot] recreated is invalid entitiy..\n";
                    return;
                }
              

                h->id = recreated;
                hof[recreated] = h;

                // Restore UI order at original index (optional)
                InsertIntoDisplayOrder(displayOrder, recreated, index);

                //if (index < 0 || index >(int)displayOrder.size())
                //    displayOrder.push_back(recreated);
                //else
                //    displayOrder.insert(displayOrder.begin() + index, recreated);
         
            };

        a.redo = [h, &factory, &displayOrder, &hof]()
            {
                // Redo delete = delete the currently live entity
                if (h->id == INVALID_ENTITY)
                    return;
                Entity old = h->id;
                RemoveFromDisplayOrder(displayOrder, old);
                factory.DeleteEntity(old);
                hof.erase(old);
                h->id = INVALID_ENTITY;
            };

        undo.Push(std::move(a));
    }

    inline void PushDeleteSubtree(
        UndoManager& undo,
        Factory& factory,
        std::vector<Entity>& displayOrder,
        std::unordered_map<Entity, std::shared_ptr<EntityHandle>>& hof,
        Entity root,
        Entity& se,
        std::shared_ptr<EntityHandle>& sh)
    {
        if (root == INVALID_ENTITY)
            return;

        std::vector<Entity> entitiesToDelete;
        HierarchyHelpers::CollectSubtreeEntities(root, entitiesToDelete);
        std::unordered_set<Entity> deletedSet(entitiesToDelete.begin(), entitiesToDelete.end());
        if (entitiesToDelete.empty())
            return;

        auto deleted = std::make_shared<std::vector<DeletedNode>>();
        deleted->reserve(entitiesToDelete.size());

        // Capture all info BEFORE deletion
        for (Entity e : entitiesToDelete)
        {
            DeletedNode node;
            node.oldEntity = e;
            node.snapshot = MakeEntitySnapshot(e);
            node.displayIndex = FindIndexInDisplayOrder(displayOrder, e);

            auto itHandle = hof.find(e);
            if (itHandle != hof.end())
            {
                node.handle = itHandle->second;
            }
            else
            {
                node.handle = std::make_shared<EntityHandle>();
                node.handle->id = e;
                hof[e] = node.handle;
            }

            deleted->push_back(node);
        }

        // Delete now
        for (Entity e : entitiesToDelete)
        {
            auto* g = Coordinator::GetInstance();
            if (g && g->HasComponent<Parent>(e))
            {
                Entity parent = g->GetComponent<Parent>(e).parent;

                // only detach from parent if parent is NOT also being deleted
                if (parent != INVALID_ENTITY && deletedSet.find(parent) == deletedSet.end())
                {
                    HierarchyHelpers::SetParent(e, HierarchyHelpers::INVALID);
                }
            }

            RemoveFromDisplayOrder(displayOrder, e);
            hof.erase(e);
            factory.DeleteEntity(e);
        }

        for (auto& node : *deleted)
        {
            if (node.handle)
                node.handle->id = INVALID_ENTITY;
        }

        se = INVALID_ENTITY;
        sh.reset();

        UndoManager::Action a;

        a.undo = [deleted, &displayOrder, &hof, &se, &sh]() mutable
            {
                auto* g = Coordinator::GetInstance();
                if (!g) return;

                std::unordered_map<Entity, Entity> oldToNew;

                // Pass 1: recreate all entities
                for (auto& node : *deleted)
                {
                    Entity recreated = CreateFromSnapshot(node.snapshot);
                    if (recreated == INVALID_ENTITY)
                        continue;

                    oldToNew[node.oldEntity] = recreated;

                    if (node.handle)
                    {
                        node.handle->id = recreated;
                        hof[recreated] = node.handle;
                    }
                }

                // Pass 2: clear recreated hierarchy data
                for (auto& node : *deleted)
                {
                    auto itNew = oldToNew.find(node.oldEntity);
                    if (itNew == oldToNew.end())
                        continue;

                    Entity newEntity = itNew->second;

                    if (g->HasComponent<Parent>(newEntity))
                        g->RemoveComponent<Parent>(newEntity);

                    if (g->HasComponent<Children>(newEntity))
                        g->RemoveComponent<Children>(newEntity);
                }

                // Pass 3: rebuild hierarchy using saved parent links
                for (auto& node : *deleted)
                {
                    auto itNewChild = oldToNew.find(node.oldEntity);
                    if (itNewChild == oldToNew.end())
                        continue;

                    Entity newChild = itNewChild->second;

                    //if (node.snapshot.hasParent)
                    //{
                    //    Entity oldParent = node.snapshot.parent_snap.parent;
                    //    auto itNewParent = oldToNew.find(oldParent);

                    //    if (itNewParent != oldToNew.end())
                    //    {
                    //        Entity newParent = itNewParent->second;
                    //        HierarchyHelpers::SetParent(newChild, newParent);
                    //    }
                    //    else
                    //    {
                    //        HierarchyHelpers::SetParent(newChild, INVALID_ENTITY);
                    //    }
                    //}
                    //else
                    //{
                    //    HierarchyHelpers::SetParent(newChild, INVALID_ENTITY);
                    //}
                    if (node.snapshot.hasParent)
                    {
                        Entity oldParent = node.snapshot.parent_snap.parent;
                        Entity parentToUse = INVALID_ENTITY;

                        // Case 1: parent was deleted too and got recreated
                        auto itNewParent = oldToNew.find(oldParent);
                        if (itNewParent != oldToNew.end())
                        {
                            parentToUse = itNewParent->second;
                        }
                        else
                        {
                            // Case 2: parent was not deleted; if it still exists, use it directly
                            if (oldParent != INVALID_ENTITY)
                            {
                                bool liveParentExists =
                                    g->HasComponent<Framework::Transform>(oldParent) ||
                                    g->HasComponent<Name>(oldParent) ||
                                    g->HasComponent<LayerTag>(oldParent);

                                if (liveParentExists)
                                    parentToUse = oldParent;
                            }
                        }

                        HierarchyHelpers::SetParent(newChild, parentToUse);
                    }
                    else
                    {
                        HierarchyHelpers::SetParent(newChild, INVALID_ENTITY);
                    }
                }

                // Pass 4: restore display order
                for (auto& node : *deleted)
                {
                    if (!node.handle || node.handle->id == INVALID_ENTITY)
                        continue;

                    InsertIntoDisplayOrder(displayOrder, node.handle->id, node.displayIndex);
                }

                // Pass 5: restore selection to root
                if (!deleted->empty())
                {
                    auto& rootNode = deleted->back(); // root is last
                    sh = rootNode.handle;
                    se = sh ? sh->id : INVALID_ENTITY;
                }
            };

        a.redo = [deleted, &displayOrder, &hof, &se, &sh, &factory]() mutable
            {
                // deleted vector order is children first, parent last
                // deleting in this same order is okay
                for (auto& node : *deleted)
                {
                    if (!node.handle || node.handle->id == INVALID_ENTITY)
                        continue;

                    Entity e = node.handle->id;

                    RemoveFromDisplayOrder(displayOrder, e);
                    hof.erase(e);
                    factory.DeleteEntity(e);
                    node.handle->id = INVALID_ENTITY;
                }

                se = INVALID_ENTITY;
                sh.reset();
            };

        undo.Push(std::move(a));
    }

    /*
    Components
    */

    inline void PushTransformEdit(UndoManager& undo,
        std::shared_ptr<EntityHandle> h,
        Framework::Transform const& before,
        Framework::Transform const& after)
    {
        std::cout << "[UndoH] PushTransformEdit called\n";

        std::cout << "[UndoH] h ptr = " << h.get()
            << " id = " << (h ? h->id : INVALID_ENTITY)
            << " INVALID_ENTITY = " << INVALID_ENTITY
            << "\n";

        if (!h || h->id == INVALID_ENTITY) return;
        std::cout << "[UndoH] PushTransformEdit no early return\n";
        UndoManager::Action a;

        a.undo = [h, before]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<Framework::Transform>(h->id)) return;

                c->GetComponent<Framework::Transform>(h->id) = before;
                std::cout << "[UndoH] undo transform for entity id " << h->id 
                    << "[UndoH] transform values are " << before << "\n";
            };

        a.redo = [h, after]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<Framework::Transform>(h->id)) return;

                c->GetComponent<Framework::Transform>(h->id) = after;
                //std::cout << "[UndoH] redo transform for entity id " << h->id
                 //   << "[UndoH] transform values are " << after << "\n";
            };

        undo.Push(std::move(a));
    }

    inline void PushRenderableEdit(UndoManager& undo,
        std::shared_ptr<EntityHandle> h,
        Framework::Renderable const& before,
        Framework::Renderable const& after)
    {
        if (!h || h->id == INVALID_ENTITY) return;

        UndoManager::Action a;

        a.undo = [h, before]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<Framework::Renderable>(h->id)) return;

                c->GetComponent<Framework::Renderable>(h->id) = before;
            };

        a.redo = [h, after]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<Framework::Renderable>(h->id)) return;

                c->GetComponent<Framework::Renderable>(h->id) = after;
            };

        undo.Push(std::move(a));
    }

    inline void PushNameEdit(UndoManager& undo,
        std::shared_ptr<EntityHandle> h,
        Name const& before,
        Name const& after)
    {


        std::cout << "[UndoH] PushNameEdit called\n";
        if (!h || h->id == INVALID_ENTITY) return;
        std::cout << "[UndoH] PushNameEdit called after early return check\n";
        UndoManager::Action a;

        a.undo = [h, before]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                std::cout << "[UndoH] PushNameEdit undo \n";
                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<Name>(h->id)) return;

                c->GetComponent<Name>(h->id) = before;
            };

        a.redo = [h, after]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                std::cout << "[UndoH] PushNameEdit redo \n";
                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<Name>(h->id)) return;

                c->GetComponent<Name>(h->id) = after;
            };

        undo.Push(std::move(a));
    }

    inline void PushAnimationEdit(UndoManager& undo,
        std::shared_ptr<EntityHandle> h,
        Framework::Animation const& before,
        Framework::Animation const& after)
    {
        std::cout << "[UndoH] PushAnimationEdit called\n";
        if (!h || h->id == INVALID_ENTITY) return;
        std::cout << "[UndoH] PushAnimationEdit called after early return check\n";

        UndoManager::Action a;

        a.undo = [h, before]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                std::cout << "[UndoH] PushAnimationEdit undo\n";

                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<Framework::Animation>(h->id)) return;

                c->GetComponent<Framework::Animation>(h->id) = before;
            };

        a.redo = [h, after]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                std::cout << "[UndoH] PushAnimationEdit redo\n";

                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<Framework::Animation>(h->id)) return;

                c->GetComponent<Framework::Animation>(h->id) = after;
            };

        undo.Push(std::move(a));
    }

    inline void PushAABBEdit(
        UndoManager& undo,
        std::shared_ptr<EntityHandle> h,
        AABB const& before,
        AABB const& after)
    {
        if (!h || h->id == INVALID_ENTITY) return;

        UndoManager::Action a;

        a.undo = [h, before]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<AABB>(h->id)) return;

                c->GetComponent<AABB>(h->id) = before;
            };

        a.redo = [h, after]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<AABB>(h->id)) return;

                c->GetComponent<AABB>(h->id) = after;
            };

        undo.Push(std::move(a));
    }

    inline void PushHealthEdit(
        UndoManager& undo,
        std::shared_ptr<EntityHandle> h,
        Health const& before,
        Health const& after)
    {
        if (!h || h->id == INVALID_ENTITY) return;

        UndoManager::Action a;

        a.undo = [h, before]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<Health>(h->id)) return;

                c->GetComponent<Health>(h->id) = before;
            };

        a.redo = [h, after]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<Health>(h->id)) return;

                c->GetComponent<Health>(h->id) = after;
            };

        undo.Push(std::move(a));
    }

    inline void PushTextEdit(UndoManager& undo,
        std::shared_ptr<EntityHandle> h,
        TextComponent const& before,
        TextComponent const& after)
    {
        if (!h || h->id == INVALID_ENTITY) return;

        UndoManager::Action a;

        a.undo = [h, before]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<TextComponent>(h->id)) return;

                c->GetComponent<TextComponent>(h->id) = before;
            };

        a.redo = [h, after]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<TextComponent>(h->id)) return;

                c->GetComponent<TextComponent>(h->id) = after;
            };

        undo.Push(std::move(a));
    }

    /*
    * Remove component undo
    */

    //inline void PushRemoveName(UndoManager& undo,
    //    std::shared_ptr<EntityHandle> h,
    //    Name const& removedValue)
    //{
    //    if (!h || h->id == INVALID_ENTITY) return;

    //    UndoManager::Action a;

    //    a.undo = [h, removedValue]()
    //        {
    //            if (!h || h->id == INVALID_ENTITY) return;
    //            auto* c = Coordinator::GetInstance();
    //            if (!c) return;
    //            if (c->HasComponent<Name>(h->id)) return;

    //            c->AddComponent<Name>(h->id, removedValue);
    //        };

    //    a.redo = [h]()
    //        {
    //            if (!h || h->id == INVALID_ENTITY) return;
    //            auto* c = Coordinator::GetInstance();
    //            if (!c) return;
    //            if (!c->HasComponent<Name>(h->id)) return;

    //            c->RemoveComponent<Name>(h->id);
    //        };

    //    undo.Push(std::move(a));
    //}

}

