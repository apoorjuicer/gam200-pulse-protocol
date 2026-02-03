/*****************************************************************************
 * @file        VFXManager.cpp
 * @project     Pulse Protocol
 * @author		Goh Pin Kai
 *
 * @brief       Implements VFXManager behavior: initializes particle pools and tuning,
 *              applies beat-driven camera punch, maps judgement strings to distinct
 *              particle/shake presets, and updates particles/camera each frame. When
 *              a valid song time is provided, beat crossings are detected via BeatTracker
 *              and trigger OnBeat().
 *
 * @copyright  Copyright (C) 2026 DigiPen Institute of Technology.
 *             Reproduction or disclosure of this file or its contents without the
 *             prior written consent of DigiPen Institute of Technology is prohibited.
 ******************************************************************************/

#include "VFXManager.h"
#include <algorithm>

void VFXManager::Init(float bpm, size_t maxParticles)
{
    mBpm = bpm;
    mParticles.CreatePool(maxParticles);

    // If your game uses a default zoom, set it here
    mCamFx.SetBaseZoom(1.0f);
}

void VFXManager::SetBpm(float bpm)
{
    mBpm = std::max(1.0f, bpm);
}

void VFXManager::OnBeat()
{
    PunchDesc punch;
    punch.zoomAmount = 0.06f;
    punch.zoomReturnSpeed = 12.0f;
    mCamFx.AddBeatPunch(punch);
}

void VFXManager::OnJudgement(const std::string& judgement, const glm::vec2& worldPos)
{
    // Normalize (some systems send PERFECT/GOOD/MISS)
    if (judgement == "PERFECT")      SpawnForPerfect(worldPos);
    else if (judgement == "GOOD")    SpawnForGood(worldPos);
    else                              SpawnForMiss(worldPos);
}

void VFXManager::SpawnForMiss(const glm::vec2& p)
{
    ParticleSpawnDesc d;
    d.position = p;
    d.count = 10;
    d.speedMin = 0.3f; d.speedMax = 0.9f;
    d.lifeMin = 0.10f; d.lifeMax = 0.25f;
    mParticles.SpawnBurst(d);

    ShakeDesc s;
    s.amplitude = 0.020f;
    s.decay = 14.0f;
    mCamFx.AddShake(s);
}

void VFXManager::SpawnForGood(const glm::vec2& p)
{
    ParticleSpawnDesc d;
    d.position = p;
    d.count = 18;
    d.speedMin = 0.7f; d.speedMax = 1.3f;
    d.lifeMin = 0.20f; d.lifeMax = 0.45f;
    mParticles.SpawnBurst(d);

    ShakeDesc s;
    s.amplitude = 0.020f;
    s.decay = 10.0f;
    mCamFx.AddShake(s);
}

void VFXManager::SpawnForPerfect(const glm::vec2& p)
{
    ParticleSpawnDesc d;
    d.position = p;
    d.count = 26;
    d.speedMin = 0.9f; d.speedMax = 1.8f;
    d.lifeMin = 0.25f; d.lifeMax = 0.55f;
    mParticles.SpawnBurst(d);

    ShakeDesc s;
    s.amplitude = 0.015f;
    s.decay = 9.0f;
    mCamFx.AddShake(s);
}

void VFXManager::Update(float dt, float songTimeSec)
{
    // particle + camera always update
    mParticles.Update(dt);
    mCamFx.Update(dt);

    // beat-driven stuff only if we have a valid song time
    if (songTimeSec >= 0.0f)
    {
        if (mBeat.Update(songTimeSec)) // <-- matches your BeatTracker.h
            OnBeat();
    }
}

