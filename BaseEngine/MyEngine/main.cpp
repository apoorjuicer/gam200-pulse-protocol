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
#include "Core/Log.h" // <-- include your logging system
#include <GLFW/glfw3.h>

// ----------------- Memory leak detection for DEBUG -----------------
#if (_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>  
#include <crtdbg.h>
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

CoreEngine* Engine = nullptr;

// ----------------- Toggle key for crash test -----------------
static std::string KeyToString(int key) {
    switch (key) {
    case GLFW_KEY_W: return "W";
    case GLFW_KEY_A: return "A";
    case GLFW_KEY_S: return "S";
    case GLFW_KEY_D: return "D";
    case GLFW_KEY_P: return "P";
    default: return std::to_string(key);
    }
}

int main() {
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // ----------------- Initialize Logging System -----------------
    GAM200graphics::Log::Init();
    GAM200_CORE_INFO("Core Logger Initialized.");
    GAM200_INFO("Client Logger Initialized.");

    // ----------------- Create the Engine instance -----------------
    Engine = new CoreEngine();

    Engine->Setup();
    Engine->Init(); // <-- only if it does NOT touch OpenGL

    // ----------------- Transform Setup -----------------
    Framework::Transform tr{};            // fill in fields as your renderer expects

    // where to draw (your update() normalizes by 1600x900)
    tr.Pos = { 800.0f, 450.0f };   // center of a 1600x900 “virtual” screen
    tr.Scale = { 128.0f, 128.0f };   // size in pixels in that same space
    tr.angle_disp = 0.0f;           // degrees
    tr.angle_speed = 0.0f;           // no rotation

    // model & shader indices
    tr.mdl_ref = 0.0f;               // bg_model()
    tr.shd_ref = 0.0f;               // test shader

    // sprite-sheet / animation
    tr.rows = 1;
    tr.columns = 1;
    tr.currentFrame = 0;
    tr.animationSpeed = 0.0f;        // static

    tr.isVisible = false;
    tr.autoHide = false;
    tr.visibilityTimer = 0.0f;

    auto* g_coordinator = Coordinator::GetInstance();
    int entity = g_coordinator->CreateEntity();
    g_coordinator->AddComponent(entity, tr);  // <-- add Transform

    // ----------------- Main Game Loop -----------------
    while (!glfwWindowShouldClose(Engine->GetWindow()))
    {
        glfwPollEvents();
        
        Engine->GameLoop(); // normal engine loop
    }

    Engine->Exit();

    // Clean up
    delete Engine;
    Engine = nullptr;

#if 0
    // ----------------- Memory leak check -----------------
    _CrtMemCheckpoint(&sNew);
    if (_CrtMemDifference(&sDiff, &sOld, &sNew))
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
