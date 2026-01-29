/*****************************************************************************
 * @file        VFXAPI.h
 * @project     Pulse Protocol
 * @author		Goh Pin Kai
 *
 * @brief       C++/CLI bridge API for triggering gameplay VFX from managed code.
 *              Exposes static functions to notify the engine of hit judgements and
 *              beat/BPM events, forwarding calls to the LogicSystem's VFX controller.
 *
 * @copyright   Copyright (C) 2026
 *              DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without
 *              prior written consent is prohibited.
 ******************************************************************************/

#pragma once

//using namespace System;

namespace ScriptAPI
{
    public enum class HitResult
    {
        Miss = 0,
        Good = 1,
        Perfect = 2
    };

    public ref class VFXAPI abstract sealed
    {
    public:
        static void PlayHit(HitResult result, float x, float y);
        static void SetBPM(float bpm);
        static void BeatPulse(); //opt
    };
}
