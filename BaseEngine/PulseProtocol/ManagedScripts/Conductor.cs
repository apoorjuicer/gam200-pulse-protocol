using System;
using ScriptAPI;


public class Conductor : Script
{
    // =========================
    // Global Access
    // =========================
    public static Conductor? Instance { get; private set; }

    // =========================
    // Tunables
    // =========================
    public float BPM = 120f;

    // =========================
    // Internal state
    // =========================
    private float secondsPerBeat;
    private float songTime;
    private float nextHalfBeat;
    private int lastBeat = -1;
    private bool initialized = false;
    private float audioTimeAccumulator = 0f; // Total time
    private float prevAudioPos = 0f;         // Last known FMOD pos
    private float cachedAdjustedTime = 0f;   // Computed once per frame, shared by all callers
    public float GetBeatDuration()
    {
        return 60.0f / BPM;
    }

    // default vals calibrated for my com but
    // LatencyCalibrator will override this thing AFTER test so every new Conductor instance reads it on init.
    public static float CalibratedOffset = 0.125f;

    // Set by VisualLatencyCalibrator — how many seconds to pre-fire visuals to compensate for display pipeline lag.
    public static float CalibratedDisplayOffset = 0.05f;

    // Compensates for FMOD output buffer lag (~90ms hardware delay).
    public float MusicLatencyOffset = 0.125f;

    // Compensates for how many seconds the display pipeline delays rendered frames.
    // Positive = pre-fire visuals earlier to compensate for a slow monitor / VSync frame-hold.
    public float DisplayLatencyOffset = 0f;

    // Compensates for where beat 1 actually starts in the BGM audio file.
    public float BeatStartOffset = 0.024f;

    public override void Update()
    {
        //if (Instance == null) { Instance = this; }
        //else if (Instance != this)
        //{
        //    Console.WriteLine("[Conductor] WARNING: Duplicate Conductor detected, disabling this one.");
        //    return;
        //}


        // just re init ah
        Instance = this;
        if (Instance == null)
            Instance = this;
        else if (Instance != this)
        {
            Console.WriteLine("[Conductor] WARNING: Duplicate Conductor detected, disabling this one.");
            initialized = true;
            return;
        }


        if (!initialized)
        {
            MusicLatencyOffset   = CalibratedOffset;
            DisplayLatencyOffset = CalibratedDisplayOffset;
            secondsPerBeat = 60.0f / BPM;
            songTime = 0.0f;
            nextHalfBeat = secondsPerBeat * 0.5f;
            lastBeat = -1;
            audioTimeAccumulator = 0f;
            prevAudioPos = 0f;

            Console.WriteLine($"[Conductor] Init | BPM={BPM} | SPB={secondsPerBeat:F3} | LatencyOffset={MusicLatencyOffset:F4}s");

            initialized = true;
            return;
        }

        if (!GameStart.Ready) return;

        // delta time usage
        float dt = Time.DeltaTime;
        //songTime += dt;

        // Instead of making the audio be linked to dt(will drift due to FPS),
        // we use Fmod to tell us the audio pos instead. Apparently this is how OSU does it? idk xd
        float audioPos = (float)GetAudio().GetBGMPosition();
        if (audioPos > 0f) // START AS SOON AS TRACK IS PLAYED
        {
            // When the loop ends, FMOD restarts the audio back to 0, so we manually add it for the beats when it does 
            if (audioPos < prevAudioPos - 1.0f)
            {
                // Save how long this loop was and updtae our beat counter so instead
                // of jumping back to beat 0 and freezing the game, it just keeps going
                audioTimeAccumulator += prevAudioPos;    
            }
            
            // where we left off
            prevAudioPos = audioPos;

            // instead of just restarting, just add it to the first loop. Cos if we reset 2 0, its stuck at beat 127 and it will nvr go pass it 
            songTime = audioTimeAccumulator + audioPos;
        }
        else
        {
            // INCASE IT NO WORK ;-;
            songTime += dt; 
        }

        // Apply both offsets: hardware latency + per-song beat phase.
        // Cached so VisualBeatSystem, ComboSystem, etc. all read the same value this frame.
        cachedAdjustedTime = songTime - MusicLatencyOffset - BeatStartOffset;
        float adjustedTime = cachedAdjustedTime;

        int currentBeat = (int)Math.Floor(adjustedTime / secondsPerBeat);

        // Full beat
        if (currentBeat > lastBeat)
        {
            lastBeat = currentBeat;
            Console.WriteLine($"[Conductor] Beat {currentBeat} | GameTime={Time.GameTime:F3} | Frame={Time.FrameCount}");
        }

        // Half beat (use while to catch up if a frame skips)
        while (adjustedTime >= nextHalfBeat)
        {
            Console.WriteLine($"[Conductor] HalfBeat near Beat {lastBeat} | GameTime={Time.GameTime:F3}");
            nextHalfBeat += secondsPerBeat;
        }
    }

    public int GetCurrentBeat() => lastBeat;

    public int GetVisualCurrentBeat()
    {
        float visualTime = cachedAdjustedTime + DisplayLatencyOffset;
        return (int)Math.Floor(visualTime / secondsPerBeat);
    }
    public float GetSongTime() => songTime;

    public float GetAdjustedSongTime() => cachedAdjustedTime;

    // Use this in VisualBeatSystem and any other purely-visual beat query.
    // Pre-fires the phase by DisplayLatencyOffset to compensate for monitor/VSync delay.
    public float GetVisualAdjustedSongTime() => cachedAdjustedTime + DisplayLatencyOffset;

    public float SecondsPerBeat => secondsPerBeat;

    // True once FMOD has started reporting a position > 0.
    // Use this to gate any system that needs a stable, audio-driven beat clock.
    public bool IsFMODSynced => prevAudioPos > 0f;

 
    public void ResetClock()
    {
        songTime = 0.0f;
        lastBeat = -1;
        nextHalfBeat = secondsPerBeat * 0.5f;
        audioTimeAccumulator = 0f;
        prevAudioPos = 0f;
        cachedAdjustedTime = 0f;
        Console.WriteLine("[Conductor] Clock reset — syncing to audio start");
        Console.WriteLine("[Conductor] ResetClock called from: " + Environment.StackTrace);
    }

    public void OnDestroy()
    {
        if (Instance == this)
            Instance = null;
    }
}