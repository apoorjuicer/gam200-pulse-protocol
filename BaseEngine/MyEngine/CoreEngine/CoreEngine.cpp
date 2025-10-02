/******************************************************************************/
/**
 * @file       CoreEngine.cpp
 * @project    Pulse Protocol
 * @author     Chia Wei Xuan Rachael (primary) - 50%
 * @author     Chloe, Ben, Reginald, Pin Kai, Jun Yong (secondary) - 10% each
 * @brief      Implementation of the core engine class that initializes all
 *             engine subsystems (graphics, input, audio, physics), manages
 *             the main game loop with delta time calculation, handles system
 *             updates, integrates ImGui editor support, and provides debug
 *             rendering utilities for development visualization.
 * @copyright  Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/


#include <windows.h>   // for OutputDebugString, etc.
#include "pch/pch.h"
#include "graphics/stb_image.h"
#include "Core/Log.h"
#pragma comment(lib, "winmm.lib")

//Coordinator g_coordinator;
//extern InputManager* g_inputSystem;
//extern GraphicsManager* g_graphicsSystem;


// Game window dimensions and title
const char WindowTitle[] = "Game Window";
 Config config;
 int WindowWidth;
 int WindowHeight;

//void APIENTRY glDebugOutput(GLenum source,
//	GLenum type,
//	unsigned int id,
//	GLenum severity,
//	GLsizei length,
//	const char* message,
//	const void* userParam)
//{
//	// ignore non-significant error/warning codes
//	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
//
//	std::cout << "---------------" << std::endl;
//	std::cout << "Debug message (" << id << "): " << message << std::endl;
//
//	switch (source)
//	{
//	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
//	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
//	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
//	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
//	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
//	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
//	} std::cout << std::endl;
//
//	switch (type)
//	{
//	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
//	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
//	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
//	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
//	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
//	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
//	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
//	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
//	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
//	} std::cout << std::endl;
//
//	switch (severity)
//	{
//	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
//	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
//	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
//	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
//	} std::cout << std::endl;
//	std::cout << std::endl;
//}

// THIS FUNCTION IS TO BE REMOVED AFTER RESOURCE MANAGER HAS BEEN TESTED AND WORKING.
//static GLuint LoadTexture(const char* filepath) {
//	stbi_set_flip_vertically_on_load(true); // flip image vertically when loading
//
//	int width, height, channels;
//	unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
//
//	if (!data) {
//		std::cerr << "Failed to load texture: " << filepath << std::endl;
//		return 0;
//	}
//
//	GLuint textureID;
//	glGenTextures(1, &textureID);
//	glBindTexture(GL_TEXTURE_2D, textureID);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//	stbi_image_free(data);
//	return textureID;
//}

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
	/*Config load window width and height*/
	if (!LoadConfig(config, config.GetConfigPath())) {
		std::cerr << "Failed to load config!\n";
	}
	else {
		std::cout << "Load config From FilePath: " << std::filesystem::absolute(config.GetConfigPath()) << "\n";
	}

	WindowWidth = config.GetWindowWidth();
	WindowHeight = config.GetWindowHeight();

	std::cout << "WindowWidth is: " << WindowWidth <<"\n";
	std::cout << "WindowHeight is: " << WindowHeight << "\n";

	if (!glfwInit()) {
		spdlog::error("Failed to initialize GLFW");
		return;
	}

	//// --- Logging Setup ---
	//try {
	//	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	//	auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
	//		"logs/engine.log", 1024 * 1024 * 5, 3);
	//	// 5 MB max per file, keep 3 backups

	//	std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink };
	//	auto logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());
	//	spdlog::set_default_logger(logger);

	//	spdlog::set_level(spdlog::level::debug); // debug/info/warn/error
	//	spdlog::flush_on(spdlog::level::info);

	//	spdlog::info("=== Engine Logging Initialized ===");
	//}
	//catch (const spdlog::spdlog_ex& ex) {
	//	std::cerr << "Log init failed: " << ex.what() << std::endl;
	//}
	//// ----------------------
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

	// Initialize DebugDraw AFTER glewInit()
	DebugDraw::Init();

	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		//glDebugMessageCallback(glDebugOutput, nullptr);
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

	//GLuint barTexture = LoadTexture("../../MyEngine/Assets/Sprites/perfect.png");
	//if (barTexture > 0) {
	//	Entity box2 = g_inputSystem->GetFeedbackEntity2();  // Use getter
	//	auto& transform2 = g_coordinator->GetComponent<Framework::Transform>(box2);
	//	transform2.textureID = barTexture;
	//	std::cout << "Texture loaded and assigned to box 2: " << barTexture << std::endl;
	//}

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
	g_coordinator->RegisterSystem<GraphicsManager>();
	g_coordinator->RegisterSystem<AudioManager>();
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

	if (g_graphicsSystem) g_graphicsSystem->Update();
	if (g_inputSystem) g_inputSystem->Update(dt);

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

	auto profiler = ProfilingManager::GetInstance();

	// application loop 
	static bool keystateP = false;
	static bool PKeyWasPressed = false;
	while(!glfwWindowShouldClose(window) && GameRunning)
	{
		//Get the current time in milliseconds
		unsigned currenttime = timeGetTime();
		//Convert it to the time passed since the last frame (in seconds)
		float dt = (currenttime - LastTime) / 1000.0f;
		//Update the when the last update started
		LastTime = currenttime;

		profiler->StartFrame();


        //  1. Poll event(input key)
        glfwPollEvents();
		// Read key state
		bool PKeyPressed = (glfwGetKey(Engine->GetWindow(), GLFW_KEY_P) == GLFW_PRESS);

		// Toggle on key press
		if (PKeyPressed && !PKeyWasPressed) {
			keystateP = !keystateP;
			if (keystateP)
				GAM200_INFO("Crash Test ENABLED (press P again to disable).");
			else
				GAM200_INFO("Crash Test DISABLED.");
		}
		PKeyWasPressed = PKeyPressed;


		// ----------------- Execute crash test if toggle is ON -----------------
		if (keystateP)
		{
			try
			{
				GAM200_CORE_INFO("Starting crash test triggered by P key...");

				int a = 5;
				int b = 0;
				if (b == 0)
					GAM200_CRITICAL("Test crash: Division by zero detected!"); // logs to CrashLog.txt

				int c = a / b; // won't execute
				(void)c;

			}
			catch (const std::exception& e)
			{
				GAM200_CORE_ERROR("Exception caught during crash test: {0}", e.what());
			}

			// Reset toggle to prevent repeat execution
			//keystateP = false;
		}
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

		RenderDebugDraw();

        //7. Swap Buffer(like two window)
        glfwSwapBuffers(window);

		profiler->EndFrame();

		// Print profiling stats every 2 secs
		printTimer += dt;
		if (printTimer >= 1.0f) {
			printTimer = 0.0f;
			profiler->PrintStats();
			profiler->Reset();
		}
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

	DebugDraw::Shutdown();

    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

void CoreEngine::RenderDebugDraw() {
	auto g_coordinator = Coordinator::GetInstance();
	auto g_inputSystem = g_coordinator->GetSystem<InputManager>();

	if (!g_inputSystem) return;

	glm::mat4 projection = glm::ortho(-800.0f, 800.0f, -450.0f, 450.0f);
	DebugDraw::Begin(projection);

	// F1 - DrawRect
	if (g_inputSystem->IsDebugRectsEnabled()) {
		for (auto& entity : g_inputSystem->EntityMember) {
			auto& transform = g_coordinator->GetComponent<Framework::Transform>(entity);
			glm::vec2 center = { transform.Pos.x, transform.Pos.y };
			glm::vec2 half = 0.5f * glm::vec2(transform.Scale.x, transform.Scale.y);

			glm::vec2 min = center - half;
			glm::vec2 max = center + half;
			if (min.x > max.x) std::swap(min.x, max.x);
			if (min.y > max.y) std::swap(min.y, max.y);

			DebugDraw::DrawRect(min, max, { 1.0f, 0.0f, 0.0f });
		}
	}


	// F2 - DrawLine
	if (g_inputSystem->IsDebugLinesEnabled()) {
		DebugDraw::DrawLine({ -800.0f, -450.0f }, { 800.0f, 450.0f }, { 1.0f, 0.0f, 0.0f });
		DebugDraw::DrawLine({ 800.0f, -450.0f }, { -800.0f, 450.0f }, { 0.0f, 1.0f, 0.0f });
		DebugDraw::DrawLine({ 0.0f, -450.0f }, { 0.0f, 450.0f }, { 0.0f, 0.0f, 1.0f });
		DebugDraw::DrawLine({ -800.0f, 0.0f }, { 800.0f, 0.0f }, { 1.0f, 1.0f, 0.0f });
	}

	// F3 - DrawCircle
	if (g_inputSystem->IsDebugCirclesEnabled()) {
		DebugDraw::DrawCircle({ 0.0, 0.0f }, 200.0f, { 1.0f, 0.0f, 1.0f }, 32);
		DebugDraw::DrawCircle({ 400.0f, 300.0f }, 100.0f, { 0.0f, 1.0f, 1.0f }, 24);
	}

	// F4 - DrawCircleFilled
	if (g_inputSystem->IsDebugCirclesFilledEnabled()) {
		DebugDraw::DrawCircleFilled({ 100.0f, 300.0f }, 80.0f, { 1.0f, 0.5f, 0.0f }, 24);
		DebugDraw::DrawCircleFilled({ 100.0f, -300.0f }, 60.0f, { 0.5f, 0.0f, 1.0f }, 16);
	}

	// F5 - DrawPoint
	if (g_inputSystem->IsDebugPointsEnabled()) {
		for (float x = -800.0f; x < 800.0f; x += 200.0f) {
			for (float y = -450.0f; y < 450.0f; y += 200.0f) {
				DebugDraw::DrawPoint({ x, y }, 5.0f, { 1.0f, 1.0f, 0.0f });
			}
		}
	}

	// F6 - Grid
	if (g_inputSystem->IsDebugGridEnabled()) {
		for (float x = -800.0f; x < 800.0f; x += 100.0f) {
			DebugDraw::DrawLine({ x, -450.0f }, { x, 450.0f }, { 0.3f, 0.3f, 0.3f });
		}
		for (float y = -450.0f; y < 450.0f; y += 100.0f) {
			DebugDraw::DrawLine({ -800.0f, y }, { 800.0f, y }, { 0.3f, 0.3f, 0.3f });
		}
	}

	DebugDraw::End();
}
