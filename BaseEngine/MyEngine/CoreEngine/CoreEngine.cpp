#include <windows.h>   // for OutputDebugString, etc.
#include "pch/pch.h"
#pragma comment(lib, "winmm.lib")

//Coordinator g_coordinator;
extern InputManager* g_inputSystem;
extern GraphicsManager* g_graphicsSystem;

//A global pointer to the core
CoreEngine* Engine;
extern Coordinator g_coordinator;


// Game window dimensions and title
const char WindowTitle[] = "Game Window";
const int WindowHeight = 1400;
const int WindowWidth = 800;

CoreEngine::CoreEngine() : window(nullptr)
{
	LastTime = 0;
	GameRunning = false;
}

CoreEngine::~CoreEngine()
{
	if (window)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}


void CoreEngine::Init()
{
	// 3 in 1 system
	g_coordinator.init();

	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return;
	}

	window = glfwCreateWindow(WindowHeight, WindowWidth, WindowTitle, nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(window);
	glewInit();

	// Create systems AFTER GL is ready
	g_inputSystem = new InputManager();
	g_graphicsSystem = new GraphicsManager();

	// Initialize systems AFTER GL is ready
	g_inputSystem->Initialize(window);
	g_graphicsSystem->Initialize();
	g_coordinator.InitializeAllSystems();
	std::cout << "Engine initialized." << std::endl;

	// Display M1 test instructions
	std::cout << "\n=== M1 INPUT SYSTEM DEMONSTRATION ===" << std::endl;
	std::cout << "SPACE     -> Primary rhythm beat input" << std::endl;
	std::cout << "A/S/D     -> Weapon combo keys (spear/sword/bow)" << std::endl;
	std::cout << "MOUSE     -> UI interaction testing" << std::endl;
	std::cout << "Close window to exit testing" << std::endl;
	std::cout << "=======================================" << std::endl;

	GameRunning = true;
}

// add/register your systems and component here  for our games here 
void CoreEngine::Setup()
{
	// Register your component here
	g_coordinator.RegisterComponent<Framework::Transform>();

	// Register your systems here
	g_coordinator.RegisterSystem<InputManager>();
	g_coordinator.RegisterSystem<GraphicsManager>();

	// Set your signatures for both component and system
	Signature graphicsSig;
	graphicsSig.set(g_coordinator.GetComponentType<Framework::Transform>());  // <-- needs Transform
	g_coordinator.SetSystemSignature<GraphicsManager>(graphicsSig);
}

void CoreEngine::Update(float dt)
{
	if (g_inputSystem) {
		// Update input system
		g_inputSystem->Update();

		// Run M1 testing demonstrations
		g_inputSystem->RunM1Test();

	}
	if (g_graphicsSystem) {
		g_graphicsSystem->Update();
	}

	g_coordinator.UpdateAllSystems(dt);
}

// Ask pin kai if he needs this. 
void CoreEngine::Render()
{

}

void CoreEngine::GameLoop()
{
	LastTime = timeGetTime();

	// application loop
	while (!glfwWindowShouldClose(window) && GameRunning)
	{
		//Get the current time in milliseconds
		unsigned currenttime = timeGetTime();
		//Convert it to the time passed since the last frame (in seconds)
		float dt = (currenttime - LastTime) / 1000.0f;
		//Update the when the last update started
		LastTime = currenttime;

		//  1. Poll event(input key)
		glfwPollEvents();

		//  2. update engine cpp
		Update(dt);

		//  3. ImGui update

		//  4. game update(NOT A WHILE LOOP) gsm

		//  5. game rendering
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

		//  6. ImGui rendering

		//7. Swap Buffer(like two window)
		glfwSwapBuffers(window);
	}


}


void CoreEngine::Exit()
{
	if (g_inputSystem) {
		g_inputSystem->Shutdown();
		delete g_inputSystem;
		g_inputSystem = nullptr;
	}
	if (g_graphicsSystem) {
		g_graphicsSystem->Shutdown();
		delete g_graphicsSystem;
		g_graphicsSystem = nullptr;
	}


	if (window)
	{
		glfwDestroyWindow(window);
		window = nullptr;
	}
	glfwTerminate();
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



//// ECS setup
//g_coordinator.init();
//g_coordinator.RegisterComponent<Player>();
//g_coordinator.RegisterComponent<Framework::Transform>();
//
//g_coordinator.RegisterSystem<InputManager>();
//g_coordinator.RegisterSystem<GraphicsManager>();
// 
// Signature signature;
// signature.set(g_coordinator.GetComponentType<Player>());
// g_coordinator.SetSystemSignature<InputManager>(signature);
//
//Engine.Init(); // <-- only if it does NOT touch OpenGL
//
//int entity = g_coordinator.CreateEntity();
//g_coordinator.AddComponent(entity, Player{ 100, 200 });
