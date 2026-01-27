#include "CameraEffects.h"
#include <cstdlib>

float CameraEffects::Rand01()
{
    return float(std::rand()) / float(RAND_MAX);
}

void CameraEffects::AddBeatPunch(const PunchDesc& d)
{
    // “Punch” just sets the target amount; Update() eases it back
    mPunchZoom01 = (mPunchZoom01 < d.zoomAmount) ? d.zoomAmount : mPunchZoom01;
    mPunchReturnSpeed = d.zoomReturnSpeed;
}

void CameraEffects::AddShake(const ShakeDesc& d)
{
    // Stack shake strength (keep max)
    mShakeStrength = (mShakeStrength < d.amplitude) ? d.amplitude : mShakeStrength;
    mShakeDecay = d.decay;
}

void CameraEffects::Update(float dt)
{
    // Punch return to 0
    if (mPunchZoom01 > 0.0f)
    {
        mPunchZoom01 -= mPunchReturnSpeed * dt;
        if (mPunchZoom01 < 0.0f) mPunchZoom01 = 0.0f;
    }

    // Shake decay
    if (mShakeStrength > 0.0f)
    {
        mShakeStrength -= mShakeDecay * dt;
        if (mShakeStrength < 0.0f) mShakeStrength = 0.0f;
    }

    // Random offset each frame while shaking
    if (mShakeStrength > 0.0f)
    {
        float ox = (Rand01() * 2.0f - 1.0f) * mShakeStrength;
        float oy = (Rand01() * 2.0f - 1.0f) * mShakeStrength;
        mShakeOffset = { ox, oy };
    }
    else
    {
        mShakeOffset = { 0.0f, 0.0f };
    }
}
