/******************************************************************************/
/**
 * @file        CoreEngine.cpp
 * @project     Pulse Protocol
 * @author		Chia Wei Xuan Rachael (primary) - 50%
 * @author		Chloe, Ben, Reginald, Pin Kai, Jun Yong (secondary) - 10% each
 * @brief		Implementation of the core engine class that initializes all
 *				engine subsystems (graphics, input, audio, physics), manages
 *				the main game loop with delta time calculation, handles system
 *				updates, integrates ImGui editor support, and provides debug
 *				rendering utilities for development visualization.
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include <windows.h> // for OutputDebugString, etc.
#include "pch/pch.h"
#include "graphics/stb_image.h"
#include "CoreEngine/Log/Log.h"
#include "CoreEngine/Factory/Factory.h"
#pragma comment(lib, "winmm.lib")

// Game window dimensions and title
const char WindowTitle[] = "Game Window";
Config config;
AssetRegistry ar;
int WindowWidth;
int WindowHeight;

void APIENTRY glDebugOutput(GLenum source,
							GLenum type,
							unsigned int id,
							GLenum severity,
							GLsizei,
							const char *message,
							const void *userParam)
{
	(void)userParam;
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
		return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		std::cout << "Source: API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		std::cout << "Source: Window System";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		std::cout << "Source: Shader Compiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		std::cout << "Source: Third Party";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		std::cout << "Source: Application";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		std::cout << "Source: Other";
		break;
	}
	std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		std::cout << "Type: Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		std::cout << "Type: Deprecated Behaviour";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		std::cout << "Type: Undefined Behaviour";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		std::cout << "Type: Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		std::cout << "Type: Performance";
		break;
	case GL_DEBUG_TYPE_MARKER:
		std::cout << "Type: Marker";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		std::cout << "Type: Push Group";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		std::cout << "Type: Pop Group";
		break;
	case GL_DEBUG_TYPE_OTHER:
		std::cout << "Type: Other";
		break;
	}
	std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		std::cout << "Severity: high";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		std::cout << "Severity: medium";
		break;
	case GL_DEBUG_SEVERITY_LOW:
		std::cout << "Severity: low";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		std::cout << "Severity: notification";
		break;
	}
	std::cout << std::endl;
	std::cout << std::endl;
}

CoreEngine::CoreEngine() : window(nullptr), DebugMode(false)
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
	if (!LoadConfig(config, config.GetConfigPath()))
	{
		std::cerr << "Failed to load config!\n";
	}
	else
	{
		std::cout << "Load config From FilePath: " << std::filesystem::absolute(config.GetConfigPath()) << "\n";
	}

	WindowWidth = config.GetWindowWidth();
	WindowHeight = config.GetWindowHeight();

	std::cout << "WindowWidth is: " << WindowWidth << "\n";
	std::cout << "WindowHeight is: " << WindowHeight << "\n";

	if (!glfwInit())
	{
		spdlog::error("Failed to initialize GLFW");
		return;
	}

	// loading metadata for audio,texture,font into
	// ASset reg's container each respectively
	ar.LoadAudioManifest();
	ar.LoadTextureManifest();
	ar.LoadFontManifest();

	//// --- Logging Setup ---
	// try {
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
	// catch (const spdlog::spdlog_ex& ex) {
	//	std::cerr << "Log init failed: " << ex.what() << std::endl;
	//}
	//// ----------------------
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle, nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(window);
	glfwSetWindowAspectRatio(window, 16, 9);
	glfwSetFramebufferSizeCallback(window,
								   [](GLFWwindow *, int fbw, int fbh)
								   {
									   if (fbw <= 0 || fbh <= 0)
										   return;

									   // Letterboxed viewport + scale
									   GLApp::ComputeLetterbox(fbw, fbh);

									   WindowWidth = fbw;
									   WindowHeight = fbh;

									   if (auto g = Coordinator::GetInstance()->GetSystem<GraphicsManager>())
									   {
										   g->OnResize(fbw, fbh);
									   }
								   });

	int fbw = 0, fbh = 0;
	glfwGetFramebufferSize(window, &fbw, &fbh);
	GLApp::ComputeLetterbox(fbw, fbh);
	glViewport(0, 0, fbw, fbh);

	WindowWidth = fbw;
	WindowHeight = fbh;
	glewInit();

	// Initialize DebugDraw AFTER glewInit()
	DebugDraw::Init();

	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
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
	auto g_rhythmSystem = g_coordinator->GetSystem<RhythmGameplayInput>();
	auto g_audioSystem = g_coordinator->GetSystem<AudioManager>();
	auto g_graphicsSystem = g_coordinator->GetSystem<GraphicsManager>();
	auto g_CollisionManager = g_coordinator->GetSystem<CollisionManager>();

	MessageManager::GetInstance();
	std::cout << "[MessageManager] Initialized" << std::endl;

	// Initialize systems AFTER GL is ready
	g_inputSystem->Initialize(window);
	g_rhythmSystem->Initialize();
	g_graphicsSystem->Initialize();
	g_audioSystem->Initialize();
	g_coordinator->InitializeAllSystems();

	ProfilingManager::GetInstance()->SetResetInterval(60.0);
	ProfilingManager::GetInstance()->SetDisplayUpdateInterval(1.0);

	// GLuint barTexture = LoadTexture("../../MyEngine/Assets/Sprites/perfect.png");
	// if (barTexture > 0) {
	//	Entity box2 = g_inputSystem->GetFeedbackEntity2();  // Use getter
	//	auto& transform2 = g_coordinator->GetComponent<Framework::Transform>(box2);
	//	transform2.textureID = barTexture;
	//	std::cout << "Texture loaded and assigned to box 2: " << barTexture << std::endl;
	// }

	GameRunning = true;
	std::string filePath = "../../MyEngine/JSON/test.json"; // AssetManager.mPrefabMap[path];
	Factory factory;
	SceneManager SM;
	factory.CreateEntityFromJson(filePath);

	std::string filePath2 = "../../MyEngine/JSON/test2.json";
	Entity entity = factory.DefaultObj();
	LogicComponent l;
	l.ScriptName = "IT WORKS!!!!";
	g_coordinator->AddComponent(entity, l);
	factory.SaveEntityToJson(entity, filePath2);

	std::string filePath3 = "../../MyEngine/JSON/MainMenu.json";
	SM.LoadSceneFromJson(filePath3);
	SM.SaveCurrentScene();
}

void CoreEngine::Setup()
{
	// 3 in 1 system
	Coordinator::GetInstance()->init();

	auto g_coordinator = Coordinator::GetInstance();

	// Register your component here
	g_coordinator->RegisterComponent<Framework::Transform>();
	g_coordinator->RegisterComponent<Framework::Renderable>();
	g_coordinator->RegisterComponent<Framework::Animation>();
	g_coordinator->RegisterComponent<NPC>();
	g_coordinator->RegisterComponent<AABB>();
	g_coordinator->RegisterComponent<LogicComponent>();

	// Register your systems here, this order is IMPORTANT FOR UPDATE LOOP!!!!
	g_coordinator->RegisterSystem<InputManager>();
	g_coordinator->RegisterSystem<RhythmGameplayInput>();
	g_coordinator->RegisterSystem<GraphicsManager>();
	g_coordinator->RegisterSystem<AudioManager>();
	g_coordinator->RegisterSystem<CollisionManager>();
	g_coordinator->RegisterSystem<SceneManager>();
	g_coordinator->RegisterSystem<LogicSystem>();

	// Set signature for component and system
	Signature collisionSig;
	collisionSig.set(g_coordinator->GetComponentType<NPC>());
	collisionSig.set(g_coordinator->GetComponentType<AABB>());
	g_coordinator->SetSystemSignature<CollisionManager>(collisionSig);

	Signature inputSig;
	inputSig.set(g_coordinator->GetComponentType<Framework::Transform>());
	inputSig.set(g_coordinator->GetComponentType<Framework::Renderable>());
	inputSig.set(g_coordinator->GetComponentType<Framework::Animation>());
	g_coordinator->SetSystemSignature<InputManager>(inputSig);

	Signature rhythmSig;
	rhythmSig.set(g_coordinator->GetComponentType<Framework::Transform>());
	rhythmSig.set(g_coordinator->GetComponentType<Framework::Renderable>());
	rhythmSig.set(g_coordinator->GetComponentType<Framework::Animation>());
	g_coordinator->SetSystemSignature<RhythmGameplayInput>(rhythmSig);

	Signature graphicsSig;
	graphicsSig.set(g_coordinator->GetComponentType<Framework::Transform>());  // <-- needs Transform
	graphicsSig.set(g_coordinator->GetComponentType<Framework::Renderable>()); // <-- needs Renderable
	graphicsSig.set(g_coordinator->GetComponentType<Framework::Animation>());  // <-- needs Animation
	g_coordinator->SetSystemSignature<GraphicsManager>(graphicsSig);

	Signature sceneSig;
	sceneSig.set(g_coordinator->GetComponentType<NPC>());
	sceneSig.set(g_coordinator->GetComponentType<AABB>());
	sceneSig.set(g_coordinator->GetComponentType<Framework::Transform>());
	sceneSig.set(g_coordinator->GetComponentType<Framework::Renderable>());
	sceneSig.set(g_coordinator->GetComponentType<Framework::Animation>());
	g_coordinator->SetSystemSignature<GraphicsManager>(sceneSig);

	Signature LogicSig;
	LogicSig.set(g_coordinator->GetComponentType<LogicComponent>());
	g_coordinator->SetSystemSignature<GraphicsManager>(LogicSig);
}

void CoreEngine::Update(float dt)
{
	auto g_coordinator = Coordinator::GetInstance();
	auto g_audioSystem = g_coordinator->GetSystem<AudioManager>();
	auto g_inputSystem = g_coordinator->GetSystem<InputManager>();
	auto g_rhythmSystem = g_coordinator->GetSystem<RhythmGameplayInput>();
	auto g_graphicsSystem = g_coordinator->GetSystem<GraphicsManager>();
	auto g_sceneSystem = g_coordinator->GetSystem<SceneManager>();

	MessageManager::GetInstance()->ProcessQueue(glfwGetTime());

	if (g_graphicsSystem)
		g_graphicsSystem->Update();
	if (g_inputSystem)
		g_inputSystem->Update(dt);
	if (g_rhythmSystem)
		g_rhythmSystem->Update(dt);

	g_coordinator->UpdateAllSystems(dt);
}

void CoreEngine::GameLoop()
{
	LastTime = timeGetTime();

	auto profiler = ProfilingManager::GetInstance();

	// application loop
	static bool keystateP = false;
	static bool PKeyWasPressed = false;
	while (!glfwWindowShouldClose(window) && GameRunning)
	{
		// Get the current time in milliseconds
		unsigned currenttime = timeGetTime();

		// Convert it to the time passed since the last frame (in seconds)
		float dt = (currenttime - LastTime) / 1000.0f;

		// Update the when the last update started
		LastTime = currenttime;

		profiler->StartFrame();

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
		// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

		profiler->EndFrame();

		//  6. ImGui rendering
#ifdef ENABLE_EDITOR
		MyEditor::Draw();
		MyEditor::Render();
#endif

		RenderDebugDraw();

		// 7. Swap Buffer(like two window)
		glfwSwapBuffers(window);

		profiler->Reset();
	}
}

void CoreEngine::Exit()
{
	auto g_coordinator = Coordinator::GetInstance();

	auto g_inputSystem = g_coordinator->GetSystem<InputManager>();
	auto g_rhythmSystem = g_coordinator->GetSystem<RhythmGameplayInput>();
	auto g_audioSystem = g_coordinator->GetSystem<AudioManager>();
	auto g_graphicsSystem = g_coordinator->GetSystem<GraphicsManager>();

	if (g_inputSystem)
		g_inputSystem->Shutdown();
	if (g_rhythmSystem)
		g_rhythmSystem->Shutdown();
	if (g_audioSystem)
		g_audioSystem->Shutdown();
	if (g_graphicsSystem)
		g_graphicsSystem->Shutdown();

	g_coordinator->ExitAllSystems();

	DebugDraw::Shutdown();
	MessageManager::DestroyInstance();

	if (window)
	{
		glfwDestroyWindow(window);
		window = nullptr;
	}
	glfwTerminate();
}

void CoreEngine::RenderDebugDraw()
{
	auto g_coordinator = Coordinator::GetInstance();
	auto g_inputSystem = g_coordinator->GetSystem<InputManager>();
	if (!g_inputSystem)
		return;

	const float aspect = (WindowHeight > 0)
							 ? static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight)
							 : (16.0f / 9.0f);

	// reference world height
	const float worldH = 900.0f;
	const float worldW = worldH * aspect;

	glm::mat4 projection = glm::ortho(-worldW * 0.5f, worldW * 0.5f,
									  -worldH * 0.5f, worldH * 0.5f);
	DebugDraw::Begin(projection);

	// F1 - DrawRect
	if (g_inputSystem->isDebugRectsEnabled())
	{
		for (auto &entity : g_inputSystem->DebugEntities)
		{
			auto &transform = g_coordinator->GetComponent<Framework::Transform>(entity);
			glm::vec2 center = {transform.Pos.x, transform.Pos.y};
			glm::vec2 half = 0.5f * glm::vec2(transform.Scale.x, transform.Scale.y);
			glm::vec2 min = center - half;
			glm::vec2 max = center + half;

			DebugDraw::DrawRect(min, max, {1.0f, 1.0f, 0.0f}); // Yellow
		}
	}

	// F2 - DrawLine
	if (g_inputSystem->isDebugLinesEnabled())
	{
		DebugDraw::DrawLine({-800.0f, -450.0f}, {800.0f, 450.0f}, {1.0f, 0.0f, 0.0f});
		DebugDraw::DrawLine({800.0f, -450.0f}, {-800.0f, 450.0f}, {0.0f, 1.0f, 0.0f});
		DebugDraw::DrawLine({0.0f, -450.0f}, {0.0f, 450.0f}, {0.0f, 0.0f, 1.0f});
		DebugDraw::DrawLine({-800.0f, 0.0f}, {800.0f, 0.0f}, {1.0f, 1.0f, 0.0f});
	}

	// F3 - DrawCircle
	if (g_inputSystem->isDebugCirclesEnabled())
	{
		DebugDraw::DrawCircle({0.0, 0.0f}, 200.0f, {1.0f, 0.0f, 1.0f}, 32);
		DebugDraw::DrawCircle({400.0f, 300.0f}, 100.0f, {0.0f, 1.0f, 1.0f}, 24);
	}

	// F4 - DrawCircleFilled
	if (g_inputSystem->isDebugCirclesFilledEnabled())
	{
		DebugDraw::DrawCircleFilled({100.0f, 300.0f}, 80.0f, {1.0f, 0.5f, 0.0f}, 24);
		DebugDraw::DrawCircleFilled({100.0f, -300.0f}, 60.0f, {0.5f, 0.0f, 1.0f}, 16);
	}

	// F5 - DrawPoint
	if (g_inputSystem->isDebugPointsEnabled())
	{
		for (float x = -800.0f; x < 800.0f; x += 200.0f)
		{
			for (float y = -450.0f; y < 450.0f; y += 200.0f)
			{
				DebugDraw::DrawPoint({x, y}, 5.0f, {1.0f, 1.0f, 0.0f});
			}
		}
	}

	// F6 - Grid
	if (g_inputSystem->isDebugGridEnabled())
	{
		for (float x = -800.0f; x < 800.0f; x += 100.0f)
		{
			DebugDraw::DrawLine({x, -450.0f}, {x, 450.0f}, {0.3f, 0.3f, 0.3f});
		}
		for (float y = -450.0f; y < 450.0f; y += 100.0f)
		{
			DebugDraw::DrawLine({-800.0f, y}, {800.0f, y}, {0.3f, 0.3f, 0.3f});
		}
	}

	DebugDraw::End();
}
