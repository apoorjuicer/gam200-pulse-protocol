#pragma once
#include <vector>
#include <algorithm>
#include "UndoManager.h"
#include <memory>
#include "CoreEngine/Factory/Factory.h" 
#include "SnapShot.h"
#include "EditorTypes.h"
#include <iostream>
//using Entity = std::uint32_t; lazy put header
//std::uint32_t = Entity  


namespace PulseEditor
{
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
                handleOf.erase(old);
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

    inline void PushAnimationEdit(UndoManager& undo,
        std::shared_ptr<EntityHandle> h,
        Framework::Animation const& before,
        Framework::Animation const& after)
    {
        if (!h || h->id == INVALID_ENTITY) return;

        UndoManager::Action a;

        a.undo = [h, before]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<Framework::Animation>(h->id)) return;

                c->GetComponent<Framework::Animation>(h->id) = before;
            };

        a.redo = [h, after]()
            {
                if (!h || h->id == INVALID_ENTITY) return;
                auto* c = Coordinator::GetInstance();
                if (!c) return;
                if (!c->HasComponent<Framework::Animation>(h->id)) return;

                c->GetComponent<Framework::Animation>(h->id) = after;
            };

        undo.Push(std::move(a));
    }

}

