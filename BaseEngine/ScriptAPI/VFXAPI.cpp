/*****************************************************************************
 * @file        VFXAPI.cpp
 * @project     Pulse Protocol
 * @author		Goh Pin Kai
 *
 * @brief       Implements the VFXAPI managed-to-native forwarding layer.
 *              Resolves the engine Coordinator and LogicSystem at runtime and
 *              invokes VFX callbacks (judgement, beat pulse, BPM updates). Also
 *              converts HitResult values into judgement strings used by the VFX system.
 *
 * @copyright   Copyright (C) 2026
 *              DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without
 *              prior written consent is prohibited.
 ******************************************************************************/
#pragma once
#define generic generic_workaround
#include "VFXAPI.h"
#include "../GameLogic/GameLogic.h"          // LogicSystem
#include "../CoreEngine/ECS/Coordinator.h"        // Coordinator::GetInstance()
#include <msclr/marshal_cppstd.h>   // for converting C# to C++ string
#undef generic


namespace ScriptAPI
{
    static System::String^ ToJudgementString(HitResult r)
    {
        switch (r)
        {
        case HitResult::Perfect: return L"PERFECT";
        case HitResult::Good:    return L"GOOD";
        default:                 return L"MISS";
        }

    }

    void VFXAPI::PlayHit(HitResult result, float x, float y)
    {
        // ITS BEING CALLED YIPPEEEEEEEEEEEEEEEEEEEEEE
        System::Console::WriteLine("VFXAPI: PlayHit called with Result {0} at {1},{2}", result, x, y);

        auto* c = Coordinator::GetInstance();
        if (!c) return;

        auto logic = c->GetSystem<LogicSystem>();
        if (!logic) return;

        
        std::string judgement = msclr::interop::marshal_as<std::string>(ToJudgementString(result));
        logic->GetVfx().OnJudgement(judgement, glm::vec2{x, y});
    }

    void VFXAPI::BeatPulse()
    {
        auto* c = Coordinator::GetInstance();
        if (!c) return;

        auto logic = c->GetSystem<LogicSystem>();
        if (!logic) return;

        logic->GetVfx().OnBeat();
    }

    void VFXAPI::SetBPM(float bpm)  //opt
    {
        auto* c = Coordinator::GetInstance();
        if (!c) return;

        auto logic = c->GetSystem<LogicSystem>();
        if (!logic) return;

        logic->GetVfx().SetBpm(bpm);
    }
}
