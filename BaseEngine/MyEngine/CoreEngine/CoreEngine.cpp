#include "pch/pch.h"
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

	// Display M1 test instructions
	std::cout << "\n=== M1 INPUT SYSTEM DEMONSTRATION ===" << std::endl;
	std::cout << "SPACE     -> Primary rhythm beat input" << std::endl;
	std::cout << "A/S/D     -> Weapon combo keys (spear/sword/bow)" << std::endl;
	std::cout << "MOUSE     -> UI interaction testing" << std::endl;
	std::cout << "Close window to exit testing" << std::endl;
	std::cout << "=======================================" << std::endl;



}

void CoreEngine::Update()
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



	/*while (GameRunning)
	{
		for (unsigned i = 0; i < Systems.size(); ++i)
			Systems[i]->Update(dt);
	}*/


}

void CoreEngine::Exit()
{
	// ask chloe how to test it 
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