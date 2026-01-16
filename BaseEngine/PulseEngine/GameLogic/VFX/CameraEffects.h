#pragma once
#include <glm/vec2.hpp>

struct CameraPunchParams
{
    float zoomAmount = 0.08f;   // how much to zoom in on beat
    float zoomReturnSpeed = 10.0f; // how fast it returns
};

struct CameraShakeParams
{
    float amplitude = 0.03f; // NDC-ish offset amount
    float decay = 8.0f;      // higher = stops faster
};

class CameraEffects
{
public:
    void SetBaseZoom(float z) { mBaseZoom = z; }
    float GetBaseZoom() const { return mBaseZoom; }

    // Call when beat happens
    void TriggerBeatPunch(const CameraPunchParams& p);

    // Call when hit/miss/etc happens
    void TriggerShake(const CameraShakeParams& p);

    // Update every frame
    void Update(float dt);

    // Apply on your camera values each frame:
    //   cam.zoom = baseZoom * (1 + punchZoom)
    //   cam.center += shakeOffset
    float GetPunchZoom01() const { return mPunchZoom01; }   // 0..~zoomAmount
    glm::vec2 GetShakeOffset() const { return mShakeOffset; }

private:
    float mBaseZoom = 1.0f;

    // Beat punch state
    float mPunchZoom01 = 0.0f;
    float mPunchReturnSpeed = 10.0f;

    // Shake state
    float mShakeStrength = 0.0f;
    float mShakeDecay = 8.0f;
    glm::vec2 mShakeOffset{0.0f, 0.0f};

    // tiny RNG
    float Rand01();
};
