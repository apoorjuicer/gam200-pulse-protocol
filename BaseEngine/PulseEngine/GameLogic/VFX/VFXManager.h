#pragma once
#include "BeatTracker.h"
#include "CameraEffects.h"
#include "ParticleSystem.h"
#include <glm/vec2.hpp>

enum class HitResult
{
    Perfect,
    Good,
    Miss
};

class VFXManager
{
public:
    void Init(float bpm);
    void Reset(float songTimeSec = 0.0f);

    // Call every frame
    void Update(float dt, float songTimeSec);

    // Gameplay calls these
    void OnHit(HitResult r, const glm::vec2& worldPos);

    // Access
    BeatTracker& Beat() { return mBeat; }
    CameraEffects& Camera() { return mCamFx; }
    ParticleSystem& Particles() { return mParticles; }

private:
    BeatTracker mBeat;
    CameraEffects mCamFx;
    ParticleSystem mParticles;
};
