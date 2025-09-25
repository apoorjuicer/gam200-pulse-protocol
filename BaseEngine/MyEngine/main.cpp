// ==================================================================================
// main.cpp
// Authors: Team Bloop - Rachael (Window Setup), Chloe Lau Rey En (Input System)
//          Pin Kai, Benjamin, Reginald, Carrie, Fenny, Jun Yong (TBA)
// Brief: Main application entry point for custom game engine. Currently demonstrates
//        window creation and input system for M1 milestone. Additional systems
//        will be integrated as team completes their respective components.
// ==================================================================================
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>   // for OutputDebugString, etc.
#endif

#include <iostream>
#include "Core/Coordinator.h"
#include "pch/pch.h"
#include "Core/Assets.h"

#if (_DEBUG)
// MEM LEAKS LIBRARIES (See _CrtMemCheckpoint for examples to check for mem leaks)
#define _CRTDBG_MAP_ALLOC //to get more details
#include <cstdlib>  
#include <crtdbg.h>   //for malloc and free
static _CrtMemState sOld;
static _CrtMemState sNew;
static _CrtMemState sDiff;
#endif

// Game window dimensions and title
const char WindowTitle[] = "Game Window";
const int Game_Window_Height = 1400;
const int Game_Window_Width = 800;

// just concepts for game states, will be in GSM 
struct GameStates {
    bool MainMenu = true;
    bool IsRunning = false;
    bool IsPaused = false;
    bool GameOver = false;

    //libs that are good to use.
        //fmod
        //imgui
        //glm 
        //glfw
        //freetype for font
        //stb
        //xcore lib for game engine
        //spdlog
        //rapid json
} gamestate;

CoreEngine Engine;
Coordinator g_coordinator;
//InputManager* g_inputSystem = nullptr;
//GraphicsManager* g_graphicsSystem = nullptr;

int main() {
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    Engine.Setup();
    // If CoreEngine::Init() also calls glewInit() or GraphicsManager->Initialize(),
    // REMOVE those calls from CoreEngine::Init() or call Engine.Init() *here* only
    // for non-GL things:
    Engine.Init(); // <-- only if it does NOT touch OpenGL


    Framework::Transform tr{};            // fill in fields as your renderer expects
    // e.g. tr.mdl_ref = 1; tr.shd_ref = 1; tr.rows = ...; tr.columns = ...; tr.textureID = ...
    // After registering components (including Framework::Transform) and creating the entity:

    // where to draw (your update() normalizes by 1600x900)
    tr.Pos = { 800.0f, 450.0f };   // center of a 1600x900 �virtual� screen
    tr.Scale = { 128.0f, 128.0f };   // size in pixels in that same space
    tr.angle_disp = 0.0f;           // degrees
    tr.angle_speed = 0.0f;           // no rotation

    // model & shader indices (you push models: 0=bg, 1=box; shaders: 0=test,1=bg,2=sprite,3=font)
    tr.mdl_ref = 0.0f;               // box_model()
    tr.shd_ref = 0.0f;               // sprite shader
    // NOTE: In drawTrans() you currently check `if (x.shd_ref == 1)` before grabbing uniforms.
    // Either change that `1` to `2`, or set `tr.shd_ref = 1` if your bg shader has those uniforms.

    // sprite-sheet / animation
    tr.rows = 1;
    tr.columns = 1;
    tr.currentFrame = 0;
    tr.animationSpeed = 0.0f;        // static

    // a valid GL texture id. Use your loader or 0 if you just want a colored quad.
    tr.textureID = /* loadTexture("Assets/sprites/hero.png") */ 0;
    int entity = g_coordinator.CreateEntity();
    g_coordinator.AddComponent(entity, tr);  // <-- add Transform

    // this is where the update() is 
    Engine.GameLoop();

    // clean up
    Engine.Exit();

#if 0
    _CrtMemCheckpoint(&sNew); //take a snapshot (lock in the memory use again and now we compare :3)
    if (_CrtMemDifference(&sDiff, &sOld, &sNew)) // if there is a difference
    {
        OutputDebugString("-----------_CrtMemDumpStatistics ---------\n");
        _CrtMemDumpStatistics(&sDiff);
        OutputDebugString("-----------_CrtMemDumpAllObjectsSince ---------\n");
        _CrtMemDumpAllObjectsSince(&sOld);
        OutputDebugString("-----------_CrtDumpMemoryLeaks ---------\n");
        _CrtDumpMemoryLeaks();
    }
    else
    {
        OutputDebugString(" ////////////////////////////////////////////////////// \n");
        OutputDebugString(" //                                                  // \n");
        OutputDebugString(" //                NO MEMORY LEAKS !!!               // \n");
        OutputDebugString(" //                                                  // \n");
        OutputDebugString(" ////////////////////////////////////////////////////// \n");
    }
#endif

    return 0;
}