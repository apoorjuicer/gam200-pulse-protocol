#pragma once
#include <string>
#include <glm/vec2.hpp>
#include "ParticleSystem.h"
#include "CameraEffects.h"

class VFXManager
{
public:
    void Init(float bpm, size_t maxParticles);

    void SetBpm(float bpm);
    void Update(float dt);

    // call this when beat occurs (or when BeatTracker says “beat”)
    void OnBeat();

    // call this when judgement occurs ("MISS", "BAD", "GOOD", "PERFECT")
    void OnJudgement(const std::string& judgement, const glm::vec2& worldPos);

    // camera outputs (apply to your camera each frame)
    float GetPunchZoom01() const { return mCamFx.GetPunchZoom01(); }
    glm::vec2 GetShakeOffset() const { return mCamFx.GetShakeOffset(); }

    const std::vector<Particle>& GetParticles() const { return mParticles.GetParticles(); }

private:
    float mBpm = 120.0f;

    ParticleSystem mParticles;
    CameraEffects  mCamFx;

    // Tuning per judgement
    void SpawnForMiss(const glm::vec2& p);
    void SpawnForGood(const glm::vec2& p);
    void SpawnForPerfect(const glm::vec2& p);
};
