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

InputManager* g_inputSystem = nullptr;
AudioManager* g_audioSystem = nullptr; 

void initEngine() {
    glewInit();
    std::cout << "Engine initialized." << std::endl;


    // Display M1 test instructions
    std::cout << "\n=== M1 INPUT SYSTEM DEMONSTRATION ===" << std::endl;
    std::cout << "Q/W/E/R        -> Rhythm beat inputs (core gameplay)" << std::endl;
    std::cout << "1/2/3          -> Weapon switching (Spear/Sword/Bow)" << std::endl;
    std::cout << "RIGHT MOUSE    -> Aim mode (hold)" << std::endl;
    std::cout << "LEFT MOUSE     -> Attack/Shoot or UI interaction" << std::endl;
    std::cout << "MOUSE MOVE     -> Aiming direction" << std::endl;
    std::cout << "Close window to exit testing" << std::endl;
    std::cout << "===========================================" << std::endl;

}

void update() //Is this being called since there is another engine.updated? - Jun Yong
{
    if (g_inputSystem) {
        // Update input system 
        g_inputSystem->Update();
   

        // Run M1 testing demonstrations
        g_inputSystem->RunM1Test();

        // Future game systems would be updated here:
        // gameManager.Update();
        // physics.Simulate();
    }

   

}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    // You can add more rendering code here
}

int main() {
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // Setup ECS and register components/systems
    Engine.Setup();

    g_audioSystem = new AudioManager(); 

    Engine.Init(); //for all the system init yeah? where the gsm load and init

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

    std::vector<Entity> entities(MaxEntity);
    int i = 0;
    for (auto& x : entities)
    {
        //std::cout << i++;
    }
    
    //class Position;
    //class Velocity;


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

    glfwMakeContextCurrent(window);
    g_inputSystem = new InputManager();
   
    //initEngine();
    g_inputSystem->Initialize(window);
    

    
    
    //Engine.Update(); in here are vvv
    // This will be where the application loop is. No more game loop, just the update
    while (!glfwWindowShouldClose(window)) {
        //  1. Poll event(input key)
        //  2. update engine cpp
        //  3. ImGui update
        //  4. game update(NOT A WHILE LOOP)
        //  5. game rendering
        //  6. ImGui rendering
        //  7. Swap Buffer(like two window)
        
        
        Player& player = g_coordinator.GetComponent<Player>(entity);
        //std::cout << "Player: " << player.x << ", " << player.y << std::endl;
        //std::cout << "Player: " << player.position.x << ", " << player.position.y << std::endl;
 
        /*if ()
        {
            std::cout << "Player: " << ++player.x << ", " << ++player.y << std::endl;
        }*/



        glfwPollEvents();
        Engine.Update();
        render();
        glfwSwapBuffers(window);
    }

    Engine.Exit(); 
    
    
    // Engine.exit already calls the 2 below, is it still needed????
    // CLEANUP InputManager
    //if (g_inputSystem) {
    //    g_inputSystem->Shutdown();
    //    delete g_inputSystem;
    //    g_inputSystem = nullptr;
    //}

    //if (g_audioSystem) {
    //    g_audioSystem->Shutdown();
    //    delete g_audioSystem;
    //    g_audioSystem = nullptr;
    //}

    glfwDestroyWindow(window);
    glfwTerminate();

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