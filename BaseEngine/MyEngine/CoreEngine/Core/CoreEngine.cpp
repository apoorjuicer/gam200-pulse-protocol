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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 
#include <windows.h>			   // for OutputDebugString, etc.
#include <mmsystem.h>			   // for timeGetTime()
#pragma comment(lib, "winmm.lib")  //shortcut instead of doing in CMake cos fuck me cmake is hard

#include "pch/pch.h"
#include "graphics/stb_image.h"
#include "CoreEngine/Factory/Factory.h"
#include "../MyEngine/CoreEngine/Prefab/PrefabInstance.h"
#include "../MyEngine/TestCases/TestCase.h"

#include "../MyEngine/Resources/AssetRegistry.h"
#include "../MyEngine/CoreEngine/Asset/AssetsManager.h"
#include "../MyEngine/Serialization/Serialization.h"

#include "../MyEngine/Components/DisplayName.h"
#include "../MyEngine/Components/EntityType.h"

#include "../MyEngine/UI/GUIManager.h"
#pragma comment(lib, "winmm.lib")

#include <shlwapi.h>                // GetModuleFileNameA(), PathRemoveFileSpecA()
#pragma comment(lib, "shlwapi.lib") // Needed for <shlwapi.h>

// Game window dimensions and title
const char WindowTitle[] = "Game Window";
Config config;
AssetsManager mAssets;

int WindowWidth;
int WindowHeight;


CoreEngine* Engine = nullptr;

using HelloWorldFunctionPtr = void(*)(void);
using InitFunctionPtr = void(*)(CoreEngine*);

Framework::Transform* CoreEngine::GetTransformComponent(int ID)
{
	auto g_coordinator = Coordinator::GetInstance();
	return &g_coordinator->GetComponent <Framework::Transform>(ID);
}

// Load coreclr.dll from our application so that we are able to use the functions 
// contained within it to interface with the .NET runtime
void CoreEngine::startScriptEngine()
{
	// Steppppppppppp 1, we get the current executable directory so that we can find the coreclr.dll to load
	std::string runtimePath(MAX_PATH, '\0');

	// Get the path to MyEngine.dll instead of the .exe
	HMODULE hEngine = GetModuleHandleA("MyEngine.dll");
	if (!hEngine)
	{
		throw std::runtime_error("Failed to get MyEngine.dll module handle");
	}

	GetModuleFileNameA(hEngine, runtimePath.data(), MAX_PATH);
	PathRemoveFileSpecA(runtimePath.data());

	// Since PathRemoveFileSpecA() removes from data(), the size is not updated, so we must manually update it
	runtimePath.resize(std::strlen(runtimePath.data()));

	//std::cout << "[CoreEngine] Runtime path: \n\n\n\n\n\n\n\n" << runtimePath << std::endl;
	
	// Also, while we're at it, set the current working directory to the current executable directory
	std::filesystem::current_path(runtimePath);


	// Construct the CoreCLR path
	std::string coreClrPath(runtimePath); // Works
	coreClrPath += "\\coreclr.dll";
	
	//std::cout << "[CoreEngine] Looking for:  \n\n\n\n\n\n\n\n" << coreClrPath << std::endl;

	// Load the CoreCLR DLL
	coreClr = LoadLibraryExA(coreClrPath.c_str(), nullptr, 0);
	if (!coreClr)
	{
		DWORD error = GetLastError();  
		std::cout << error << std::endl;

		std::ostringstream oss;
		oss << "Failed to load CoreCLR from: " << coreClrPath
			<< "\nWindows Error Code: " << error;

		throw std::runtime_error("Failed to load CoreCLR.");
	}

	// Step 2, we get CoreCLR hosting functions
	initializeCoreClr = getCoreClrFuncPtr<coreclr_initialize_ptr>("coreclr_initialize");
	createManagedDelegate = getCoreClrFuncPtr<coreclr_create_delegate_ptr>("coreclr_create_delegate");
	shutdownCoreClr = getCoreClrFuncPtr<coreclr_shutdown_ptr>("coreclr_shutdown");

	// Step 3, we then construct AppDomain properties used when starting the runtime
	std::string tpaList = buildTpaList(runtimePath);
	// Define CoreCLR properties
	std::array propertyKeys =
	{
		"TRUSTED_PLATFORM_ASSEMBLIES",      // Trusted assemblies (like the GAC)
		"APP_PATHS",                        // Directories to probe for application assemblies
	};
	std::array propertyValues =
	{
		tpaList.c_str(),
		runtimePath.c_str()
	};

	//Step 4, kms.. jk, start the CoreCLR runtime (linking part)
	int result = initializeCoreClr
	(
		runtimePath.c_str(),     // AppDomain base path
		"SampleHost",            // AppDomain friendly name, this can be anything you want really
		static_cast<int>(propertyKeys.size()),     // Property count
		propertyKeys.data(),     // Property names
		propertyValues.data(),   // Property values
		&hostHandle,             // Host handle
		&domainId                // AppDomain ID
	);

	// Check if intiialization of CoreCLR failed
	if (result < 0)
	{
		std::ostringstream oss;
		oss << std::hex << std::setfill('0') << std::setw(8)
			<< "Failed to initialize CoreCLR. Error 0x" << result << "\n";
		throw std::runtime_error(oss.str());
	}


}

// Unload the .NET runtime 
void CoreEngine::stopScriptEngine()
{
    // Shutdown CoreCLR
    const int RESULT = shutdownCoreClr(hostHandle, domainId);
    if (RESULT < 0)
    {
        std::stringstream oss;
        oss << std::hex << std::setfill('0') << std::setw(8)
            << "Failed to shut down CoreCLR. Error 0x" << RESULT << "\n";
        throw std::runtime_error(oss.str());
    }
}

// Construct a list of DLLs that tells the CLR which are the DLLs it can load at runtime and 
// where to find them. Any DLLs not a part of this list will not be able to be loaded. SO RMB TO ADD
std::string CoreEngine::buildTpaList(const std::string& directory)
{
	// Constants
	static const std::string SEARCH_PATH = directory + "\\*.dll";
	static constexpr char PATH_DELIMITER = ';';

	// Create a osstream object to compile the string
	std::ostringstream tpaList;
	
	// Search the current directory for the TPAs (.DLLs)
	WIN32_FIND_DATAA findData;
	HANDLE fileHandle = FindFirstFileA(SEARCH_PATH.c_str(), &findData);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			// Append the assembly to the list
			tpaList << directory << '\\' << findData.cFileName << PATH_DELIMITER;
		} while (FindNextFileA(fileHandle, &findData));
		FindClose(fileHandle);
	}
	return tpaList.str();
}


void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei,
	const char* message,
	const void* userParam)
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
	// mAssets.Initialize();
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

	///*saving config data*/ //tested saving works
	// std::string newConfigPath = "../../MyEngine/JSON/config2.json";
	// config.SetWindowWidth(800);
	// config.SetWindowHeight(600);
	// std::cout << "set WindowWidth is: " << config.GetWindowWidth() << "\n";
	// std::cout << "set WindowHeight is: " << config.GetWindowHeight() << "\n";
	// if (!SaveConfig(config, newConfigPath))
	//{
	//	std::cerr << "Failed to load config!\n";
	// }
	// else
	//{
	//	std::cout << "save config to FilePath: " << std::filesystem::absolute(newConfigPath) << "\n";
	// }

	if (!glfwInit())
	{
		//spdlog::error("Failed to initialize GLFW");
		return;
	}

	// loading metadata for audio,texture,font into
	// ASset reg's container each respectively
	// ar.LoadAudioManifest();
	// ar.LoadTextureManifest();
	// ar.LoadFontManifest();
	// ar.LoadSceneManifest();

	unsigned int version = 0;
	FMOD::System* system = nullptr;

	FMOD::System_Create(&system);
	system->getVersion(&version);

	std::cout << "FMOD version: "
		<< (version >> 16) << "."
		<< ((version >> 8) & 0xFF) << "."
		<< (version & 0xFF) << std::endl;

	system->release();

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
	GLHelper::ptr_window = window;

	GLHelper::setup_event_callbacks();

	// === FORCE FULLSCREEN AT STARTUP ===
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		glfwSetWindowMonitor(
			window,
			monitor,         // full screen monitor
			0, 0,            // position
			mode->width,     // resolution
			mode->height,
			mode->refreshRate
		);
	}

	// Pause/resume on focus change (covers ALT+TAB and returning from Ctrl+Alt+Del)
	glfwSetWindowFocusCallback(window,
		[](GLFWwindow* win, int focused)
		{
			(void)win;
			if (!Engine) return;
			if (focused)
				Engine->SetPaused(false);
			else
				Engine->SetPaused(true);
		});

	// Also pause when minimized / restored
	glfwSetWindowIconifyCallback(window,
		[](GLFWwindow* win, int iconified)
		{
			(void)win;
			if (!Engine) return;
			if (iconified == GLFW_TRUE)
				Engine->SetPaused(false);
			// When restored, focus callback will unpause if the window regains focus
		});


	glfwSetWindowAspectRatio(window, 16, 9);
	glfwSetFramebufferSizeCallback(window,
		[](GLFWwindow*, int fbw, int fbh)
		{
			if (fbw <= 0 || fbh <= 0)
				return;
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

	WindowWidth = fbw;
	WindowHeight = fbh;

	glewInit();


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
	mAssets.Initialize(); //loaded registry inside
	//std::cout << mAssets.getRegistry().FindTexture("hero") << "\n";
	// Singleton for Engine, but once game use EventBus/messaging system
	std::cout << "path for bgm : " << mAssets.getRegistry().FindAudio("bgm")->path << "\n";
	// PLEASE CALL THIS EVERY SINGLE U WANT 2 USE SINGLETON
	auto g_coordinator = Coordinator::GetInstance();

	auto g_inputSystem = g_coordinator->GetSystem<InputManager>();
	auto g_rhythmSystem = g_coordinator->GetSystem<RhythmGameplayInput>();
	auto g_audioSystem = g_coordinator->GetSystem<AudioManager>();		//default cons
	auto g_graphicsSystem = g_coordinator->GetSystem<GraphicsManager>(); //default cons
	auto g_CollisionManager = g_coordinator->GetSystem<CollisionManager>();

	g_audioSystem->SetRegistry(&mAssets.getRegistry());
	g_graphicsSystem->SetRegistry(&mAssets.getRegistry());

	mAssets.SetAudioManager(g_audioSystem.get());
	mAssets.SetGraphicsManager(g_graphicsSystem.get());

	//g_graphicsSystem->GM_FindFont("PatrickHand_font");
	//g_graphicsSystem->GM_FindFont("liberation_font");

	MessageManager::GetInstance();
	std::cout << "[MessageManager] Initialized" << std::endl;

	// Initialize systems AFTER GL is ready
	g_inputSystem->Initialize(window);
	g_rhythmSystem->Initialize();
	g_graphicsSystem->Initialize();
	g_audioSystem->Initialize();
	g_coordinator->InitializeAllSystems();

	g_graphicsSystem->GM_FindFont("PatrickHand_font");
	g_graphicsSystem->GM_FindFont("liberation_font");

	ProfilingManager::GetInstance()->SetResetInterval(60.0);
	ProfilingManager::GetInstance()->SetDisplayUpdateInterval(1.0);

	// GLuint barTexture = LoadTexture("../../MyEngine/Assets/Sprites/perfect.png");
	// if (barTexture > 0) {
	//	Entity box2 = g_inputSystem->GetFeedbackEntity2();  // Use getter
	//	auto& transform2 = g_coordinator->GetComponent<Framework::Transform>(box2);
	//	transform2.textureID = barTexture;
	//	std::cout << "Texture loaded and assigned to box 2: " << barTexture << std::endl;
	// }

	GameRunning = true;	 // 3.. 2.. 1.. GAME START
	startScriptEngine(); // 3.. 2.. 1.. DLL SCRIPT! START ROLLINGGGGGGGGGGGGGGGGGG

	// Step 1. Get the function ptrs
	InitFunctionPtr initFunc = GetFunctionPtr<InitFunctionPtr>
		(
			"ScriptAPI",                 // Name of the Assembly
			"ScriptAPI.EngineInterface", // Full name of the class
			"Init"                 // Name of the function
		);

	auto addScript = GetFunctionPtr<bool(*)(int, const char*)>
		(
			"ScriptAPI",
			"ScriptAPI.EngineInterface",
			"AddScriptViaName"
		);

	/*auto executeUpdate = GetFunctionPtr<void(*)(void)>
		(
			"ScriptAPI",
			"ScriptAPI.EngineInterface",
			"ExecuteUpdate"
		);*/

	// Step 2: Initialize it with  pointer to CoreEngine as param
	initFunc(this); 
	
	// Step 3: Adding script to an entity[please work ;-; ]
	addScript(0, "TestScript"); // THE 0 IS ENTITY ID 0
	
	
	//this is for testing
	HelloWorldFunctionPtr hwFunc = GetFunctionPtr<HelloWorldFunctionPtr>
		(
			"ScriptAPI",                 // Name of the Assembly
			"ScriptAPI.EngineInterface", // Full name of the class
			"HelloWorld"                 // Name of the function
		);
	hwFunc();
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
	g_coordinator->RegisterComponent<Health>();
	g_coordinator->RegisterComponent<LogicComponent>();
	g_coordinator->RegisterComponent<NamingComponent>();
	g_coordinator->RegisterComponent<UIElement>();
	g_coordinator->RegisterComponent<Name>();
	g_coordinator->RegisterComponent<LayerTag>();
	g_coordinator->RegisterComponent<PrefabInstance>();

	// Register your systems here, this order is IMPORTANT FOR UPDATE LOOP!!!!
	g_coordinator->RegisterSystem<InputManager>();
	g_coordinator->RegisterSystem<RhythmGameplayInput>();
	//g_coordinator->RegisterSystem<GUIManager>();
	g_coordinator->RegisterSystem<GraphicsManager>();
	g_coordinator->RegisterSystem<AudioManager>();
	g_coordinator->RegisterSystem<CollisionManager>();
	g_coordinator->RegisterSystem<SceneManager>();
	g_coordinator->RegisterSystem<LogicSystem>();

	// Set signature for component and system
	Signature collisionSig;
	collisionSig.set(g_coordinator->GetComponentType<NPC>());
	collisionSig.set(g_coordinator->GetComponentType<AABB>());
	collisionSig.set(g_coordinator->GetComponentType<Health>());
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

	//Signature UISig;
	//UISig.set(g_coordinator->GetComponentType<UIElement>());
	//UISig.set(g_coordinator->GetComponentType<Framework::Transform>());
	//g_coordinator->SetSystemSignature<GUIManager>(UISig);

	Signature graphicsSig;
	graphicsSig.set(g_coordinator->GetComponentType<Framework::Transform>());  // <-- needs Transform
	graphicsSig.set(g_coordinator->GetComponentType<Framework::Renderable>()); // <-- needs Renderable
	graphicsSig.set(g_coordinator->GetComponentType<Framework::Animation>());  // <-- needs Animation
	graphicsSig.set(g_coordinator->GetComponentType<LayerTag>());
	g_coordinator->SetSystemSignature<GraphicsManager>(graphicsSig);

	Signature sceneSig;
	sceneSig.set(g_coordinator->GetComponentType<NPC>());
	sceneSig.set(g_coordinator->GetComponentType<AABB>());
	sceneSig.set(g_coordinator->GetComponentType<Health>());
	sceneSig.set(g_coordinator->GetComponentType<NamingComponent>());
	sceneSig.set(g_coordinator->GetComponentType<Framework::Transform>());
	sceneSig.set(g_coordinator->GetComponentType<Framework::Renderable>());
	sceneSig.set(g_coordinator->GetComponentType<Framework::Animation>());
	g_coordinator->SetSystemSignature<SceneManager>(sceneSig);

	Signature LogicSig;
	LogicSig.set(g_coordinator->GetComponentType<LogicComponent>());
	g_coordinator->SetSystemSignature<LogicSystem>(LogicSig);
}

void CoreEngine::Update(float dt)
{
	auto g_coordinator = Coordinator::GetInstance();
	auto g_audioSystem = g_coordinator->GetSystem<AudioManager>();
	auto g_inputSystem = g_coordinator->GetSystem<InputManager>();
	auto g_rhythmSystem = g_coordinator->GetSystem<RhythmGameplayInput>();
	auto g_graphicsSystem = g_coordinator->GetSystem<GraphicsManager>();
	auto g_sceneSystem = g_coordinator->GetSystem<SceneManager>();
	auto profiler = ProfilingManager::GetInstance();

	auto executeUpdate = GetFunctionPtr<void(*)(void)>
		(
			"ScriptAPI",
			"ScriptAPI.EngineInterface",
			"ExecuteUpdate"
		);
	executeUpdate();

	// Update loop for our scripts
	/*if (executeUpdateFunc)
	{
		executeUpdateFunc();
	}*/

	// ACCEPTED BOTH VERSION BUT COMMENT OUT JUST TO SEE IF MERGE WORKS FOR NOW 
	// if (!m_paused)
	// {
	// 	profiler->StartSystem("Scene System");
	// 	if (g_sceneSystem) g_sceneSystem->Update(dt);
	// 	profiler->EndSystem("Scene System");

	// 	profiler->StartSystem("LogicSystem");
	// 	g_coordinator->UpdateAllSystems(dt);
	// 	profiler->EndSystem("LogicSystem");

	// 	profiler->StartSystem("Input System");
	// 	if (g_inputSystem) g_inputSystem->Update(dt);
	// 	profiler->EndSystem("Input System");

	// 	profiler->StartSystem("RhythmGameplayInput");
	// 	if (g_rhythmSystem) g_rhythmSystem->Update(dt);
	// 	profiler->EndSystem("RhythmGameplayInput");

	// 	profiler->StartSystem("Audio System");
	// 	if (g_audioSystem) g_audioSystem->Update();
	// 	profiler->EndSystem("Audio System");
	// }

	profiler->StartSystem("Scene System");
	if (g_sceneSystem)
		g_sceneSystem->Update(dt);
	profiler->EndSystem("Scene System");

	profiler->StartSystem("LogicSystem");
	g_coordinator->UpdateAllSystems(dt);
	profiler->EndSystem("LogicSystem");

	profiler->StartSystem("Input System");
	if (g_inputSystem)
		g_inputSystem->Update(dt);
	profiler->EndSystem("Input System");

	profiler->StartSystem("RhythmGameplayInput");
	if (g_rhythmSystem)
		g_rhythmSystem->Update(dt);
	profiler->EndSystem("RhythmGameplayInput");

	profiler->StartSystem("Graphics System");
	if (g_graphicsSystem)
		g_graphicsSystem->Update();
	profiler->EndSystem("Graphics System");

	profiler->StartSystem("Audio System");
	if (g_audioSystem)
		g_audioSystem->Update();
	profiler->EndSystem("Audio System");

	profiler->StartSystem("Collision System");
	// If collision is in UpdateAllSystems, remove this
	profiler->EndSystem("Collision System");
	// Test.Update(dt);

	// Always process engine / message stuff
	MessageManager::GetInstance()->ProcessQueue(glfwGetTime());

	// Always draw/update graphics once per frame
	if (g_graphicsSystem)
		g_graphicsSystem->Update();

	// 2) If paused, stop here. no gameplay / input / scene logic
	if (m_paused)
	{
		return;
	}

	// 3) Normal running path
	if (g_inputSystem)    g_inputSystem->Update(dt);
	if (g_rhythmSystem)   g_rhythmSystem->Update(dt);

}

void CoreEngine::GameLoop()
{
    LastTime = timeGetTime();
    auto profiler = ProfilingManager::GetInstance();

    while (!glfwWindowShouldClose(window) && GameRunning)
    {
        // --- dt calculation ---
        unsigned currenttime = timeGetTime();
        float dt = (currenttime - LastTime) / 1000.0f;
        LastTime = currenttime;

        glfwPollEvents();

        // --- Game update (ECS systems) ---
        profiler->StartFrame();
        Update(dt);
        profiler->EndFrame();

        // --- Game rendering ---
        RenderDebugDraw();      // your OpenGL game draw

        // --- Editor (ImGui) on top of game ---
        if (m_editorFrameCallback)
        {
            m_editorFrameCallback();   // calls NewFrame + Draw + Render
        }

        // --- Present frame ---
        glfwSwapBuffers(window);

        profiler->Reset();
    }
}


void CoreEngine::Exit()
{
	stopScriptEngine(); // stop the Csharp Scripts from using "bridge". iykyk

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
		for (auto& entity : g_inputSystem->DebugEntities)
		{
			auto& transform = g_coordinator->GetComponent<Framework::Transform>(entity);
			glm::vec2 center = { transform.Pos.x, transform.Pos.y };
			glm::vec2 half = 0.5f * glm::vec2(transform.Scale.x, transform.Scale.y);
			glm::vec2 min = center - half;
			glm::vec2 max = center + half;

			DebugDraw::DrawRect(min, max, { 1.0f, 1.0f, 0.0f }); // Yellow
		}
	}

	// F2 - DrawLine
	if (g_inputSystem->isDebugLinesEnabled())
	{
		DebugDraw::DrawLine({ -800.0f, -450.0f }, { 800.0f, 450.0f }, { 1.0f, 0.0f, 0.0f });
		DebugDraw::DrawLine({ 800.0f, -450.0f }, { -800.0f, 450.0f }, { 0.0f, 1.0f, 0.0f });
		DebugDraw::DrawLine({ 0.0f, -450.0f }, { 0.0f, 450.0f }, { 0.0f, 0.0f, 1.0f });
		DebugDraw::DrawLine({ -800.0f, 0.0f }, { 800.0f, 0.0f }, { 1.0f, 1.0f, 0.0f });
	}

	// F3 - DrawCircle
	if (g_inputSystem->isDebugCirclesEnabled())
	{
		DebugDraw::DrawCircle({ 0.0, 0.0f }, 200.0f, { 1.0f, 0.0f, 1.0f }, 32);
		DebugDraw::DrawCircle({ 400.0f, 300.0f }, 100.0f, { 0.0f, 1.0f, 1.0f }, 24);
	}

	// F4 - DrawCircleFilled
	if (g_inputSystem->isDebugCirclesFilledEnabled())
	{
		DebugDraw::DrawCircleFilled({ 100.0f, 300.0f }, 80.0f, { 1.0f, 0.5f, 0.0f }, 24);
		DebugDraw::DrawCircleFilled({ 100.0f, -300.0f }, 60.0f, { 0.5f, 0.0f, 1.0f }, 16);
	}

	// F5 - DrawPoint
	if (g_inputSystem->isDebugPointsEnabled())
	{
		for (float x = -800.0f; x < 800.0f; x += 200.0f)
		{
			for (float y = -450.0f; y < 450.0f; y += 200.0f)
			{
				DebugDraw::DrawPoint({ x, y }, 5.0f, { 1.0f, 1.0f, 0.0f });
			}
		}
	}

	// F6 - Grid
	if (g_inputSystem->isDebugGridEnabled())
	{
		for (float x = -800.0f; x < 800.0f; x += 100.0f)
		{
			DebugDraw::DrawLine({ x, -450.0f }, { x, 450.0f }, { 0.3f, 0.3f, 0.3f });
		}
		for (float y = -450.0f; y < 450.0f; y += 100.0f)
		{
			DebugDraw::DrawLine({ -800.0f, y }, { 800.0f, y }, { 0.3f, 0.3f, 0.3f });
		}
	}

	// ---------------------------------------------------------------------
	// F7: spawn/delete 1,000 moving sprites (hero)
	// ---------------------------------------------------------------------
	static std::vector<Entity>                   s_stressEntities;
	static std::unordered_map<Entity, glm::vec2> s_velocity;
	static double s_lastTick = glfwGetTime();

	// to fix warnings, previously using same variables 2 times
	float stressWorldH = 900.0f;
	float stressAspect = (WindowHeight > 0) ? float(WindowWidth) / float(WindowHeight) : (16.0f / 9.0f);
	float stressWorldW = stressWorldH * stressAspect;

	if (g_inputSystem->ConsumeStressToggleRequest())
	{
		if (!s_stressEntities.empty())
		{
			Factory factory;
			for (Entity e : s_stressEntities) factory.DeleteEntity(e);
			s_stressEntities.clear();
			s_velocity.clear();
			std::cout << "[StressTest] Deleted spawned entities (F7)\n";
		}
		else
		{
			// Resolve the real path for the 'hero' texture via the registry
			const std::string heroPath = mAssets.GetTexturePath("hero");  // e.g. ".../Textures/Characters/hero.png"
			const GLuint sharedTex = mAssets.GetOrLoadTexture(heroPath);  // goes through GraphicsManager cache
			if (!sharedTex) {
				std::cerr << "[StressTest] Failed to load texture: " << heroPath << "\n";
			}

			//const float worldH = 900.0f;
			//const float aspect = (WindowHeight > 0)
			//	? float(WindowWidth) / float(WindowHeight)
			//	: (16.0f / 9.0f);
			//const float worldW = worldH * aspect;

			std::mt19937 rng{ std::random_device{}() };
			std::uniform_real_distribution<float> posX(-stressWorldW, stressWorldW);
			std::uniform_real_distribution<float> posY(-stressWorldH, stressWorldH);
			std::uniform_real_distribution<float> vel(-220.0f, 220.0f);
			std::uniform_real_distribution<float> size(28.0f, 64.0f);

			s_stressEntities.reserve(1000);
			for (int i = 0; i < 1000; ++i)
			{
				Entity e = Factory{}.DefaultObj();
				s_stressEntities.push_back(e);

				// Transform
				auto& t = g_coordinator->GetComponent<Framework::Transform>(e);
				t.Pos = { posX(rng), posY(rng) };
				float s = size(rng);
				t.Scale = { s, s };
				t.Rotation = { 0.0f, 0.0f };

				// Renderable  USE SPRITE PIPELINE + TEXTURE
				auto& r = g_coordinator->GetComponent<Framework::Renderable>(e);
				r.useTexture = (sharedTex != 0);
				r.textureID = sharedTex;          // GL handle already cached by GM
				r.spriteName = heroPath;           // use the SAME key string you loaded with
				r.mdl_ref = 1.0f;               // ? sprite quad model (not background/other model)
				r.shd_ref = 2.0f;               // ? sprite shader that samples the texture
				// r.tintColor stays white by default

				// Motion
				s_velocity[e] = { vel(rng), vel(rng) };
			}

			std::cout << "[StressTest] Spawned 1000 entities with texture: " << heroPath << " (F7)\n";
		}
	}

	if (!s_stressEntities.empty())
	{
		const double now = glfwGetTime();
		const float dt = float(now - s_lastTick);
		s_lastTick = now;

		//const float worldH = 900.0f;
		//const float aspect = (WindowHeight > 0)
		//	? float(WindowWidth) / float(WindowHeight)
		//	: (16.0f / 9.0f);
		//const float worldW = worldH * aspect;

		const float left = -stressWorldW;
		const float right = stressWorldW;
		const float bottom = -stressWorldH;
		const float top = stressWorldH;

		for (Entity e : s_stressEntities)
		{
			if (!g_coordinator->HasComponent<Framework::Transform>(e)) continue;

			auto& t = g_coordinator->GetComponent<Framework::Transform>(e);
			auto& v = s_velocity[e];

			t.Pos.x += v.x * dt;
			t.Pos.y += v.y * dt;

			const float halfW = t.Scale.x * 0.5f;
			const float halfH = t.Scale.y * 0.5f;

			if (t.Pos.x - halfW < left) { t.Pos.x = left + halfW; v.x = -v.x; }
			if (t.Pos.x + halfW > right) { t.Pos.x = right - halfW; v.x = -v.x; }
			if (t.Pos.y - halfH < bottom) { t.Pos.y = bottom + halfH; v.y = -v.y; }
			if (t.Pos.y + halfH > top) { t.Pos.y = top - halfH; v.y = -v.y; }
		}
	}
	// ---------------------------------------------------------------------



	DebugDraw::End();
}

 void CoreEngine::SetPaused(bool paused)
{
	if (m_paused == paused) return;
	m_paused = paused;

	// Pause/resume audio, too
	if (auto audio = Coordinator::GetInstance()->GetSystem<AudioManager>()) {
		audio->SetPaused(paused);
	}
 }

 void CoreEngine::TogglePaused()
{
	SetPaused(!m_paused);
 }

void CoreEngine::PlayGame()
{
	m_isPlaying = true;

	auto coord = Coordinator::GetInstance();

	// ALWAYS update the original scene path when Play is pressed
	if (auto sceneMgr = coord->GetSystem<SceneManager>())
	{
		std::string sceneName = sceneMgr->GrabCurrentScene();

		// Remove "Saved" prefix if it exists
		if (sceneName.find("Saved") == 0)
		{
			sceneName = sceneName.substr(5);
		}
		// Remove "Scene" suffix if it exists
		size_t scenePos = sceneName.find("Scene");
		if (scenePos != std::string::npos)
		{
			sceneName = sceneName.substr(0, scenePos);
		}

		// ALWAYS update this path, even if already set
		m_originalScenePath = "../../MyEngine/JSON/" + sceneName + ".json";

		std::cout << "[CoreEngine] Original scene path: " << m_originalScenePath << std::endl; // DEBUG
	}

	if (auto audio = coord->GetSystem<AudioManager>()) {
		audio->RestartBGM();
	}

	// Call Init() on all scripts
	if (auto logicSys = coord->GetSystem<LogicSystem>())
	{
		for (auto const& entity : logicSys->EntityMember)
		{
			auto& scriptname = coord->GetComponent<LogicComponent>(entity);
			auto script = logicSys->GrabScript(scriptname.ScriptName);
			if (script)
			{
				script->Init(entity);
			}
		}
	}
}

void CoreEngine::StopGame()
{
	m_isPlaying = false;

	auto coord = Coordinator::GetInstance();

	// pause the bg music
	if (auto audio = coord->GetSystem<AudioManager>())
	{
		audio->StopBGM();
	}

	//// reloads original scene to reset all entities to initial starting position
	//if (auto sceneMgr = coord->GetSystem<SceneManager>())
	//{
	//	if (!m_originalScenePath.empty())
	//	{
	//		sceneMgr->LoadSceneFromJson(m_originalScenePath);

	//		// Sync GUI tracking after scene reload
	//		//if (auto guiMgr = coord->GetSystem<GUIManager>())
	//		//{
	//		//	guiMgr->SyncTrackingWithScene();
	//		//	std::cout << "[CoreEngine] GUI tracking synced after Stop" << std::endl;
	//		//}
	//	}
	//}
}

// this might have been causing the random crash cus its calling restartBGM() too
void CoreEngine::RestartRuntimeForPlay()
{
	//auto coord = Coordinator::GetInstance();
	//if (!coord) return;

	//// 1) AUDIO: force BGM to start at 0
	//if (auto audio = coord->GetSystem<AudioManager>()) {
	//	// if engine is paused (Stop button), keep audio paused too
	//	audio->RestartBGM();
	//}

	//// 2) RHYTHM: shutdown + initialize
	//if (auto rhythm = coord->GetSystem<RhythmGameplayInput>()) {
	//	rhythm->Shutdown();
	//	rhythm->Initialize();
	//}
}

void CoreEngine::PauseGame()
{
	m_isPlaying = false;
	auto coord = Coordinator::GetInstance();

	if (auto audio = coord->GetSystem<AudioManager>()) {
		audio->PauseBGM();
	}
}

void CoreEngine::ResumeGame()
{
	m_isPlaying = true;
	auto coord = Coordinator::GetInstance();

	if (auto audio = coord->GetSystem<AudioManager>()) {
		audio->ResumeBGM();
	}
}

void CoreEngine::SetEditorFrameCallback(const std::function<void()>& cb)
{
	m_editorFrameCallback = cb;
}

void CoreEngine::ToggleFullscreen()
{
	static bool isFullscreen = true;	

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	if (!isFullscreen)
	{
		// SWITCH TO FULLSCREEN
		glfwSetWindowMonitor(
			window,
			monitor,
			0, 0,
			mode->width,
			mode->height,
			mode->refreshRate
		);
	}
	else
	{
		// SWITCH TO WINDOWED MODE (use config size)
		glfwSetWindowMonitor(
			window,
			nullptr,
			100, 100,                    // position
			config.GetWindowWidth(),
			config.GetWindowHeight(),
			GLFW_DONT_CARE
		);
	}

	isFullscreen = !isFullscreen;
}