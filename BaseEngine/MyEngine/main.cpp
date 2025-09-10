#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

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

void initEngine() {
    glewInit();
    std::cout << "Engine initialized." << std::endl;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    // You can add more rendering code here
}

int main() {
//#ifdef _DEBUG
//    _CrtMemCheckpoint(&sOld); // Take initial snapshot (its like lock in the state of memory)
//#endif

    if (!glfwInit()) {
        return -1;
    }

    // Size of game window here. 1400 x 800 is just a temp
    GLFWwindow* window = glfwCreateWindow(Game_Window_Height, Game_Window_Width, WindowTitle, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize engine
    initEngine();

    // Game loop goes here gamestate.IsRunning 
    while (!glfwWindowShouldClose(window)) {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Graphics cleanup 
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