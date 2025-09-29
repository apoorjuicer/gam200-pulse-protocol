#include "pch/pch.h"

void LevelManager::LoadDemo()
{
    auto* g_coordinator = Coordinator::GetInstance();

    //// Read from JSON file
    //FILE* fp = fopen("Assets/Levels/demo_level.json", "rb");
    //if (!fp) {
    //    std::cerr << "Failed to open level file!" << std::endl;
    //    return;
    //}

    //char readBuffer[65536];
    //rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    //rapidjson::Document levelData;
    //levelData.ParseStream(is);
    //fclose(fp);

    //if (levelData.HasParseError()) {
    //    std::cerr << "JSON parse error!" << std::endl;
    //    return;
    //}
    //// Create entities from JSON
    //const rapidjson::Value& entities = levelData["entities"];
    //for (rapidjson::SizeType i = 0; i < entities.Size(); i++) {
    //    Entity bgEntity = g_coordinator->CreateEntity();

    //    Framework::Transform bgTransform{};
    //    bgTransform.Pos = {
    //        entities[i]["position"]["x"].GetFloat(),
    //        entities[i]["position"]["y"].GetFloat()
    //    };
    //    bgTransform.Scale = {
    //        entities[i]["scale"]["x"].GetFloat(),
    //        entities[i]["scale"]["y"].GetFloat()
    //    };
    //    bgTransform.mdl_ref = 0.0f;
    //    bgTransform.shd_ref = 0.0f;
    //    bgTransform.rows = 1;
    //    bgTransform.columns = 1;
    //    bgTransform.currentFrame = 0;
    //    bgTransform.animationSpeed = 0.0f;
    //    bgTransform.isVisible = true;

    //    g_coordinator->AddComponent(bgEntity, bgTransform);
    //}

    // Background entity
    Entity bgEntity = g_coordinator->CreateEntity();
    Framework::Transform bgTransform{};
    bgTransform.Pos = { 800.0f, 450.0f };
    bgTransform.Scale = { 1600.0f, 900.0f };  // Covers full screen
    bgTransform.mdl_ref = 0.0f;
    bgTransform.shd_ref = 0.0f;
    bgTransform.rows = 1;
    bgTransform.columns = 1;
    bgTransform.currentFrame = 0;
    bgTransform.animationSpeed = 0.0f;
    bgTransform.isVisible = true;
    g_coordinator->AddComponent(bgEntity, bgTransform);

    // Feedback entity for InputManager (the ONE rectangle that appears)
    Entity inputEntity = g_coordinator->CreateEntity();
    Framework::Transform inputTr{};
    inputTr.Pos = { 800.0f, 450.0f };
    inputTr.Scale = { 128.0f, 128.0f };
    inputTr.mdl_ref = 1.0f;
    inputTr.shd_ref = 1.0f;
    inputTr.isVisible = false;  // Starts invisible, shows on input
    inputTr.currentFrame = 0;
    inputTr.autoHide = false;
    inputTr.visibilityTimer = 0.0f;

    inputTr.color = glm::vec3(1.0f, 0.5f, 0.0f);
    g_coordinator->AddComponent(inputEntity, inputTr);

    // Tell InputManager which entity to use for feedback
    auto inputManager = g_coordinator->GetSystem<InputManager>();
    inputManager->SetFeedbackEntity(inputEntity);
}