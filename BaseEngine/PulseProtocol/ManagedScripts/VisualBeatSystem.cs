using System;
using ScriptAPI;

public class VisualBeatSystem : Script
{
    private static readonly float[] DefaultBeatOffsets = new float[] { 0f };

    // =========================
    // Singleton
    // =========================
    public static VisualBeatSystem? Instance { get; private set; }

    // =========================
    // Pulse Settings
    // =========================
    public float scaleAmp = 0.03f;      // +3% swell
    public int pulsesPerBeat = 1;       // 1=every beat, 2=half, 4=quarter
    public float width = 0.12f;         // smaller = sharper/snappier pulse

    // =========================
    // Opacity Settings
    // =========================
    public float minOpacity = 0.20f;    // 20% (when "open"/expanded)
    public float maxOpacity = 1.00f;    // 100% (when "closed"/back to base)

    // =========================
    // Judgement Flash
    // =========================
    public float flashDuration = 0.3f;  // seconds the colour flash lasts

    private float _flashTimer = 0f;
    private float _flashR = 1f, _flashG = 1f, _flashB = 1f;

    // =========================
    // Cached base scale
    // =========================
    private bool _cachedBaseScale = false;
    private float _baseScaleX = 1f;
    private float _baseScaleY = 1f;
    private TurnPhase? _batPulsePhase = null;
    private float _batPulseAnchorTime = float.MinValue;
    private int _batPulseNextIndex = 0;
    private float _batPulseTimer = 0f;
    public float discretePulseDuration = 0.10f;

    public override void Update()
    {
        Instance = this;

        TransformComponent t = GetTransform();
        SpriteComponent s = GetSprite();

        // During stage clear, keep border fully hidden and stop beat animation.
        if (StageClearElement.IsCleared)
        {
            t.IsVisible = false;
            return;
        }

        if (!t.IsVisible)
            t.IsVisible = true;

        if (Conductor.Instance == null) return;

        float spb = Conductor.Instance!.SecondsPerBeat;
        if (spb <= 0.0001f) return;

        // Cache initial scale once
        if (!_cachedBaseScale)
        {
            _baseScaleX = t.ScaleX;
            _baseScaleY = t.ScaleY;
            _cachedBaseScale = true;
        }

        float pulse = GetActivePulseStrength(spb);

        // =========================
        // Scale: open on beat, close back
        // =========================
        float scale = 1f + pulse * scaleAmp;
        t.ScaleX = _baseScaleX * scale;
        t.ScaleY = _baseScaleY * scale;

        // =========================
        // Opacity: 100% on beat, dims as it decays
        // =========================
        float opacity = maxOpacity - (1f - pulse) * (maxOpacity - minOpacity);
        s.TintA = Clamp01(opacity);

        // =========================
        // Judgement colour flash
        // PERFECT = green, GREAT = blue, MISS = red
        // Fades back to white over flashDuration seconds
        // =========================
        if (_flashTimer > 0f)
        {
            _flashTimer -= Time.DeltaTime;
            float ft = Clamp01(_flashTimer / flashDuration); // 1 at flash start, 0 at end
            s.TintR = 1f - ft * (1f - _flashR);
            s.TintG = 1f - ft * (1f - _flashG);
            s.TintB = 1f - ft * (1f - _flashB);
        }
        else
        {
            s.TintR = 1f;
            s.TintG = 1f;
            s.TintB = 1f;
        }
    }

    // Called by ComboSystem whenever a judgement fires
    public void FlashJudgement(string judgement)
    {
        _flashTimer = flashDuration;
        switch (judgement)
        {
            case "PERFECT":
                _flashR = 0f; _flashG = 1f; _flashB = 0f;   // green
                break;
            case "GREAT":
                _flashR = 0f; _flashG = 0.5f; _flashB = 1f; // blue
                break;
            case "MISS":
                _flashR = 1f; _flashG = 0f; _flashB = 0f;   // red
                break;
            default:
                _flashR = 1f; _flashG = 1f; _flashB = 1f;
                break;
        }
    }

    private float Clamp01(float v)
    {
        if (v < 0f) return 0f;
        if (v > 1f) return 1f;
        return v;
    }

    private float GetActivePulseStrength(float spb)
    {
        if (Conductor.Instance == null)
            return 0f;

        float songTime = Conductor.Instance.GetVisualAdjustedSongTime();
        float w = Math.Max(0.0001f, width);

        if (TryGetBatPulseAnchor(out TurnPhase phase, out float anchorTime))
        {
            return UpdateDiscreteBatPulse(phase, anchorTime, songTime);
        }

        ResetBatPulseState();

        int ppb = Math.Max(1, pulsesPerBeat);
        float subBeat = spb / ppb;
        float normalPhase = songTime / subBeat;
        normalPhase = normalPhase - (float)Math.Floor(normalPhase);
        if (normalPhase > 0.5f) normalPhase -= 1.0f;
        return (float)Math.Exp(-(normalPhase * normalPhase) / (2f * w * w));
    }

    private bool TryGetBatPulseAnchor(out TurnPhase phase, out float anchorTime)
    {
        phase = TurnPhase.Countdown;
        anchorTime = 0f;

        if (EnemyRhythmController.Instance == null || TurnManager.Instance == null)
            return false;

        if (!(EnemyRhythmController.Instance is Bat) || EnemyRhythmController.Instance.IsDying)
            return false;

        phase = TurnManager.Instance.GetCurrentPhase();
        if (phase == TurnPhase.EnemyDisplay)
        {
            anchorTime = TurnManager.Instance.GetPhaseStartVisualTime();
            return true;
        }

        if (phase == TurnPhase.PlayerInput)
        {
            anchorTime = TurnManager.Instance.GetPlayerInputStartTime();
            return true;
        }

        return false;
    }

    private float UpdateDiscreteBatPulse(TurnPhase phase, float anchorTime, float songTime)
    {
        if (_batPulsePhase != phase || Math.Abs(_batPulseAnchorTime - anchorTime) > 0.0001f)
        {
            _batPulsePhase = phase;
            _batPulseAnchorTime = anchorTime;
            _batPulseNextIndex = 0;
            _batPulseTimer = 0f;
        }

        float[] offsets = EnemyRhythmController.Instance?.SlotOffsets ?? DefaultBeatOffsets;
        float elapsed = songTime - anchorTime;

        while (_batPulseNextIndex < offsets.Length && elapsed >= offsets[_batPulseNextIndex])
        {
            _batPulseTimer = discretePulseDuration;
            _batPulseNextIndex++;
        }

        if (_batPulseTimer <= 0f)
            return 0f;

        float pulse = _batPulseTimer / Math.Max(0.0001f, discretePulseDuration);
        _batPulseTimer -= Time.DeltaTime;
        if (_batPulseTimer < 0f) _batPulseTimer = 0f;
        return Clamp01(pulse);
    }

    private void ResetBatPulseState()
    {
        _batPulsePhase = null;
        _batPulseAnchorTime = float.MinValue;
        _batPulseNextIndex = 0;
        _batPulseTimer = 0f;
    }
}
