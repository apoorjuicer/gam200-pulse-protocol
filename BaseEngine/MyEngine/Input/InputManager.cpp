#include "InputManager.h"
#include <iostream>
#include <cstring>
#include "pch/pch.h"

namespace Framework
{
	InputManager::InputManager() : window(nullptr), mouseX(0.0), mouseY(0.0)
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
	}

	// RETURNING OF KEY STATES (BOOL)
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
		return currentKeys[button];
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

	// specific rhythm game functions 
	bool InputManager::IsRhythmKeyPressed() {
		return IsKeyJustPressed(GLFW_KEY_SPACE);
	}

	bool InputManager::IsWeaponKey(int key) {
		return (key == GLFW_KEY_A || key == GLFW_KEY_S || key == GLFW_KEY_D);
	}

	double InputManager::GetInputTiming() {
		return glfwGetTime(); // get simple timing using glfw
	}

	void InputManager::RunM1Test()
	{
		// test rhythm key (aka space key)
		if (IsRhythmKeyPressed())
		{
			double timing = GetInputTiming();
			std::cout << "[M1 SPACEBAR TEST] Rhythm Key (SPACE) pressed at: " << timing << "s" << std::endl;
			std::cout << "          Beat input detected for rhythm mechanics" << std::endl;
		}

		// test mouse input
		if (IsMouseJustPressed(GLFW_MOUSE_BUTTON_LEFT))
		{
			double x, y;
			GetMousePosition(x, y);
			std::cout << "[M1 MOUSE TEST] Left mouse button pressed at position: (" << x << ", " << y << ")" << std::endl;
			std::cout << "          UI interaction capability confirmed" << std::endl;
		}

		// test simple weapon combo keys
		if (IsKeyJustPressed(GLFW_KEY_A) && IsWeaponKey(GLFW_KEY_A))
		{
			std::cout << "[M1 SPEAR TEST] Spear attack key (A) pressed" << std::endl;
		}

		if (IsKeyJustPressed(GLFW_KEY_S) && IsWeaponKey(GLFW_KEY_S))
		{
			std::cout << "[M1 SWORD TEST] Sword attack key (A) pressed" << std::endl;
		}

		if (IsKeyJustPressed(GLFW_KEY_D) && IsWeaponKey(GLFW_KEY_D))
		{
			std::cout << "[M1 BOW TEST] Bow attack key (D) pressed" << std::endl;
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
}
