///*********************************************************************************************
// \file      Sound.cpp
// \par		Bloop
// \author    Leu Jun Yong (l.junyong@digipen.edu) 90%
//            
//
//
//\copyright All content   2024 DigiPen Institute of Technology Singapore. All rights reserved.
//**********************************************************************************************/
//
//#include "Sound.h"
//
//namespace Framework {
//    // Define FMOD variables
//    FMOD::System* pSystem = nullptr;
//
//    bool mouseRelease = true;
//    bool pauseChecker = false;
//
//    SoundSystem::SoundSystem() {}
//
//    SoundSystem& SoundSystem::Instance() {
//        static SoundSystem instance; // Create a single static instance
//        return instance;
//    }
//
//    void SoundSystem::Initialize() {
//        // Create and initialize the FMOD system
//        FMOD::System_Create(&pSystem);
//        if (pSystem) {
//            pSystem->init(32, FMOD_INIT_NORMAL, nullptr);
//        }
//        else {
//            std::cerr << "FMOD system initialization failed." << std::endl;
//        }
//
//        AssetsManager::Instance().LoadAllSound();
//    }
//
//    void SoundSystem::PlayNew() {
//        if (!pSystem || !levelManager) return;
//
//        // Stop and release all existing channels
//        for (FMOD::Channel* channel : Instance().channels) {
//            if (channel) {
//                channel->stop(); // Stop any playing sound
//            }
//        }
//        Instance().channels.clear(); // Clear the channel list to prepare for new sounds
//
//        // Loop through GameObjects and play the BGM for the current scene
//        for (auto it = Framework::levelManager->GameObjects.GOCM.begin();
//            it != Framework::levelManager->GameObjects.GOCM.end(); it++) {
//            Music* musicComponent = dynamic_cast<Music*>(it->second->GetCom(Com_Music));
//            if (musicComponent) {
//                Instance().PlayMySound(AssetsManager::Instance().GetSound(musicComponent->MusicID)); 
//                Instance().PauseAllSounds();
//            }
//        }
//    }
//
//    void SoundSystem::PauseAllSounds() {
//        for (FMOD::Channel* channel : Instance().channels) {
//            if (channel) {
//                channel->setPaused(true); // Pause each channel
//            }
//        }
//    }
//
//    void SoundSystem::ResumeAllSounds() {
//        for (FMOD::Channel* channel : Instance().channels) {
//            if (channel) {
//                channel->setPaused(false); // Resume each channel
//            }
//        }
//    }
//
//    void SoundSystem::PlayMySound(FMOD::Sound* Soundtest) {
//        FMOD::Channel* testchannel = nullptr;
//
//        if (Soundtest) {
//            pSystem->playSound(Soundtest, nullptr, false, &testchannel);
//            if (testchannel) {
//                Instance().channels.push_back(testchannel); // Store the channel for later management
//            }
//            if (Instance().isMuted) {
//                testchannel->setMute(true); 
//            }
//        }
//    }
//
//    void SoundSystem::StopSound(FMOD::Sound* targetSound) {
//        if (!targetSound) return; // Ensure the target sound is valid
//
//        for (auto it = Instance().channels.begin(); it != Instance().channels.end(); ++it) {
//            FMOD::Channel* channel = *it;
//            if (channel) {
//                FMOD::Sound* currentSound = nullptr;
//                channel->getCurrentSound(&currentSound); // Get the sound being played on this channel
//
//                // Check if this channel is playing the target sound
//                if (currentSound == targetSound) {
//                    channel->stop(); // Stop the sound
//                    Instance().channels.erase(it); // Remove the channel from the vector
//                    break; // Exit loop once the target sound is stopped
//                }
//            }
//        }
//    }
//
//
//    void SoundSystem::Update(float timeSlice) {
//        UNREFERENCED_PARAMETER(timeSlice);
//        if (pSystem) {
//            pSystem->update();
//        }
//
//        // Handle pause functionality
//        if (GLHelper::keyPause && !pauseChecker) {
//            Instance().PauseAllSounds();
//            pauseChecker = true;
//        }
//        else if (!GLHelper::keyPause && pauseChecker) {
//            Instance().ResumeAllSounds();
//            pauseChecker = false;
//        }
//
//        // Play click sound only if not muted and no sound is currently playing
//        if (!Instance().isMuted) {
//            if (GLHelper::leftmousetriggerme && mouseRelease) {
//                Instance().PlayMySound(AssetsManager::Instance().GetSound("ClickSFX"));
//                std::cout << "HELLO THERE!" << std::endl;
//                mouseRelease = false;
//            }
//            else if (!GLHelper::leftmousetriggerme) {
//                mouseRelease = true;
//            }
//        }
//
//
//
//        // Check if "M" key is pressed
//        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_M) == GLFW_PRESS) {
//            if (!mKeyWasPressed) {
//                Instance().ToggleMute();
//                mKeyWasPressed = true;
//            }
//        }
//        else {
//            mKeyWasPressed = false;
//        }
//    }
//
//    void SoundSystem::ToggleMute() {
//        isMuted = !isMuted; // Toggle mute state
//        for (FMOD::Channel* channel : Instance().channels) {
//            if (channel) {
//                channel->setMute(isMuted); // Apply mute state to all channels
//            }
//        }
//    }
//
//    SoundSystem::~SoundSystem() {
//        // Stop all sounds and clear channels
//        for (FMOD::Channel* channel : Instance().channels) {
//            if (channel) {
//                channel->stop();
//            }
//        }
//        Instance().channels.clear();
//
//        AssetsManager::Instance().UnloadAllSound();
//
//        if (pSystem) {
//            pSystem->close();
//            pSystem->release();
//            pSystem = nullptr;
//        }
//    }
//
//    std::string SoundSystem::GetName() {
//        return "SoundSystem";
//    }
//
//    void SoundSystem::SendMessageMess(Message* message) {
//        if (message->messageId == SCENECHANGE) {
//            Instance().PlayNew();
//        }
//    }
//}
