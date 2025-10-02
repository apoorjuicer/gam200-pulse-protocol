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
const int Game_Window_Height = 900;
const int Game_Window_Width = 1600;

Matrix3x3 beforeTranpose = {1.f,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f};
Matrix3x3 A{};


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
//InputManager* g_inputSystem = nullptr;
//GraphicsManager* g_graphicsSystem = nullptr;

int main() {
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // Create the Engine instance
    Engine = new CoreEngine();

    Engine->Setup();
    // If CoreEngine::Init() also calls glewInit() or GraphicsManager->Initialize(),
    // REMOVE those calls from CoreEngine::Init() or call Engine.Init() *here* only
    // for non-GL things:
    Engine->Init(); // <-- only if it does NOT touch OpenGL

    // After you create the GLFW window and GL context
    #ifdef ENABLE_EDITOR
    MyEditor::Init(Engine->GetWindow());  // or your window pointer
    #endif

    // this is where the update() is 
    Engine->GameLoop();


    #ifdef ENABLE_EDITOR
    MyEditor::Shutdown();
    #endif

    Engine->Exit();

    // Clean up
    delete Engine;
    Engine = nullptr;

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