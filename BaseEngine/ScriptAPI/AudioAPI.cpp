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

    static float Normalize01(float v)
    {
        if (v < 0.0f) return 0.0f;
        if (v > 1.0f) return 1.0f;
        return v;
    }

    AudioComponent::AudioComponent(int id) : EntityID(id) {}

    void AudioComponent::Play()
    {
        auto mgr = GetAudioManager();
        if (!mgr) return;

        AudioSource* src = ScriptBridge::GetAudioSourceComponent(EntityID);
        if (!src) return;

        mgr->PlayFromAudioSource(*src);
    }

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

    void AudioComponent::SetMasterVolume(float volume01)
    {
        auto mgr = GetAudioManager();
        if (!mgr) return;

        float v = Normalize01(volume01);
        mgr->mMasterVol = v;
        mgr->applyMasterVolume();
    }

    void AudioComponent::SetMusicVolume(float volume01)
    {
        auto mgr = GetAudioManager();
        if (!mgr) return;

        float v = Normalize01(volume01);
        mgr->mMusicVol = v;
        mgr->applyMusicVolume();
    }

    void AudioComponent::SetSfxVolume(float volume01)
    {
        auto mgr = GetAudioManager();
        if (!mgr) return;

        float v = Normalize01(volume01);
        mgr->mSfxVol = v;
        mgr->applySfxVolume();
    }

    float AudioComponent::GetMasterVolume()
    {
        auto mgr = GetAudioManager();
        if (!mgr) return 1.0f;
        return mgr->mMasterVol;
    }

    float AudioComponent::GetMusicVolume()
    {
        auto mgr = GetAudioManager();
        if (!mgr) return 1.0f;
        return mgr->mMusicVol;
    }

    float AudioComponent::GetSfxVolume()
    {
        auto mgr = GetAudioManager();
        if (!mgr) return 1.0f;
        return mgr->mSfxVol;
    }
}
