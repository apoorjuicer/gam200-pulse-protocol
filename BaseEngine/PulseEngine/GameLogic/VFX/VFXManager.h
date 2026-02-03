/*****************************************************************************
 * @file        VFXManager.h
 * @project     Pulse Protocol
 * @author		Goh Pin Kai
 *
 * @brief       Central VFX controller combining beat tracking, camera feedback, and
 *              particle bursts. Reacts to beat events and hit judgements (MISS/GOOD/
 *              PERFECT), producing camera punch/shake outputs and particle data for rendering.
 *
 * @copyright  Copyright (C) 2026 DigiPen Institute of Technology.
 *             Reproduction or disclosure of this file or its contents without the
 *             prior written consent of DigiPen Institute of Technology is prohibited.
 ******************************************************************************/

#pragma once
#include <string>
#include <glm/vec2.hpp>
#include "ParticleSystem.h"
#include "CameraEffects.h"
#include "BeatTracker.h"

 //for .NET
#include "CoreEngine/Core/ImportExport.h" // for DLL_API
// stupid warning just saying to make sure we are using same compilier type and runtime MDd. WHICH WE ARE SO WHY WARNING STILL??????????
#pragma warning(push)
#pragma warning(disable: 4251)


class DLL_API VFXManager
{
public:
    void Init(float bpm, size_t maxParticles);

    void SetBpm(float bpm);
    void Update(float dt, float songTimeSec);
    void Update(float dt) { Update(dt, -1.0f); }    //function overload for update

    // call this when beat occurs (or when BeatTracker says “beat”)
    void OnBeat();

    // call this when judgement occurs ("MISS", "GOOD", "PERFECT")
    void OnJudgement(const std::string& judgement, const glm::vec2& worldPos);

    // camera outputs (apply to your camera each frame)
    float GetPunchZoom01() const { return mCamFx.GetPunchZoom01(); }
    glm::vec2 GetShakeOffset() const { return mCamFx.GetShakeOffset(); }

    const std::vector<Particle>& GetParticles() const { return mParticles.GetParticles(); }

private:
    float mBpm = 120.0f;

    ParticleSystem mParticles;
    CameraEffects  mCamFx;
    BeatTracker    mBeat;

    // Tuning per judgement
    void SpawnForMiss(const glm::vec2& p);
    void SpawnForGood(const glm::vec2& p);
    void SpawnForPerfect(const glm::vec2& p);
};
