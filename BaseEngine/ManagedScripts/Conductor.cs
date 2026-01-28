using System;
using ScriptAPI;

public class Conductor : Script
{
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

    public override void Update()
    {
        
        if (!initialized)
        {
            secondsPerBeat = 60.0f / BPM;
            songTime = 0.0f;
            nextHalfBeat = secondsPerBeat * 0.5f;
            lastBeat = -1;
            initialized = true;
            return;
        }

        // delta time usage
        float dt = Time.DeltaTime;
        songTime += dt;

        int currentBeat = (int)Math.Floor(songTime / secondsPerBeat);

        // Full beat
        if (currentBeat > lastBeat)
        {
            lastBeat = currentBeat;
            // Beat-triggered logic here
            // Console.WriteLine($"Beat {currentBeat}");
        }

        // Half beat
        if (songTime >= nextHalfBeat)
        {
            nextHalfBeat += secondsPerBeat * 0.5f;
            // Half-beat logic here
        }
    }

    public int GetCurrentBeat()
    {
        return lastBeat;
    }
}
