using System;
using ScriptAPI;

public class Conductor : Script
{
    // =========================
    // Global Access
    // =========================
    public static Conductor Instance { get; private set; }

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

    public float GetBeatDuration()
    {
        return 60.0f / BPM;
    }

    public override void Update()
    {
        if (!initialized)
        {
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

            secondsPerBeat = 60.0f / BPM;
            songTime = 0.0f;
            nextHalfBeat = secondsPerBeat * 0.5f;
            lastBeat = -1;

            initialized = true;

            Console.WriteLine($"[Conductor] Init | BPM={BPM} | SPB={secondsPerBeat:F3}");
            return;
        }

        if (Instance != this)
            return;

        // delta time usage
        float dt = Time.DeltaTime;
        songTime += dt;

        int currentBeat = (int)Math.Floor(songTime / secondsPerBeat);

        // Full beat
        if (currentBeat > lastBeat)
        {
            lastBeat = currentBeat;
            Console.WriteLine($"[Conductor] Beat {currentBeat} | GameTime={Time.GameTime:F3} | Frame={Time.FrameCount}");
        }

        // Half beat (use while to catch up if a frame skips)
        while (songTime >= nextHalfBeat)
        {
            Console.WriteLine($"[Conductor] HalfBeat near Beat {lastBeat} | GameTime={Time.GameTime:F3}");
            nextHalfBeat += secondsPerBeat * 0.5f;
        }
    }

    public int GetCurrentBeat() => lastBeat;
    public float GetSongTime() => songTime;
    public float SecondsPerBeat => secondsPerBeat;

    public void OnDestroy()
    {
        if (Instance == this)
            Instance = null;
    }
}