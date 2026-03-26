/******************************************************************************/
/**
* @file        TurnAnnouncer.cs
* @project     Pulse Protocol
* @author      Carrie Lam (70%)
*              Chloe Lau (30%)
* @brief       Displays phase announcer text with beat-synced animations to cue
*              the player on the current turn state: static "MEMORIZE!" during
*              enemy turn, pulsing brightness "GET READY..." on the prep beat,
*              and a scale-pop "INPUT NOW!" when player input opens.
*
* @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/
using System;
using ScriptAPI;

public class TurnAnnouncerUI : Script
{
    public static TurnAnnouncerUI? Instance { get; private set; }

    // =========================
    // Config
    // =========================
    public string ReadyMessage  = "GET READY...";
    public string GoMessage     = "INPUT NOW!";
    public string EnemyMessage  = "MEMORIZE!";

    // Pulse (GET READY beat)
    public float PulseSpeed    = 1.2f;  // oscillations per second (slow breath)
    public float PulseMinBrightness = 0.35f; // dim end of pulse

    // Pop + settle (INPUT NOW — stays visible, no fade)
    public float PopScale      = 1.6f;
    public float PopDuration   = 0.12f;
    public float SettleScale   = 1.1f;  // rests slightly bigger than normal after pop

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private TurnPhase lastPhase = TurnPhase.Countdown;

    private bool  _isPulsing  = false;
    private float _pulseTimer = 0f;

    private enum GoState { Idle, Popping, Settled }
    private GoState _goState  = GoState.Idle;
    private float   _goTimer  = 0f;

    private bool  _scaleCached = false;
    private float _baseScaleX  = 1f;
    private float _baseScaleY  = 1f;

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            return;
        }

        if (NavigationButtons.ShowPauseOverlay)
            return;

        if (TurnManager.Instance == null)
            return;

        TurnPhase phase = TurnManager.Instance!.GetCurrentPhase();

        // Phase transitions
        if (phase != lastPhase)
        {
            lastPhase = phase;
            OnPhaseChanged(phase);
        }

        // Pulse animation (GET READY...)
        if (_isPulsing)
        {
            _pulseTimer += Time.DeltaTime;
            // sine wave 0..1, mapped to PulseMinBrightness..1
            float sin  = (float)(Math.Sin(_pulseTimer * PulseSpeed * Math.PI * 2.0) * 0.5 + 0.5);
            float brightness = PulseMinBrightness + (1f - PulseMinBrightness) * sin;
            GetTextComponent().SetColor(brightness, brightness, brightness);
        }

        // Pop + settle animation (INPUT NOW! — stays visible)
        if (_goState == GoState.Popping)
        {
            _goTimer += Time.DeltaTime;
            float t01   = Math.Min(_goTimer / PopDuration, 1f);
            float eased = 1f - (1f - t01) * (1f - t01); // ease-out
            // springs from PopScale down to SettleScale
            float scale = PopScale + (SettleScale - PopScale) * eased;
            var tr = GetTransform();
            tr.ScaleX = _baseScaleX * scale;
            tr.ScaleY = _baseScaleY * scale;
            if (t01 >= 1f) _goState = GoState.Settled;
        }
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        Instance  = this;
        lastPhase = TurnPhase.Countdown;
        _isPulsing = false;
        _goState   = GoState.Idle;

        if (!_scaleCached)
        {
            var tr   = GetTransform();
            _baseScaleX  = tr.ScaleX;
            _baseScaleY  = tr.ScaleY;
            _scaleCached = true;
        }

        var text = GetTextComponent();
        text.SetAlignment(1);
        text.SetColor(1f, 1f, 1f);

        Hide();

        initialized = true;
        Console.WriteLine("[TurnAnnouncerUI] Initialized");
    }

    // =========================
    // Phase Change Handler
    // =========================
    private void OnPhaseChanged(TurnPhase phase)
    {
        switch (phase)
        {
            case TurnPhase.EnemyDisplay:
                StopAll();
                var et = GetTextComponent();
                et.SetText(EnemyMessage);
                et.SetColor(1f, 1f, 1f);
                et.SetVisible(true);
                Console.WriteLine($"[TurnAnnouncerUI] >> {EnemyMessage}");
                break;

            case TurnPhase.PlayerInput:
                // Beat 5 (prep): show pulsing GET READY...
                StopAll();
                var rt = GetTextComponent();
                rt.SetText(ReadyMessage);
                rt.SetColor(1f, 1f, 1f);
                rt.SetVisible(true);
                _isPulsing  = true;
                _pulseTimer = 0f;
                Console.WriteLine($"[TurnAnnouncerUI] >> {ReadyMessage} (pulsing)");
                break;

            default:
                StopAll();
                Hide();
                break;
        }
    }

    // Called from TurnManager on beatsInPhase=2 (beat 6 — player can now press)
    public void ShowGo()
    {
        StopAll();
        var text = GetTextComponent();
        text.SetText(GoMessage);
        text.SetColor(1f, 1f, 1f);
        text.SetVisible(true);

        var tr = GetTransform();
        tr.ScaleX = _baseScaleX * PopScale;
        tr.ScaleY = _baseScaleY * PopScale;

        _goState = GoState.Popping;
        _goTimer = 0f;
        Console.WriteLine($"[TurnAnnouncerUI] >> {GoMessage} (pop+fade)");
    }

    // =========================
    // Show / Hide
    // =========================
    private void StopAll()
    {
        _isPulsing = false;
        _goState   = GoState.Idle;
        _goTimer   = 0f;
        if (_scaleCached)
        {
            var tr = GetTransform();
            tr.ScaleX = _baseScaleX;
            tr.ScaleY = _baseScaleY;
        }
        GetTextComponent().SetColor(1f, 1f, 1f);
    }

    private void Hide()
    {
        GetTextComponent().SetVisible(false);
    }
}