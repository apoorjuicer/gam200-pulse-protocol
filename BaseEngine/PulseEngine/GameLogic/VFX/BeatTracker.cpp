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
