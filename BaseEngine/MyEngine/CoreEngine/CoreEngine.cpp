#include "pch/pch.h"

// global pointer to the input system
extern InputManager* g_inputSystem;

//A global pointer to the core
CoreEngine* Engine;
int CoreEngine::WindowWidth;
int CoreEngine::WindowHeight;


CoreEngine::CoreEngine()
{

}

CoreEngine::~CoreEngine()
{

}

void CoreEngine::Init()
{
	glewInit();
	std::cout << "Engine initialized." << std::endl;

	// test instructions for input (for now)
	std::cout << "\n=== INPUT GUIDE ===" << std::endl;
	std::cout << "Q/W/E -> Rhythm keys" << std::endl;
	std::cout << "Combos: Q->W, Q->E, Q->W->E" << std::endl;
	std::cout << "Mouse: Right=aim, Left=attack" << std::endl;
	std::cout << "====================" << std::endl;



}

void CoreEngine::Update()
{
	if (g_inputSystem) {
		// Update input system
		g_inputSystem->Update();
	}

	// Update rest of the game systems efficiently with delta time
	/*while (GameRunning)
	{
		for (unsigned i = 0; i < Systems.size(); ++i)
			Systems[i]->Update(dt);
	}*/


}

void CoreEngine::Exit()
{
	// 1. Shutdown() displays final stats and cleans internal state
	// 2. delete frees memory and prevents leaks
	// 3. nullptr prevents dangling pointer access
	if (g_inputSystem) {
		g_inputSystem->Shutdown();
		delete g_inputSystem;
		g_inputSystem = nullptr;
	}
}

// where application loop is. Just gonna name is game loop since its easier to understand
void CoreEngine::GameLoop()
{
		
}

//void BroadcastMessage(){}






// STILL BUILDING IM SORRY ;-;
//// MyEditor.cpp
//void imgui_update() {
//    // imgui logic here
//
//    if (ImGui::Button("Butt")) {
//        EngineCreateEntity();
//    }
//}
//
//void (*pre_game_update)() = imgui_update;
//
//// MyEngine.cpp
//void engine_run() {
//    while (window_open) {
//        window_update();
//        input_update();
//        time_update();
//
//        // pre_game_update();
//        imgui_update();
//
//        if (state == LEVEL_LOADING) {
//            gsm_game_load();
//            state = LEVEL_UPDATE;
//        }
//
//        if (state == LEVEL_UPDATE) {
//            gsm_game_update();
//            // if (key_down(key_space)) {
//            //  state = LEVEL_CHANGE;
//            //}
//        }
//
//        if (state == LEVEL_CHANGE) {
//            gsm_game_unload();
//            state = LEVEL_LOADING;
//        }
//
//        game_render();
//        imgui_render();
//        window_swap_buffers();
//    }
//}