// ==================================================================================
// InputManager.cpp  
// Author: Chloe Lau Rey En
// Brief: Implementation of InputManager class with frame-based input detection.
//        Handles rhythm gameplay inputs, weapon switching, mouse aiming, and
//        provides M1 testing functionality with console output.
// ==================================================================================

#include "InputManager.h"
#include <iostream>
#include <cstring>
#include "pch/pch.h"


	InputManager::InputManager() : window(nullptr), mouseX(0.0), mouseY(0.0),
		previousMouseX(0.0), previousMouseY(0.0), mouseSensitivity(1.0f),
		isFirstMouse(true), currentWeapon(1)
	{
		// initialize all input states to false by default
		std::memset(currentKeys, 0, sizeof(currentKeys));
		std::memset(previousKeys, 0, sizeof(previousKeys));
		std::memset(currentMouse, 0, sizeof(currentMouse));
		std::memset(previousMouse, 0, sizeof(previousMouse));
	}

	// dtor
	InputManager::~InputManager()
	{
		Shutdown();
	}

	void InputManager::Initialize(GLFWwindow* glfwWindow)
	{
		window = glfwWindow;

		if (!window) // if window is unable to open
		{
			std::cerr << "[InputManager] error: invalid window pointer" << std::endl;
			return;
		}

		std::cout << "[InputManager] Successfully initialized for rhythm game input." << std::endl;
	}

	void InputManager::Update()
	{
		if (!window) return;

		// update input state
		UpdateKeyStates();
		UpdateMouseStates();
	}

	void InputManager::UpdateKeyStates()
	{
		// store previous key frame states
		std::memcpy(previousKeys, currentKeys, sizeof(currentKeys));

		// iterate through current keyboard states
		for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++)
		{
			currentKeys[key] = (glfwGetKey(window, key) == GLFW_PRESS);
		}
	}

	void InputManager::UpdateMouseStates()
	{
		// store previous key frame states
		std::memcpy(previousMouse, currentMouse, sizeof(currentMouse));

		// iterate through current keyboard states
		for (int button = GLFW_MOUSE_BUTTON_1; button <= GLFW_MOUSE_BUTTON_LAST; button++)
		{
			currentMouse[button] = (glfwGetMouseButton(window, button) == GLFW_PRESS);
		}

		// remember to update mouse cursor position
		glfwGetCursorPos(window, &mouseX, &mouseY);

		if (isFirstMouse) {
			previousMouseX = mouseX;
			previousMouseY = mouseY;
			isFirstMouse = false;
		}
	}

	// RETURNING OF KEY STATES (BOOL) * CALCULATION OF MOUSE STATS
	bool InputManager::IsKeyPressed(int key)
	{
		if (key < 0 || key > GLFW_KEY_LAST) return false;
		return currentKeys[key];
	}

	bool InputManager::IsKeyJustPressed(int key)
	{
		if (key < 0 || key > GLFW_KEY_LAST) return false;
		return currentKeys[key] && !previousKeys[key];
	}

	bool InputManager::IsKeyReleased(int key)
	{
		if (key < 0 || key > GLFW_KEY_LAST) return false;
		return !currentKeys[key] && previousKeys[key];
	}

	bool InputManager::IsMousePressed(int button)
	{
		if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
		return currentMouse[button];
	}

	bool InputManager::IsMouseJustPressed(int button)
	{
		if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
		return currentMouse[button] && !previousMouse[button];
	}

	void InputManager::GetMousePosition(double& x, double& y)
	{
		x = mouseX;
		y = mouseY;
	}

	void InputManager::GetMouseDelta(double& deltaX, double& deltaY)
	{
		deltaX = (mouseX - previousMouseX) * mouseSensitivity;
		deltaY = (mouseY - previousMouseY) * mouseSensitivity;
	}

	void InputManager::SetMouseSensitivity(float sensitivity)
	{
		mouseSensitivity = sensitivity;
	}


	// specific rhythm game functions 
	bool InputManager::IsRhythmKeyPressed(int key) {
		return (key == GLFW_KEY_Q || key == GLFW_KEY_W ||
			key == GLFW_KEY_E || key == GLFW_KEY_R) && IsKeyJustPressed(key);
	}

	bool InputManager::IsWeaponSwitchPressed(int key) {
		return (key == GLFW_KEY_1 || key == GLFW_KEY_2 || key == GLFW_KEY_3) && IsKeyJustPressed(key);
	}

	int InputManager::GetCurrentWeaponSelected() {
		return currentWeapon;
	}

	bool InputManager::IsAiming() {
		return IsMousePressed(GLFW_MOUSE_BUTTON_RIGHT);
	}

	double InputManager::GetInputTiming() {
		return glfwGetTime(); // get simple timing using glfw
	}

	void InputManager::RunM1Test()
	{
		// testing of the rhythm keys Q/W/E/R (core rhythm gameplay)
		if (IsKeyJustPressed(GLFW_KEY_Q))
		{
			double timing = GetInputTiming();
			std::cout << "[M1 BEAT1 TEST] Q pressed at: " << timing << "s - Rhythm Beat 1" << std::endl;
		}
		if (IsKeyJustPressed(GLFW_KEY_W))
		{
			double timing = GetInputTiming();
			std::cout << "[M1 BEAT2 TEST] W pressed at: " << timing << "s - Rhythm Beat 2" << std::endl;
		}
		if (IsKeyJustPressed(GLFW_KEY_E))
		{
			double timing = GetInputTiming();
			std::cout << "[M1 BEAT3 TEST] E pressed at: " << timing << "s - Rhythm Beat 3" << std::endl;
		}
		if (IsKeyJustPressed(GLFW_KEY_R))
		{
			double timing = GetInputTiming();
			std::cout << "[M1 BEAT4 TEST] R pressed at: " << timing << "s - Rhythm Beat 4" << std::endl;
		}

		// test switchign of 3 weapons (1,2,3)
		if (IsKeyJustPressed(GLFW_KEY_1))
		{
			currentWeapon = 1;
			std::cout << "[M1 SPEAR TEST] Switched to Spear (1)" << std::endl;
		}
		if (IsKeyJustPressed(GLFW_KEY_2))
		{
			currentWeapon = 2;
			std::cout << "[M1 SWORD TEST] Switched to Sword (2)" << std::endl;
		}
		if (IsKeyJustPressed(GLFW_KEY_3))
		{
			currentWeapon = 3;
			std::cout << "[M1 GUN TEST] Switched to Gun (3)" << std::endl;
		}

		// test for aiming using mouse
		if (IsMousePressed(GLFW_MOUSE_BUTTON_RIGHT))
		{
			double x, y, deltaX, deltaY;
			GetMousePosition(x, y);
			GetMouseDelta(deltaX, deltaY);
			std::cout << "[M1 AIM TEST] Aiming mode - Position: (" << x << ", " << y
				<< ") Delta: (" << deltaX << ", " << deltaY << ")" << std::endl;
		}

		// test mouse input
		if (IsMouseJustPressed(GLFW_MOUSE_BUTTON_LEFT))
		{
			double x, y;
			GetMousePosition(x, y);
			std::cout << "[M1 MOUSE TEST] Left mouse button pressed at position: (" << x << ", " << y << ")" << std::endl;
			std::cout << "          UI interaction capability confirmed" << std::endl;
		}

		// test shooting or attacking while aiming
		if (IsMouseJustPressed(GLFW_MOUSE_BUTTON_LEFT))
		{
			double x, y;
			GetMousePosition(x, y);
			if (IsAiming()) {
				std::cout << "[M1 SHOOT TEST] Aimed shot fired at: (" << x << ", " << y << ")" << std::endl;
			}
			else {
				std::cout << "[M1 CLICK TEST] UI click at: (" << x << ", " << y << ")" << std::endl;
			}
		}
	}

	void InputManager::Shutdown()
	{
		if (window)
		{
			std::cout << "[InputManager] Shutdown completed" << std::endl;
			window = nullptr;
		}
	}