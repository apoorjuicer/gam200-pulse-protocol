/*****************************************************************************
 * @file        BeatTracker.cpp
 * @project     Pulse Protocol
 * @author		Goh Pin Kai
 *
 * @brief       Implements BeatTracker timing logic: converts BPM to seconds-per-beat,
 *              supports reset at an arbitrary song time, and advances beat state in
 *              Update() with catch-up for large delta times (multiple beats in one frame).
 *
 * @copyright  Copyright (C) 2026 DigiPen Institute of Technology.
 *             Reproduction or disclosure of this file or its contents without the
 *             prior written consent of DigiPen Institute of Technology is prohibited.
 ******************************************************************************/

#include "BeatTracker.h"

void BeatTracker::SetBPM(float bpm)
{
    if (bpm <= 0.0f) bpm = 120.0f;
    mBPM = bpm;
    mSPB = 60.0f / mBPM;
}

void BeatTracker::Reset(float songTimeSec)
{
    mBeatIndex = 0;
    mNextBeatTime = songTimeSec; // beat triggers immediately at this time
}

bool BeatTracker::Update(float songTimeSec)
{
    bool beatTriggered = false;

    // Handle large dt: catch up multiple beats if needed
    while (songTimeSec >= mNextBeatTime)
    {
        beatTriggered = true;
        mBeatIndex++;
        mNextBeatTime += mSPB;
    }

    return beatTriggered;
}
