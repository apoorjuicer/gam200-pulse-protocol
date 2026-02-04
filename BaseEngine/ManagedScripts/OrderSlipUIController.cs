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
    // State
    public EnemyIconType CurrentType { get; private set; } = EnemyIconType.None;
    public EnemyIconType NextType { get; private set; } = EnemyIconType.None;

    public bool Transitioning { get; private set; } = false;
    public float Move01 { get; private set; } = 0.0f;
    public float Fade01 { get; private set; } = 0.0f;

    public float CurrentFadeOut => 1.0f - Fade01;  // Current icon fades out
    public float NextFadeIn => Fade01;             // Next icon fades in

    public float CurrentSlotX { get; private set; }
    public float CurrentSlotY { get; private set; }
    public float NextSlotX { get; private set; }
    public float NextSlotY { get; private set; }

    private bool hasCurrentSlot = false;
    private bool hasNextSlot = false;

    private bool initialized = false;

    public override void Update()
    {
        // First-frame setup - establish singleton immediately
        if (!initialized)
        {
            if (Instance != null && Instance != this)
            {
                Console.WriteLine("[OrderSlipUI] WARNING: Multiple OrderSlipUIController instances detected!");
            }
            Instance = this;
            initialized = true;
            Console.WriteLine("[OrderSlipUI] Controller initialized");
            return;
        }

        if (!Transitioning) return;

        float dt = Time.DeltaTime;

        // Update fade progress
        Fade01 = (fadeDuration <= 0.0001f) ? 1.0f : (Fade01 + dt / fadeDuration);
        if (Fade01 > 1.0f) Fade01 = 1.0f;

        // Update movement progress
        Move01 = (transitionDuration <= 0.0001f) ? 1.0f : (Move01 + dt / transitionDuration);
        if (Move01 > 1.0f) Move01 = 1.0f;

        // End transition when both animations complete
        if (Move01 >= 1.0f && Fade01 >= 1.0f)
        {
            Transitioning = false;
            Console.WriteLine("[OrderSlipUI] Transition complete");
        }
    }

    // Call once externally after scripts exist (or from a bootstrap script)
    public void Init(EnemyIconType current, EnemyIconType upcoming)
    {
        if (Transitioning)
        {
            Console.WriteLine("[OrderSlipUI] WARNING: Init called during transition - ignoring");
            return;
        }

        CurrentType = current;
        NextType = upcoming;

        Transitioning = false;
        Move01 = 0.0f;
        Fade01 = 0.0f;

        Console.WriteLine($"[OrderSlipUI] Init Current={CurrentType}, Next={NextType}");
    }

    public void AdvanceQueue(EnemyIconType newUpcoming)
    {
        if (Transitioning)
        {
            Console.WriteLine("[OrderSlipUI] WARNING: AdvanceQueue called during transition - ignoring");
            return;
        }

        if (!SlotsReady())
        {
            Console.WriteLine("[OrderSlipUI] WARNING: Slots not registered yet - cannot advance");
            return;
        }

        // Shift queue forward
        CurrentType = NextType;
        NextType = newUpcoming;

        // Start transition
        Transitioning = true;
        Move01 = 0.0f;
        Fade01 = 0.0f;

        Console.WriteLine($"[OrderSlipUI] Advance -> Current={CurrentType}, Next={NextType}");
    }

    public void RegisterCurrentSlot(float x, float y)
    {
        CurrentSlotX = x;
        CurrentSlotY = y;
        hasCurrentSlot = true;
        Console.WriteLine($"[OrderSlipUI] Current slot registered at ({x}, {y})");
    }

    public void RegisterNextSlot(float x, float y)
    {
        NextSlotX = x;
        NextSlotY = y;
        hasNextSlot = true;
        Console.WriteLine($"[OrderSlipUI] Next slot registered at ({x}, {y})");
    }

    public bool SlotsReady()
    {
        return hasCurrentSlot && hasNextSlot;
    }

    // Easing function for smooth animation
    public float EaseSmooth(float t) => t * t * (3f - 2f * t);

    // Linear interpolation
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