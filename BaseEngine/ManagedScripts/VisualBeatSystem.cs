using System;
using ScriptAPI;

public class VisualBeatSystem : Script
{
    // =========================
    // Pulse Settings
    // =========================
    public float scaleAmp = 0.03f;      // +3% swell
    public int pulsesPerBeat = 1;       // 1=every beat, 2=half, 4=quarter
    public float width = 0.12f;         // smaller = sharper/snappier pulse

    // =========================
    // Opacity Settings (your request)
    // =========================
    public float minOpacity = 0.20f;    // 20% (when "open"/expanded)
    public float maxOpacity = 1.00f;    // 100% (when "closed"/back to base)

    // =========================
    // Cached base scale
    // =========================
    private bool _cachedBaseScale = false;
    private float _baseScaleX = 1f;
    private float _baseScaleY = 1f;

    public override void Update()
    {
        if (Conductor.Instance == null) return;

        float spb = Conductor.Instance!.SecondsPerBeat;
        if (spb <= 0.0001f) return;

        TransformComponent t = GetTransform();
        SpriteComponent s = GetSprite();

        // Cache initial scale once
        if (!_cachedBaseScale)
        {
            _baseScaleX = t.ScaleX;
            _baseScaleY = t.ScaleY;
            _cachedBaseScale = true;
        }

        // =========================
        // Beat phase (0..1)
        // =========================
        float songTime = Conductor.Instance!.GetSongTime();

        int ppb = Math.Max(1, pulsesPerBeat);
        float subBeat = spb / ppb;

        float phase = songTime / subBeat;
        phase = phase - (float)Math.Floor(phase); // 0..1

        // =========================
        // Pulse curve (high at beat, decays as it closes)
        // =========================
        float w = Math.Max(0.0001f, width);
        float pulse = (float)Math.Exp(-(phase * phase) / (2f * w * w)); // ~1 at beat, -> 0 after

        // =========================
        // Scale: open on beat, close back
        // =========================
        float scale = 1f + pulse * scaleAmp;
        t.ScaleX = _baseScaleX * scale;
        t.ScaleY = _baseScaleY * scale;

        // =========================
        // Opacity: 20% when open, ramps to 100% as it closes
        // =========================
        float opacity = minOpacity + (1f - pulse) * (maxOpacity - minOpacity);
        s.TintA = Clamp01(opacity);
    }

    private float Clamp01(float v)
    {
        if (v < 0f) return 0f;
        if (v > 1f) return 1f;
        return v;
    }
}