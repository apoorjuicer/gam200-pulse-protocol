/*********************************************************************************************
 \file      Sound.h
 \par		Bloop
 \author	Leu Jun Yong (l.junyong@digipen.edu) 100%


\copyright All content   2024 DigiPen Institute of Technology Singapore. All rights reserved.
**********************************************************************************************/

#pragma once
//#include "../Engine/Core.h"
//#include "../Engine/Message.h"
#include "fmod.hpp"
//#include "ISystem.h"
#include <GLFW/glfw3.h>
//#include "../Engine/Precompiled.h"
//#include "../Managers/LevelManager.h"
//#include "../Components/Music.h"
//#include "../Managers/AssetsManager.h"
//#include "../Components/SFX.h"
#include "../Graphics/glhelper.h"
#include <iostream>

//namespace Framework {
//
//	extern FMOD::System* pSystem;
//
//	class SoundSystem :public ISystem {
//	public:
//		static SoundSystem& Instance(); // Singleton accessor
//		bool isMuted = false;
//
//
//		void SendMessageMess(Message* message);
//
//		///All systems are updated every game frame.
//		void Update(float timeslice);
//
//		///All systems provide a string name for debugging.
//		std::string GetName();
//
//
//		///Initialize the system.
//		void Initialize();
//
//		void PlayMySound(FMOD::Sound* pSound);
//
//		///All systems need a virtual destructor to have their destructor called 
//		SoundSystem();
//		~SoundSystem();
//
//		void PlayNew();
//		void PauseAllSounds();
//		void ResumeAllSounds();
//		void StopSound(FMOD::Sound* targetSound);
//		void ToggleMute();
//
//	private:
//		std::vector<FMOD::Channel*> channels;    // Collection of active channels 
//		bool mKeyWasPressed = false; 
//
//	};
//
//
//
//}