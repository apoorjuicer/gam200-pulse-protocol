#include <windows.h>   // for OutputDebugString, etc.
#include "pch/pch.h"
#pragma comment(lib, "winmm.lib")

//Coordinator g_coordinator;
//extern InputManager* g_inputSystem;
//extern GraphicsManager* g_graphicsSystem;


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
		spdlog::error("Failed to initialize GLFW");
		return;
	}

	// --- Logging Setup ---
	try {
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
			"logs/engine.log", 1024 * 1024 * 5, 3);
		// 5 MB max per file, keep 3 backups

		std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink };
		auto logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());
		spdlog::set_default_logger(logger);

		spdlog::set_level(spdlog::level::debug); // debug/info/warn/error
		spdlog::flush_on(spdlog::level::info);

		spdlog::info("=== Engine Logging Initialized ===");
	}
	catch (const spdlog::spdlog_ex& ex) {
		std::cerr << "Log init failed: " << ex.what() << std::endl;
	}
	// ----------------------
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle, nullptr, nullptr);
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

    // Singleton for Engine, but once game use EventBus/messaging system

    // PLEASE CALL THIS EVERY SINGLE U WANT 2 USE SINGLETON
    auto g_coordinator = Coordinator::GetInstance();

	auto g_inputSystem = g_coordinator->GetSystem<InputManager>();
	auto g_audioSystem = g_coordinator->GetSystem<AudioManager>();
	auto g_graphicsSystem = g_coordinator->GetSystem<GraphicsManager>();
    auto g_CollisionManager = g_coordinator->GetSystem<CollisionManager>();


	// Initialize systems AFTER GL is ready
	g_inputSystem->Initialize(window);
	g_graphicsSystem->Initialize();
	g_audioSystem->Initialize(); 
	g_coordinator->InitializeAllSystems();



    std::cout << "Engine initialized." << std::endl;
    std::cout << "\n=== M1 INPUT SYSTEM DEMO ===" << std::endl;
    std::cout << "SPACE -> Primary rhythm beat input" << std::endl;
    std::cout << "A/S/D -> Weapon combo keys" << std::endl;
    std::cout << "MOUSE -> UI interaction testing" << std::endl;
    std::cout << "Close window to exit testing" << std::endl;

    GameRunning = true;

}

void CoreEngine::Setup()
{
	// 3 in 1 system
	Coordinator::GetInstance()->init();

    auto g_coordinator = Coordinator::GetInstance();

	// Register your component here
	g_coordinator->RegisterComponent<Framework::Transform>();
    g_coordinator->RegisterComponent<NPC>();
    g_coordinator->RegisterComponent<AABB>();
	
	// Register your systems here
	g_coordinator->RegisterSystem<InputManager>();
	g_coordinator->RegisterSystem<AudioManager>();
	g_coordinator->RegisterSystem<GraphicsManager>();
    g_coordinator->RegisterSystem<CollisionManager>();
	g_coordinator->RegisterSystem<SceneManager>();


    // Set signature for component and system
    Signature collisionSig;
    collisionSig.set(g_coordinator->GetComponentType<NPC>());
    collisionSig.set(g_coordinator->GetComponentType<AABB>());
    g_coordinator->SetSystemSignature<CollisionManager>(collisionSig);

	Signature inputSig;
	inputSig.set(g_coordinator->GetComponentType<Framework::Transform>());
	g_coordinator->SetSystemSignature<InputManager>(inputSig);

	Signature graphicsSig;
	graphicsSig.set(g_coordinator->GetComponentType<Framework::Transform>());  // <-- needs Transform
	g_coordinator->SetSystemSignature<GraphicsManager>(graphicsSig);

	Signature sceneSig;
	sceneSig.set(g_coordinator->GetComponentType<NPC>());
	sceneSig.set(g_coordinator->GetComponentType<AABB>());
	sceneSig.set(g_coordinator->GetComponentType<Framework::Transform>());  
	g_coordinator->SetSystemSignature<GraphicsManager>(sceneSig);
}

void CoreEngine::Update(float dt)
{
    auto g_coordinator = Coordinator::GetInstance();
	auto g_audioSystem = g_coordinator->GetSystem<AudioManager>();
	auto g_inputSystem = g_coordinator->GetSystem<InputManager>();
	auto g_graphicsSystem = g_coordinator->GetSystem<GraphicsManager>();
	auto g_sceneSystem = g_coordinator->GetSystem<SceneManager>();

	if (g_graphicsSystem) {
		g_graphicsSystem->Update();
	}

	if (g_inputSystem) {
		// Update input system
		g_inputSystem->Update();
		//g_audioSystem->Update();  
	}


	if (g_sceneSystem->GrabCurrentScene() == Scene::Pause)
	{
		g_sceneSystem->LoadScene(Scene::MainMenu);
	}

	g_coordinator->UpdateAllSystems(dt);
}


void CoreEngine::GameLoop()
{
    LastTime = timeGetTime();
    //init
    const float printInterval = 0.2f;
    float printTimer = 0.0f;
    
    bool spawnFromLeft = true; //true = start left move right,  false = start right move left
    bool automaticSwordSwing = true; // true = sword swinging, false = not swinging
    bool testBoxLineCollision = false; //true = boxline collision, false = monsterplayer

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
		#ifdef ENABLE_EDITOR
		MyEditor::NewFrame();
		#endif

        //  4. game update(NOT A WHILE LOOP) gsm

        //  5. game rendering
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

        //  6. ImGui rendering
		#ifdef ENABLE_EDITOR
		MyEditor::Draw();
		MyEditor::Render();
		#endif

        //7. Swap Buffer(like two window)
        glfwSwapBuffers(window);
        
    }
}

void CoreEngine::Exit()
{
    auto g_coordinator = Coordinator::GetInstance();

	auto g_inputSystem = g_coordinator->GetSystem<InputManager>();
	auto g_audioSystem = g_coordinator->GetSystem<AudioManager>();
	auto g_graphicsSystem = g_coordinator->GetSystem<GraphicsManager>();

	if (g_inputSystem) {
		g_inputSystem->Shutdown();
	}

	if (g_audioSystem) {
		g_audioSystem->Shutdown();
	}

	if (g_graphicsSystem) {
		g_graphicsSystem->Shutdown();
	}

    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}




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

//class Settings {};
//class Preferences {};
//
//
//class PhysicsManager
//{
//
//public:
//    PhysicsManager(Settings& _settings) : settings(_settings) {};
//};
//
//class Physics
//{
//    Settings& settings;
//
//    PhysicsManager physicsManager;
//
//public:
//    Physics(Settings& _settings) : physicsManager(_settings) {};
//
//};
//class Systems
//{
//    Physics physics_System;
//
//public:
//    Systems(Settings& _settings) : physics_System(_settings) {};
//};
//class Engine
//{
//    Settings settings;
//    Preferences prefs;
//    Systems systems;
//
//    Engine() : systems(settings) {};
//};
