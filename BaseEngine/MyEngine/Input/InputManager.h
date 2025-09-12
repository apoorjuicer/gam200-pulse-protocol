#pragma once
#include <GLFW/glfw3.h>
#include "pch/pch.h"


namespace Framework
{
	// input manager handles keyboard input and mouse input states
	// provides frame-based input detection and rhythm game timing support
	class InputManager
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
		void GetMousePosition(double& x, double &y);

		// specific rhythm game functions 
		bool IsRhythmKeyPressed();
		bool IsWeaponKey(int key);
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

		double mouseX, mouseY;

		// internal helper func
		void UpdateKeyStates();
		void UpdateMouseStates();
	};
}
