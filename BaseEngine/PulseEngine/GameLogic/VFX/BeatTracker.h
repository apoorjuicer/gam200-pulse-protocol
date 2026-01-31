/*****************************************************************************
 * @file        BeatTracker.h
 * @project     Pulse Protocol
 * @author		Goh Pin Kai
 *
 * @brief       Implements BeatTracker timing logic: converts BPM to seconds-per-beat,
 *              supports reset at an arbitrary song time, and advances beat state in
 *              Update() with catch-up for large delta times (multiple beats in one frame).
 *
 * @copyright   Copyright (C) 2026
 *              DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without
 *              prior written consent is prohibited.
 ******************************************************************************/

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
