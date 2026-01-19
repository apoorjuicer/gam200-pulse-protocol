#define generic generic_workaround

#include "AudioAPI.h"
#include "CoreEngine/Core/ScriptingBridge.h"
#include "CoreEngine/ECS/Coordinator.h"
#include "Audio/AudioManager.h"
#include "Audio/AudioSource.h"
#include <msclr/marshal_cppstd.h>

#undef generic

using namespace System;
using namespace msclr::interop;

namespace ScriptAPI
{
    static AudioManager* GetAudioManager()
    {
        auto coord = Coordinator::GetInstance();
        if (!coord) return nullptr;

        auto sys = coord->GetSystem<AudioManager>();
        if (!sys) return nullptr;

        return sys.get();
    }

    AudioComponent::AudioComponent(int id) : EntityID(id) {}

    // Plays the AudioSource attached to THIS entity (whatever you picked in Inspector)
    void AudioComponent::Play()
    {
        auto mgr = GetAudioManager();
        if (!mgr) return;

        AudioSource* src = ScriptBridge::GetAudioSourceComponent(EntityID);
        if (!src) return;

        mgr->PlayFromAudioSource(*src);
    }

    // Optional helpers (global BGM control)
    void AudioComponent::PlayBGM(String^ name)
    {
        auto mgr = GetAudioManager();
        if (!mgr) return;
        mgr->PlayBGM(marshal_as<std::string>(name));
    }

    void AudioComponent::StopBGM()
    {
        auto mgr = GetAudioManager();
        if (!mgr) return;
        mgr->StopBGM();
    }

    void AudioComponent::PauseBGM()
    {
        auto mgr = GetAudioManager();
        if (!mgr) return;
        mgr->PauseBGM();
    }

    void AudioComponent::ResumeBGM()
    {
        auto mgr = GetAudioManager();
        if (!mgr) return;
        mgr->ResumeBGM();
    }

    void AudioComponent::SetBGMVolume(float volume)
    {
        auto mgr = GetAudioManager();
        if (!mgr) return;
        mgr->SetBGMVolume(volume);
    }

    void AudioComponent::PlaySFX(String^ name)
    {
        auto mgr = GetAudioManager();
        if (!mgr) return;
        mgr->PlaySound(marshal_as<std::string>(name));
    }
}
