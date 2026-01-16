#include "VFXManager.h"

void VFXManager::Init(float bpm)
{
    mBeat.SetBPM(bpm);
    mParticles.CreatePool(2000);   // instead of Reserve()
}

void VFXManager::Reset(float songTimeSec)
{
    mBeat.Reset(songTimeSec);
    mParticles.Clear();
}

void VFXManager::Update(float dt, float songTimeSec)
{
    if (mBeat.Update(songTimeSec))
    {
        // Beat pulse: small zoom punch
        CameraPunchParams punch;
        punch.zoomAmount = 0.06f;
        punch.zoomReturnSpeed = 12.0f;
        mCamFx.TriggerBeatPunch(punch);
    }

    mCamFx.Update(dt);
    mParticles.Update(dt);
}

void VFXManager::OnHit(HitResult r, const glm::vec2& worldPos)
{
    // Shake strength based on hit
    CameraShakeParams shake;
    ParticleSpawnDesc burst;
    burst.position = worldPos;

    switch (r)
    {
    case HitResult::Perfect:
        shake.amplitude = 0.02f; shake.decay = 10.0f;
        burst.count = 18;
        break;
    case HitResult::Good:
        shake.amplitude = 0.012f; shake.decay = 9.0f;
        burst.count = 10;
        break;
    case HitResult::Miss:
        shake.amplitude = 0.03f; shake.decay = 7.0f;
        burst.count = 6;
        break;
    }

    mCamFx.TriggerShake(shake);
    mParticles.SpawnBurst(burst);
}
