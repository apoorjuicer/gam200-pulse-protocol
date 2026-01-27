#include "CameraEffects.h"
#include <cstdlib>
#include <algorithm>
#include <cmath>

//static float Clamp01(float x) { return std::max(0.0f, std::min(1.0f, x)); }

float CameraEffects::Rand01()
{
    return float(std::rand()) / float(RAND_MAX);
}

void CameraEffects::TriggerBeatPunch(const CameraPunchParams& p)
{
    // Add a punch immediately
    mPunchZoom01 = std::max(mPunchZoom01, p.zoomAmount);
    mPunchReturnSpeed = p.zoomReturnSpeed;
}

void CameraEffects::TriggerShake(const CameraShakeParams& p)
{
    mShakeStrength = std::max(mShakeStrength, p.amplitude);
    mShakeDecay = p.decay;
}

void CameraEffects::Update(float dt)
{
    // Return punch back to 0
    if (mPunchZoom01 > 0.0f)
    {
        mPunchZoom01 -= mPunchReturnSpeed * dt * mPunchZoom01; // exponential-ish
        if (mPunchZoom01 < 0.0001f) mPunchZoom01 = 0.0f;
    }

    // Shake
    if (mShakeStrength > 0.0f)
    {
        // Random direction each frame
        float ax = (Rand01() * 2.0f - 1.0f);
        float ay = (Rand01() * 2.0f - 1.0f);
        mShakeOffset = glm::vec2(ax, ay) * mShakeStrength;

        // Decay
        mShakeStrength -= mShakeDecay * dt * mShakeStrength;
        if (mShakeStrength < 0.0001f)
        {
            mShakeStrength = 0.0f;
            mShakeOffset = glm::vec2(0.0f);
        }
    }
    else
    {
        mShakeOffset = glm::vec2(0.0f);
    }
}
