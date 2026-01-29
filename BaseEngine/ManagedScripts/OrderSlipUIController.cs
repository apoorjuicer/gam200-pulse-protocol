using System;
using ScriptAPI;

public enum EnemyIconType { None, Rat, Bat, Gorilla }

public class OrderSlipUIController : Script
{
    public static OrderSlipUIController Instance;

    // Tunables
    public float currentScale = 1.0f;
    public float nextScale = 0.75f;
    public float transitionDuration = 0.35f;
    public float fadeDuration = 0.25f;
    public float rotateDegrees = 90.0f;
    public float arcHeight = 30.0f;

    // State
    public EnemyIconType CurrentType { get; private set; } = EnemyIconType.None;
    public EnemyIconType NextType { get; private set; } = EnemyIconType.None;

    public bool Transitioning { get; private set; } = false;
    public float Move01 { get; private set; } = 0.0f;
    public float Fade01 { get; private set; } = 0.0f;

    public float CurrentSlotX { get; private set; }
    public float CurrentSlotY { get; private set; }
    public float NextSlotX { get; private set; }
    public float NextSlotY { get; private set; }

    private bool hasCurrentSlot = false;
    private bool hasNextSlot = false;

    private bool initialized = false;

    // Call once externally after scripts exist (or call from a bootstrap script)
    public void Init(EnemyIconType current, EnemyIconType upcoming)
    {
        CurrentType = current;
        NextType = upcoming;

        Transitioning = false;
        Move01 = 0.0f;
        Fade01 = 0.0f;

        Console.WriteLine($"[OrderSlipUI] Init Current={CurrentType}, Next={NextType}");
    }

    public void AdvanceQueue(EnemyIconType newUpcoming)
    {
        if (Transitioning) return;

        CurrentType = NextType;
        NextType = newUpcoming;

        Transitioning = true;
        Move01 = 0.0f;
        Fade01 = 0.0f;

        Console.WriteLine($"[OrderSlipUI] Advance -> Current={CurrentType}, Next={NextType}");
    }

    public override void Update()
    {
        // first-frame "OnCreate"
        if (!initialized)
        {
            Instance = this;
            initialized = true;
            return;
        }

        if (!Transitioning) return;

        float dt = Time.DeltaTime;

        Fade01 = (fadeDuration <= 0.0001f) ? 1.0f : (Fade01 + dt / fadeDuration);
        if (Fade01 > 1.0f) Fade01 = 1.0f;

        Move01 = (transitionDuration <= 0.0001f) ? 1.0f : (Move01 + dt / transitionDuration);
        if (Move01 > 1.0f) Move01 = 1.0f;

        if (Move01 >= 1.0f && Fade01 >= 1.0f)
        {
            Transitioning = false;
            Console.WriteLine("[OrderSlipUI] Transition done");
        }
    }

    public void RegisterCurrentSlot(float x, float y) { CurrentSlotX = x; CurrentSlotY = y; hasCurrentSlot = true; }
    public void RegisterNextSlot(float x, float y) { NextSlotX = x; NextSlotY = y; hasNextSlot = true; }
    public bool SlotsReady() { return hasCurrentSlot && hasNextSlot; }

    public float EaseSmooth(float t) => t * t * (3f - 2f * t);
    public float Lerp(float a, float b, float t) => a + (b - a) * t;

    public string GetVibrantSprite(EnemyIconType type)
    {
        switch (type)
        {
            case EnemyIconType.Rat: return "enemy icon rat.png";
            case EnemyIconType.Bat: return "enemy icon bat.png";
            case EnemyIconType.Gorilla: return "enemy icon gorilla.png";
            default: return "Icon_None";
        }
    }

    public string GetDimSprite(EnemyIconType type)
    {
        switch (type)
        {
            case EnemyIconType.Rat: return "enemy icon rat greyed (1).png";
            case EnemyIconType.Bat: return "enemy icon bat greyed.png";
            case EnemyIconType.Gorilla: return "enemy icon gorilla greyed.png";
            default: return "Icon_None";
        }
    }
}

