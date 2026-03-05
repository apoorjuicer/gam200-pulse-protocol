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
    public float GetBeatDuration()
    {
        return 60.0f / BPM;
    }
    // Compensates for FMOD output buffer lag (~90ms hardware delay).
    // Tune this once per machine — it should stay constant across songs.
    public float MusicLatencyOffset = 0.125f;
    // Compensates for where beat 1 actually starts in the BGM audio file.
    // If the file has silence or a pickup note before the first downbeat, set this
    // to that timestamp (in seconds). Tune this per song.
    // At 120 BPM (0.5s/beat): 0.25 = half-beat shift to align code boundary with audio beat.
    public float BeatStartOffset = 0.125f;

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

            secondsPerBeat = 60.0f / BPM;
            songTime = 0.0f;
            nextHalfBeat = secondsPerBeat * 0.5f;
            lastBeat = -1;
            audioTimeAccumulator = 0f;
            prevAudioPos = 0f;

            initialized = true;

            Console.WriteLine($"[Conductor] Init | BPM={BPM} | SPB={secondsPerBeat:F3}");
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

        // Apply both offsets: hardware latency + per-song beat phase
        float adjustedTime = songTime - MusicLatencyOffset - BeatStartOffset; // use this 2 check the hlaf beat instead

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
    public float GetSongTime() => songTime;
    public float GetAdjustedSongTime() => songTime - MusicLatencyOffset - BeatStartOffset;
    public float SecondsPerBeat => secondsPerBeat;

    // Call this the same frame PlayBGM fires so the beat clock starts
    // at t=0 in sync with audio rather than when this script initialised.
    public void ResetClock()
    {
        songTime = 0.0f;
        lastBeat = -1;
        nextHalfBeat = secondsPerBeat * 0.5f;
        audioTimeAccumulator = 0f;
        prevAudioPos = 0f;
        Console.WriteLine("[Conductor] Clock reset — syncing to audio start");
        Console.WriteLine("[Conductor] ResetClock called from: " + Environment.StackTrace);
    }

    public void OnDestroy()
    {
        if (Instance == this)
            Instance = null;
    }
}