#include <windows.h>   // for OutputDebugString, etc.
#include "pch/pch.h"
#pragma comment(lib, "winmm.lib")

//Coordinator g_coordinator;
//extern InputManager* g_inputSystem;
//extern GraphicsManager* g_graphicsSystem;

// A global pointer to the core
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
	//g_inputSystem = new InputManager();
	//g_graphicsSystem = new GraphicsManager();


	auto g_inputSystem = g_coordinator.GetSystem<InputManager>();
	auto g_graphicsSystem = g_coordinator.GetSystem<GraphicsManager>();

	// Initialize systems AFTER GL is ready
	g_inputSystem->Initialize(window);
	g_graphicsSystem->Initialize();
	g_coordinator.InitializeAllSystems();
	std::cout << "Engine initialized." << std::endl;

    g_inputSystem->Initialize(window);
    g_graphicsSystem->Initialize();
    g_coordinator.InitializeAllSystems();

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
	g_coordinator.init();

	// Register your component here
	g_coordinator.RegisterComponent<Framework::Transform>();
	
	// Register your systems here
	g_coordinator.RegisterSystem<InputManager>();
	g_coordinator.RegisterSystem<GraphicsManager>();

    Signature graphicsSig;
    graphicsSig.set(g_coordinator.GetComponentType<Framework::Transform>());
    g_coordinator.SetSystemSignature<GraphicsManager>(graphicsSig);
}

void CoreEngine::Update(float dt)
{
	auto g_inputSystem = g_coordinator.GetSystem<InputManager>();
	auto g_graphicsSystem = g_coordinator.GetSystem<GraphicsManager>();

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

void CoreEngine::Render()
{
    // Placeholder for rendering
}

void CoreEngine::GameLoop()
{
    LastTime = timeGetTime();
    //init
    const float printInterval = 0.2f;
    float printTimer = 0.0f;
    
    bool spawnFromLeft = true; //true = start left move right,  false = start right move left
    bool automaticSwordSwing = false; // true = sword swinging, false = not swinging
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
    }
    std::cout << "\n=== TEST SETTINGS ===\n";
    std::cout << "Monster Spawn Side: " << (spawnFromLeft ? "LEFT" : "RIGHT") << "\n";
    std::cout << "Automatic Sword Swing: " << (automaticSwordSwing ? "ON" : "OFF") << "\n";
    std::cout << "Testing Mode: " << (testBoxLineCollision ? "BOX-TO-LINE" : "HERO/MONSTER") << "\n";
    std::cout << "=====================\n";

    if (!testBoxLineCollision)
    {
        // HERO/MONSTER TEST SETUP
        Hero hero;
        hero.pos = { 400.0f, 700.0f };
        hero.width = 50.0f;
        hero.height = 50.0f;
        hero.swordWidth = 60.0f;
        hero.swordHeight = 20.0f;
        hero.swingProgress = 0.0f;
        hero.isSwinging = automaticSwordSwing;
        hero.hp = 10;

        Monster monsterTemplate;
        monsterTemplate.width = 50.0f;
        monsterTemplate.height = 50.0f;
        monsterTemplate.isAlive = true;
        std::vector<Monster> monsters = { monsterTemplate };
        float monsterSpeed = 200.0f;

        if (spawnFromLeft)
            monsters[0].pos = { 0.0f + monsterTemplate.width / 2.0f, 700.0f };
        else
            monsters[0].pos = { WindowWidth - monsterTemplate.width / 2.0f, 700.0f };

        bool collisionOccurred = false;
        bool heroMonsterCollided = false;
        bool swordMonsterCollided = false;

        while (!glfwWindowShouldClose(window) && !collisionOccurred)
        {
            unsigned currenttime = timeGetTime();
            float dt = (currenttime - LastTime) / 1000.0f;
            LastTime = currenttime;
            printTimer += dt;

            glfwPollEvents();
            Update(dt);

            if (monsters[0].isAlive)
            {
                float monsterVelX = (spawnFromLeft ? monsterSpeed : -monsterSpeed);
                monsters[0].pos.x += monsterVelX * dt;
                std::string direction = (monsterVelX > 0) ? "RIGHT" : "LEFT";

                if (printTimer >= printInterval)
                {
                    std::cout << "[TEST] Monster moving " << direction
                        << ". x = " << monsters[0].pos.x << "\n";
                    printTimer = 0.0f;
                }

                if (!automaticSwordSwing)
                {
                    if (!heroMonsterCollided && CheckAABBCollision(BuildHeroHitbox(hero), BuildMonsterHitbox(monsters[0])))
                    {
                        monsters[0].isAlive = false;
                        hero.hp--;
                        heroMonsterCollided = true;
                        std::cout << "[Monster-Hero] Collision! Hero HP: " << hero.hp << "\n";
                        collisionOccurred = true;
                    }
                }
                else
                {
                    hero.swingProgress = (hero.pos.x < monsters[0].pos.x) ? hero.width : -hero.swordWidth;
                    if (!swordMonsterCollided && CheckAABBCollision(BuildSwordHitbox(hero), BuildMonsterHitbox(monsters[0])))
                    {
                        monsters[0].isAlive = false;
                        swordMonsterCollided = true;
                        std::cout << "[Sword-Monster] Collision! Hero HP: " << hero.hp << "\n";
                        collisionOccurred = true;
                    }
                }
            }

            if ((spawnFromLeft && monsters[0].pos.x - monsterTemplate.width / 2.0f > WindowWidth) ||
                (!spawnFromLeft && monsters[0].pos.x + monsterTemplate.width / 2.0f < 0))
            {
                std::cout << "[TEST] Monster left screen. No collision.\n";
                break;
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
            glfwSwapBuffers(window);
        }
    }
    else
    {
        // BOX-TO-LINE TEST SETUP
        struct Line { std::string name; float x, yStart, yEnd; };
        std::vector<Line> lines = {
            {"Line1", 200.0f, 180.0f, 280.0f},
            {"Line2", 300.0f, 180.0f, 280.0f},
            {"Line3", 400.0f, 180.0f, 280.0f},
            {"Line4", 500.0f, 180.0f, 280.0f}
        };

        AABB movingBox;
        movingBox.min = { 0.0f, 180.0f };
        movingBox.max = { 100.0f, 280.0f };
        float boxSpeed = 150.0f;
        bool boxLeftScreen = false;

        while (!glfwWindowShouldClose(window) && !boxLeftScreen)
        {
            unsigned currenttime = timeGetTime();
            float dt = (currenttime - LastTime) / 1000.0f;
            LastTime = currenttime;

            glfwPollEvents();
            Update(dt);

            movingBox.min.x += boxSpeed * dt;
            movingBox.max.x += boxSpeed * dt;

            for (auto& line : lines)
            {
                bool xOverlap = (movingBox.min.x <= line.x && movingBox.max.x >= line.x);
                bool yOverlap = (movingBox.max.y >= line.yStart && movingBox.min.y <= line.yEnd);

                if (xOverlap && yOverlap)
                {
                    float boxCenter = (movingBox.min.x + movingBox.max.x) / 2.0f;
                    LineHitResult result;
                    if (boxCenter >= line.x - 10 && boxCenter <= line.x + 10) result = LineHitResult::Excellent;
                    else if (boxCenter >= line.x - 30 && boxCenter <= line.x + 30) result = LineHitResult::Good;
                    else result = LineHitResult::Bad;

                    std::cout << "[BOX-TO-LINE] Box X: " << boxCenter
                        << " collided with " << line.name
                        << " -> Result: ";
                    switch (result)
                    {
                    case LineHitResult::Bad:       std::cout << "Bad"; break;
                    case LineHitResult::Good:      std::cout << "Good"; break;
                    case LineHitResult::Excellent: std::cout << "Excellent"; break;
                    default: break;
                    }
                    std::cout << "\n";
                }
            }

            if (movingBox.min.x > WindowWidth)
            {
                std::cout << "[BOX-TO-LINE] Box left screen. Test finished.\n";
                boxLeftScreen = true;
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
            glfwSwapBuffers(window);
        }
    }

    std::cout << "Press ENTER to exit the test..." << std::endl;
    std::cin.get();
}

void CoreEngine::Exit()
{
	auto g_inputSystem = g_coordinator.GetSystem<InputManager>();
	auto g_graphicsSystem = g_coordinator.GetSystem<GraphicsManager>();

	if (g_inputSystem) {
		g_inputSystem->Shutdown();
		//delete g_inputSystem;
		//g_inputSystem = nullptr;
	}
	if (g_graphicsSystem) {
		g_graphicsSystem->Shutdown();
		//delete g_graphicsSystem;
		//g_graphicsSystem = nullptr;
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
