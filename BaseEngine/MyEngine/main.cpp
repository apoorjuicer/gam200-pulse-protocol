// ==================================================================================
// main.cpp
// Authors: Team Bloop - Rachael (Window Setup), Chloe Lau Rey En (Input System)
//          Pin Kai, Benjamin, Reginald, Carrie, Fenny, Jun Yong (TBA)
// Brief: Main application entry point for custom game engine. Currently demonstrates
//        window creation and input system. Additional systems will be integrated 
//        as team completes their respective components.
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

int main() {
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // Setup ECS and register components/systems
    Engine.Setup();

    // Initialize the engine (creates window, OpenGL context, initializes systems)
    Engine.Init();

    // Create a test entity with Transform component
    Framework::Transform tr{};

    // Set transform properties for rendering
    tr.Pos = { 800.0f, 450.0f };     // center of a 1600x900 virtual screen
    tr.Scale = { 128.0f, 128.0f };   // size in pixels
    tr.angle_disp = 0.0f;            // degrees
    tr.angle_speed = 0.0f;           // no rotation

    // model & shader indices
    tr.mdl_ref = 0.0f;               // box_model
    tr.shd_ref = 0.0f;               // sprite shader

    // sprite-sheet / animation
    tr.rows = 1;
    tr.columns = 1;
    tr.currentFrame = 0;
    tr.animationSpeed = 0.0f;        // static

    // texture ID (0 for colored quad)
    tr.textureID = 0;

    // Create entity and add Transform component
    int entity = g_coordinator.CreateEntity();
    g_coordinator.AddComponent(entity, tr);

    std::cout << "Engine setup complete. Starting game loop..." << std::endl;
    std::cout << "Controls: Q, W, E for rhythm input. Mouse for aiming." << std::endl;
    std::cout << "Combos: Q->W (Basic Strike), Q->E (Swift Slash), Q->W->E (Power Combo)" << std::endl;

    // Start the main game loop
    Engine.GameLoop();

    // Clean up
    Engine.Exit();

#if 0
    _CrtMemCheckpoint(&sNew); //take a snapshot 
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