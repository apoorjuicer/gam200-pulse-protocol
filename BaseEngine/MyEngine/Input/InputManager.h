// ==================================================================================
// InputManager.h
// Author: Chloe Lau Rey En
// Brief: Header file for InputManager class providing keyboard and mouse input 
//        handling for rhythm-action game. Supports Q/W/E/R rhythm inputs, 
//        1/2/3 weapon switching, and mouse aiming functionality.
// ==================================================================================

#pragma once
#include "pch/pch.h"



	// input manager handles keyboard input and mouse input states
	// provides frame-based input detection and rhythm game timing support
	class InputManager :// public Systems
	{
	public:
		InputManager();
		~InputManager(); // destructor

		void Initialize(GLFWwindow* window);
		void Update();
		void Shutdown();

		// basic input detection
		bool IsKeyPressed(int key);
		bool IsKeyJustPressed(int key);
		bool IsKeyReleased(int key);

		bool IsMousePressed(int button);
		bool IsMouseJustPressed(int button);

		// mouse tracking
		void GetMousePosition(double& x, double &y);
		void GetMouseDelta(double& deltaX, double& deltaY);
		void SetMouseSensitivity(float sensitivity);

		// specific rhythm game functions 
		bool IsRhythmKeyPressed(int key); // Q-W-E-R for the rhythm beats (max 4 beats) 
		bool IsWeaponSwitchPressed(int key); // 1-2-3 for weapon switching
		int GetCurrentWeaponSelected(); // gets the value of the weapon currently selected
		bool IsAiming();
		double GetInputTiming();

		// for M1 test
		void RunM1Test();

	private:
		GLFWwindow* window;

		// tracking of the differnt input states
		bool currentKeys[GLFW_KEY_LAST + 1];
		bool previousKeys[GLFW_KEY_LAST + 1];
		bool currentMouse[GLFW_MOUSE_BUTTON_LAST + 1];
		bool previousMouse[GLFW_MOUSE_BUTTON_LAST + 1];

		// for tracking of mouse
		double mouseX, mouseY;
		double previousMouseX, previousMouseY;
		float mouseSensitivity;
		bool isFirstMouse;

		// index of each weapon
		int currentWeapon;  // 1 = Spear, 2 = Sword, 3 = Gun

		// internal helper func
		void UpdateKeyStates();
		void UpdateMouseStates();
	};
