using System;
using System.Collections.Generic;
using ScriptAPI;

public class LatencyCalibrator : Script
{
    public static LatencyCalibrator? Instance { get; private set; }

    // -------------------------------------------------------------------------
    // Tunables 
    // -------------------------------------------------------------------------
    public string PingSFXName   = "SFX_Border_Beat"; // SFX id to fire as the ping cue
    public int    SampleCount   = 12;           // presses to average
    public float  PingIntervalBeats = 2f;       // how often to ping (in beats)
    public string NextScene     = "MainMenu";   // scene to load when done

    // -------------------------------------------------------------------------
    // Internal state
    // -------------------------------------------------------------------------
    private bool  calibrating   = false;
    private bool  done          = false;
    private float nextPingBeat  = 0f;           // next beat index to ping on
    private float lastPingTime  = -1f;          // songTime when last ping fired
    private List<float> offsets = new List<float>();
    private float resultOffset  = 0f;
    private float lastSampleMs  = -1f;          // most recent recorded sample in ms
    private float stdDevMs      = 0f;           // std dev of all samples in ms

    public override void Update()
    {
        Instance = this;

        if (done) return;

        // no countdown, go conductor go
        GameStart.Ready = true;

        // Press Enter/Return to start calibration
        if (!calibrating)
        {
            if (Conductor.Instance == null) return;

            var inp = GetInput();
            if (inp.IsKeyTriggered(257) || inp.IsKeyTriggered(32)) // Enter=257, Space=32
            {
                calibrating = true;
                offsets.Clear();
                nextPingBeat = Conductor.Instance.GetCurrentBeat() + PingIntervalBeats;
                Console.WriteLine("[LatencyCalibrator] Starting — press SPACE each time you HEAR the ping.");
            }
            return;
        }

        if (Conductor.Instance == null) return;
        float songTime   = Conductor.Instance.GetSongTime();
        float spb        = Conductor.Instance.SecondsPerBeat;
        float currentBeat = songTime / spb;    // raw beat, no offset applied

        // Fire ping when we reach the scheduled beat
        if (currentBeat >= nextPingBeat)
        {
            lastPingTime = songTime;
            GetAudio().PlaySFX(PingSFXName);
            nextPingBeat += PingIntervalBeats;
            Console.WriteLine($"[LatencyCalibrator] Ping fired at songTime={songTime:F3}s — press SPACE when you HEAR it!");
        }

        // Record a sample when player pressess Space
        if (GetInput().IsKeyTriggered(32) && lastPingTime >= 0f) // Space=32
        {
            float offset = songTime - lastPingTime;

            // Ignore presses that arrive before the ping or suspiciously late (> 1 beat)
            if (offset > 0f && offset < spb)
            {
                offsets.Add(offset);
                lastSampleMs = offset * 1000f;
                Console.WriteLine($"[LatencyCalibrator] Sample {offsets.Count}/{SampleCount}: offset={lastSampleMs:F0}ms");
                lastPingTime = -1f; // reset so one ping = one sample
            }
        }

        // ENOUGHHHHH, take the amount and give it to the next person (condcutor)
        if (offsets.Count >= SampleCount)
        {
            float sum = 0f;
            foreach (float o in offsets) sum += o;
            resultOffset = sum / offsets.Count;

            // Compute std dev
            float variance = 0f;
            foreach (float o in offsets) variance += (o * 1000f - resultOffset * 1000f) * (o * 1000f - resultOffset * 1000f);
            stdDevMs = (float)Math.Sqrt(variance / offsets.Count);

            Conductor.CalibratedOffset = resultOffset;
            if (Conductor.Instance != null)
                Conductor.Instance.MusicLatencyOffset = resultOffset;

            Console.WriteLine($"[LatencyCalibrator] Done! Average offset = {resultOffset*1000f:F0}ms | StdDev = {stdDevMs:F0}ms — applied to Conductor.");
            done = true;
            // change scene
        }
    }

    public void OnDestroy()
    {
        if (Instance == this) Instance = null;
    }

    // Call this from a UI "Skip" button script if player wants to skip calibration
    public void Skip()
    {
        Console.WriteLine("[LatencyCalibrator] Skipped — keeping existing MusicLatencyOffset.");
        Scene.LoadScene(NextScene);
    }

    // Read-only access for display/UI scripts
    public bool IsCalibrating      => calibrating;
    public bool IsDone             => done;
    public int  GetSamplesDone()   => offsets.Count;
    public int  GetSamplesNeeded() => SampleCount;
    public float GetResultMs()     => resultOffset * 1000f;
    public float GetLastSampleMs() => lastSampleMs;
    public float GetStdDevMs()     => stdDevMs;

    // Returns a short rating string based on std dev
    // < 25ms = Excellent, 25-50ms = Good, 50-80ms = Fair, > 80ms = Inconsistent
    public string GetConsistencyRating()
    {
        if (stdDevMs < 25f)  return "Excellent";
        if (stdDevMs < 50f)  return "Good";
        if (stdDevMs < 80f)  return "Fair";
        return "Inconsistent";
    }

    public bool ShouldRetry() => stdDevMs >= 80f;
}
