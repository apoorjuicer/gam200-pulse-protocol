#pragma once
#include <cstdint>

class BeatTracker
{
public:
    void SetBPM(float bpm);
    void Reset(float songTimeSec = 0.0f);

    // Call every frame with current song time (seconds)
    // Returns true if one or more beats were crossed this frame.
    bool Update(float songTimeSec);

    // Optional info
    uint64_t GetBeatIndex() const { return mBeatIndex; }
    float GetBPM() const { return mBPM; }
    float GetSecondsPerBeat() const { return mSPB; }

private:
    float    mBPM = 120.0f;
    float    mSPB = 0.5f;          // seconds per beat
    float    mNextBeatTime = 0.0f; // absolute song time
    uint64_t mBeatIndex = 0;
};
