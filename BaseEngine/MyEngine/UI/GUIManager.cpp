/******************************************************************************/
/**
 * @file        GUIManager.cpp
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief       Implements GUI system for in-game entity creation/destruction
 *              with persistent tracking across game sessions.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "pch/pch_temp.h"
#include "../MyEngine/UI/GUIManager.h"
#include "../MyEngine/UI/GUIButton.h"
#include "../MyEngine/CoreEngine/ECS/Coordinator.h"
#include "../MyEngine/Input/InputManager.h"

GUIManager::GUIManager() {
}

GUIManager::~GUIManager() {
    Shutdown();
}

void GUIManager::Initialize() {
    LoadGUIEntitiesTracking();
    std::cout << "[GUIManager] Successfully initialized." << std::endl;
}

void GUIManager::Update(float) {
    // Sync only when scene first loads (check if we need to sync)
    static std::string lastSyncedScene = "";
    auto* g_coordinator = Coordinator::GetInstance();
    auto sceneMgr = g_coordinator->GetSystem<SceneManager>();

    if (sceneMgr) {
        std::string currentScene = sceneMgr->GrabCurrentScene();
        if (currentScene != lastSyncedScene) {
            // Scene changed, sync now
            SyncTrackingWithScene();
            lastSyncedScene = currentScene;
        }
    }

    // Clean up any invalid entity references from GUI tracking
    if (!createdEntityNamesByGUI.empty()) {
        auto activeEntities = g_coordinator->GetAllEntities();

        createdEntityNamesByGUI.erase(
            std::remove_if(createdEntityNamesByGUI.begin(), createdEntityNamesByGUI.end(),
                [&](const std::string& name) {
                    // Check if any entity with this name still exists
                    for (auto entity : activeEntities) {
                        if (g_coordinator->HasComponent<Name>(entity)) {
                            auto& nameComp = g_coordinator->GetComponent<Name>(entity);
                            if (nameComp.name == name) {
                                return false; // Name still exists, keep it
                            }
                        }
                    }
                    return true;
                }),
            createdEntityNamesByGUI.end()
        );
    }

    updateGUIButtons();
}

void GUIManager::Shutdown() {
    std::cout << "[GUIManager] Shutdown completed." << std::endl;
}

void GUIManager::updateGUIButtons() {
    auto* g_coordinator = Coordinator::GetInstance();
    auto inputMgr = g_coordinator->GetSystem<InputManager>();

    if (!inputMgr) return;

    // Get mouse position in virtual coordinates
    float mouseX = inputMgr->getMousePositionVirtual().x;
    float mouseY = inputMgr->getMousePositionVirtual().y;
    bool mouseInViewport = inputMgr->isMouseInGameViewport();

    for (auto const& entity : GUIButtonEntities) {
        if (!g_coordinator->HasComponent<GUIButton>(entity)) continue;
        if (!g_coordinator->HasComponent<Framework::Transform>(entity)) continue;

        auto& button = g_coordinator->GetComponent<GUIButton>(entity);
        auto& transform = g_coordinator->GetComponent<Framework::Transform>(entity);

        // Check visibility and enabled state
        if (!transform.isVisible || !button.isEnabled) {
            button.currentState = ButtonState::Normal;
            continue;
        }

        // Calculate button bounds (position is at center)
        float halfWidth = transform.Scale.x * 0.5f;
        float halfHeight = transform.Scale.y * 0.5f;

        float left = transform.Pos.x - halfWidth;
        float right = transform.Pos.x + halfWidth;
        float bottom = transform.Pos.y - halfHeight;
        float top = transform.Pos.y + halfHeight;

        // Check if mouse is over button
        bool isMouseOver = (mouseX >= left && mouseX <= right &&
            mouseY >= bottom && mouseY <= top &&
            mouseInViewport);

        button.wasClickedThisFrame = false;

        if (isMouseOver) {
            button.currentState = ButtonState::Hover;

            if (inputMgr->isMouseButtonTriggered(0)) {
                std::cout << "[GUI Button] Button clicked: " << button.buttonID << std::endl;
                button.wasClickedThisFrame = true;
                executeButtonAction(button);
            }
        }
        else {
            button.currentState = ButtonState::Normal;
        }
    }
}

void GUIManager::executeButtonAction(const GUIButton& button) {
    Factory factory;
    auto* g_coordinator = Coordinator::GetInstance();

    if (button.actionType == "CreateEntity") {
        if (!button.entityJsonPath.empty()) {
            auto result = factory.CreateEntityFromJson(button.entityJsonPath);

            if (result.has_value()) {
                Entity newEntity = result.value();

                // Get entity type from NamingComponent (EntityType)
                std::string entityType = "Entity";
                if (g_coordinator->HasComponent<NamingComponent>(newEntity)) {
                    auto& naming = g_coordinator->GetComponent<NamingComponent>(newEntity);
                    entityType = naming.Type; // "Player", "Enemy", etc.
                }

                // Use counter for consistent naming
                guiEntityCounter++;

                // If entity doesn't have a Name component, add one with pretty name
                if (!g_coordinator->HasComponent<Name>(newEntity)) {
                    Name nameComp;
                    nameComp.name = "GUICreated_" + entityType + "_" + std::to_string(guiEntityCounter);
                    g_coordinator->AddComponent<Name>(newEntity, nameComp);
                }

                // Track by name
                auto& nameComp = g_coordinator->GetComponent<Name>(newEntity);
                createdEntityNamesByGUI.push_back(nameComp.name);
                std::cout << "[GUIManager] Created entity: " << nameComp.name << std::endl;

                // Auto-save tracking
                SaveGUIEntitiesTracking();
            }
        }
    }
    else if (button.actionType == "DestroyEntity") {
        if (!createdEntityNamesByGUI.empty()) {
            std::string targetName = createdEntityNamesByGUI.back();
            createdEntityNamesByGUI.pop_back();

            // Find the entity by name
            auto activeEntities = g_coordinator->GetAllEntities();
            for (auto entity : activeEntities) {
                if (g_coordinator->HasComponent<Name>(entity)) {
                    auto& nameComp = g_coordinator->GetComponent<Name>(entity);
                    if (nameComp.name == targetName) {
                        factory.DeleteEntity(entity);
                        std::cout << "[GUIManager] Destroyed entity: " << targetName << std::endl;

                        // Auto-save tracking
                        SaveGUIEntitiesTracking();
                        return;
                    }
                }
            }

            std::cout << "[GUIManager] WARNING: Could not find entity with name: " << targetName << std::endl;
        }
        else {
            std::cout << "[GUIManager] No entities to destroy (tracking list empty)" << std::endl;
        }
    }
}

void GUIManager::SaveGUIEntitiesTracking() {
    using namespace rapidjson;

    Document doc;
    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    Value namesArray(kArrayType);
    for (const auto& name : createdEntityNamesByGUI) {
        Value nameVal;
        nameVal.SetString(name.c_str(), static_cast<SizeType>(name.length()), alloc);
        namesArray.PushBack(nameVal, alloc);
    }

    doc.AddMember("guiCreatedEntities", namesArray, alloc);

    // Also save the counter to maintain consistency
    doc.AddMember("guiEntityCounter", guiEntityCounter, alloc);

    // Save to persistent file
    std::string filepath = "../../MyEngine/JSON/GUITracking.json";
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    doc.Accept(writer);

    std::ofstream ofs(filepath);
    if (ofs.is_open()) {
        ofs << buffer.GetString();
        std::cout << "[GUIManager] Saved " << createdEntityNamesByGUI.size()
            << " GUI entity names to tracking file" << std::endl;
    }
    else {
        std::cerr << "[GUIManager] ERROR: Could not save GUI tracking file!" << std::endl;
    }
}

void GUIManager::LoadGUIEntitiesTracking() {
    using namespace rapidjson;

    std::string filepath = "../../MyEngine/JSON/GUITracking.json";
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        // File doesn't exist yet - that's okay
        createdEntityNamesByGUI.clear();
        guiEntityCounter = 0;
        std::cout << "[GUIManager] No GUI tracking file found - starting fresh" << std::endl;
        return;
    }

    IStreamWrapper isw(ifs);
    Document doc;
    doc.ParseStream(isw);

    createdEntityNamesByGUI.clear();

    if (doc.HasMember("guiCreatedEntities") && doc["guiCreatedEntities"].IsArray()) {
        const auto& namesArray = doc["guiCreatedEntities"];
        for (SizeType i = 0; i < namesArray.Size(); ++i) {
            if (namesArray[i].IsString()) {
                createdEntityNamesByGUI.push_back(namesArray[i].GetString());
            }
        }
    }

    // Load the counter
    if (doc.HasMember("guiEntityCounter") && doc["guiEntityCounter"].IsInt()) {
        guiEntityCounter = doc["guiEntityCounter"].GetInt();
    }

    std::cout << "[GUIManager] Loaded " << createdEntityNamesByGUI.size()
        << " GUI entity names from tracking file (counter: " << guiEntityCounter << ")" << std::endl;
}

void GUIManager::SyncTrackingWithScene() {
    auto* g_coordinator = Coordinator::GetInstance();
    auto activeEntities = g_coordinator->GetAllEntities();

    std::cout << "[GUIManager] Syncing tracking with loaded scene..." << std::endl;

    // Find all entities with names starting with "GUICreated_"
    for (auto entity : activeEntities) {
        if (g_coordinator->HasComponent<Name>(entity)) {
            auto& nameComp = g_coordinator->GetComponent<Name>(entity);

            // Check if it's a GUI-created entity
            if (nameComp.name.find("GUICreated_") == 0) {
                // Check if not already in tracking list
                if (std::find(createdEntityNamesByGUI.begin(),
                    createdEntityNamesByGUI.end(),
                    nameComp.name) == createdEntityNamesByGUI.end()) {

                    createdEntityNamesByGUI.push_back(nameComp.name);
                    std::cout << "[GUIManager] Added to tracking: " << nameComp.name << std::endl;
                }
            }
        }
    }

    // Save the updated tracking
    if (!createdEntityNamesByGUI.empty()) {
        SaveGUIEntitiesTracking();
    }

    std::cout << "[GUIManager] Tracking synced. Total tracked entities: "
        << createdEntityNamesByGUI.size() << std::endl;
}