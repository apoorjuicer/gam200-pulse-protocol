///******************************************************************************/
///**
// * @file        GUIManager.h
// * @project     Pulse Protocol
// * @author      Chloe Lau Rey En
// * @brief       Declares the GUIManager system for managing interactive UI buttons.
// *              Handles button hover/click detection, entity creation/destruction
// *              actions, and persistent tracking of GUI-created entities across
// *              game sessions using JSON serialization.
// *
// * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
// *              Reproduction or disclosure of this file or its contents without the
// *              prior written consent of DigiPen Institute of Technology is prohibited.
// */
// /******************************************************************************/
//
//#pragma once
//#include "pch/pch_temp.h"
//#include "../MyEngine/CoreEngine/ECS/System.h"
//#include "../MyEngine/CoreEngine/Factory/Factory.h"
//#include "../MyEngine/Components/DisplayName.h"
//#include "../MyEngine/Components/EntityType.h"
//#include "../MyEngine/CoreEngine/Scene/SceneManager.h"
//
//struct UIElement;
//
//class GUIManager : public Systems {
//public:
//    GUIManager();
//    ~GUIManager();
//
//    void Initialize();
//    void Update(float);
//    void Shutdown();
//
//    // Set of all GUI button entities in the scene
//    std::set<Entity> GUIButtonEntities;
//
//    // Sync tracking list with entities that exist in the loaded scene
//    void SyncTrackingWithScene();
//
//private:
//    // List of entity names created by GUI buttons (for tracking/deletion)
//    std::vector<std::string> createdEntityNamesByGUI;
//
//    // Counter for consistent entity naming across sessions
//    int guiEntityCounter = 0;
//
//    // Update all GUI buttons (hover/click detection)
//    void updateGUIButtons();
//
//    // Execute button action (create or destroy entity)
//    void executeButtonAction(const GUIButton& button);
//
//    // Save/load GUI entity tracking to persistent JSON file
//    void SaveGUIEntitiesTracking();
//    void LoadGUIEntitiesTracking();
//};