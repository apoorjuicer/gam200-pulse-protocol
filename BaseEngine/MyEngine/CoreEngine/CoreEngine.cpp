#include <windows.h>   // for OutputDebugString, etc.
#include "pch/pch.h"
#pragma comment(lib, "winmm.lib")

//Coordinator g_coordinator;
//extern InputManager* g_inputSystem;
//extern GraphicsManager* g_graphicsSystem;

//A global pointer to the core
CoreEngine* Engine;
extern Coordinator g_coordinator;


// Game window dimensions and title
const char WindowTitle[] = "Game Window";

void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

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

	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return;
	}

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(WindowHeight, WindowWidth, WindowTitle, nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(window);
	glewInit();

	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	// Create systems AFTER GL is ready
	//g_inputSystem = new InputManager();
	//g_graphicsSystem = new GraphicsManager();
	auto g_inputSystem = g_coordinator.GetSystem<InputManager>();
	auto g_graphicsSystem = g_coordinator.GetSystem<GraphicsManager>();
	auto g_audioSystem = g_coordinator.GetSystem<AudioManager>();

	// Initialize systems AFTER GL is ready
	g_inputSystem->Initialize(window);
	g_inputSystem->RegisterInputListener("CoreEngine", [](const InputEvent& event) {
		if (event.type == InputEvent::Type::RHYTHM_INPUT) {
			auto g_inputSystem = g_coordinator.GetSystem<InputManager>();
			if (!g_inputSystem || !g_inputSystem->CanInput()) {
				std::cout << "[GAME OVER] No more chances!" << std::endl;
				return; // ignore input if game over
			}

			auto& boxTransform = g_coordinator.GetComponent<Framework::Transform>(0);
			boxTransform.isVisible = true;
			boxTransform.visibilityTimer = 0.5f;
			boxTransform.autoHide = true;

			auto g_audioSystem = g_coordinator.GetSystem<AudioManager>();
			if (g_audioSystem) {
				g_audioSystem->PlayInputSFX(1.0f);
			}
		}
	});

	g_graphicsSystem->Initialize();
	g_audioSystem->Initialize(); 
	g_coordinator.InitializeAllSystems();

	std::cout << "Engine initialized." << std::endl;
	std::cout << "Input system ready for rhythm gameplay." << std::endl;

	GameRunning = true;
}

// add/register your systems and component here  for our games here 
void CoreEngine::Setup()
{
	// 3 in 1 system
	g_coordinator.init();

	// Register your component here
	g_coordinator.RegisterComponent<Framework::Transform>();

	// Register your systems here
	g_coordinator.RegisterSystem<InputManager>();
	g_coordinator.RegisterSystem<GraphicsManager>();
	g_coordinator.RegisterSystem<AudioManager>();

	// Set your signatures for both component and system
	Signature graphicsSig;
	graphicsSig.set(g_coordinator.GetComponentType<Framework::Transform>());  // <-- needs Transform
	g_coordinator.SetSystemSignature<GraphicsManager>(graphicsSig);
}

void CoreEngine::Update(float dt)
{
	auto& boxTransform = g_coordinator.GetComponent<Framework::Transform>(0);
	if (boxTransform.autoHide && boxTransform.isVisible) {
		boxTransform.visibilityTimer -= dt;
		if (boxTransform.visibilityTimer <= 0.0f) {
			boxTransform.isVisible = false;
			boxTransform.autoHide = false;
		}
	}

	auto g_inputSystem = g_coordinator.GetSystem<InputManager>();
	auto g_graphicsSystem = g_coordinator.GetSystem<GraphicsManager>();
	auto g_audioSystem = g_coordinator.GetSystem<AudioManager>();

	if (g_inputSystem) {
		// Update input system
		g_inputSystem->Update();
	}
	if (g_graphicsSystem) {
		g_graphicsSystem->Update();
	}
		//g_audioSystem->Update();  

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
	while(!glfwWindowShouldClose(window) && GameRunning)
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
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

		//  6. ImGui rendering

		//7. Swap Buffer(like two window)
		glfwSwapBuffers(window);
	}


}


void CoreEngine::Exit()
{
	auto g_inputSystem = g_coordinator.GetSystem<InputManager>();
	auto g_graphicsSystem = g_coordinator.GetSystem<GraphicsManager>();
	auto g_audioSystem = g_coordinator.GetSystem<AudioManager>();

	if (g_inputSystem) {
		g_inputSystem->Shutdown();
	}

	if (g_audioSystem) {
		g_audioSystem->Shutdown();

	}
	if (g_graphicsSystem) {
		g_graphicsSystem->Shutdown();
	}

	//if (g_audioSystem) {
	//	g_audioSystem->Shutdown();
	//}

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



// ECS setup
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
