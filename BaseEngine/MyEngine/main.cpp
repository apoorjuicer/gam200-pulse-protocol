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
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "pch/pch.h"

#if (_DEBUG)
// MEM LEAKS LIBRARIES (See _CrtMemCheckpoint for examples to check for mem leaks)
#include "windows.h"
#define _CRTDBG_MAP_ALLOC //to get more details
#include <cstdlib>  
#include <crtdbg.h>   //for malloc and free
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
        // spdlog
} gamestate;


#ifdef _DEBUG
//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//_CrtSetBreakAlloc(15046); // Uncomment and replace param with the allocation number to break on specific leaks

_CrtMemState sOld;
_CrtMemState sNew;
_CrtMemState sDiff;
#endif

InputManager* g_inputSystem = nullptr;
//CoreEngine* engine = new CoreEngine();

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

void update()
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
    _CrtMemCheckpoint(&sOld); // Take initial snapshot (its like lock in the state of memory)
#endif

    if (!glfwInit()) {
        return -1;
    }

    // Size of game window here. 1400 x 800 is just a temp, we will change to get from resource manager which gets from text file
    GLFWwindow* window = glfwCreateWindow(Game_Window_Height, Game_Window_Width, WindowTitle, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    g_inputSystem = new InputManager();
    initEngine();

    g_inputSystem->Initialize(window);

    //engine_initialize();
    //engine_run() in here are vvv
    // This will be where the application loop is. No more game loop, just the update
    while (!glfwWindowShouldClose(window)) {
        //  1. Poll event(input key)
        //  2. update engine cpp
        //  3. ImGui update
        //  4. game update(NOT A WHILE LOOP)
        //  5. game rendering
        //  6. ImGui rendering
        //  7. Swap Buffer(like two window)

        glfwPollEvents();
        update();
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //engine_exit(); the cleanup all in here

    // CLEANUP InputManager
    if (g_inputSystem) {
        g_inputSystem->Shutdown();
        delete g_inputSystem;
        g_inputSystem = nullptr;
    }

    glfwDestroyWindow(window);
    glfwTerminate();


#ifdef _DEBUG
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