///******************************************************************************/
///**
// * @file        GUIManager.h
// * @project     Pulse Protocol
// * @author      Chloe Lau Rey En
// * @brief       Simple button hover and click detection using Name/Transform/Renderable.
// *              No UIElement component needed - uses layer system instead.
// *
// * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
// */
// /******************************************************************************/
//
//#pragma once
//#include "pch/pch_temp.h"
//#include "../MyEngine/CoreEngine/ECS/System.h"
//
//class GUIManager : public Systems {
//public:
//    GUIManager();
//    ~GUIManager();
//
//    void Init() override;
//    void Update(float dt) override;
//    void Exit() override;
//
//    // Set of all button entities (populated from UI layer)
//    std::set<Entity> ButtonEntities;
//
//private:
//    void updateButtons();
//    void handleButtonClick(Entity buttonEntity, const std::string& buttonName);
//};