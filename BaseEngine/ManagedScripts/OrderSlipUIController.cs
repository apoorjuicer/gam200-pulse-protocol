using System;
using ScriptAPI;

public enum EnemyIconType { None, Rat, Bat, Gorilla }

public class OrderSlipUIController : Script
{
    public static OrderSlipUIController Instance;

    // Tunables (timing + motion only)
    public float transitionDuration = 0.35f;
    public float fadeDuration = 0.25f;
    public float rotateDegrees = 90.0f; // RotZ
    public float arcHeight = 30.0f;     // Y arc amount

    // State
    public EnemyIconType CurrentType { get; private set; } = EnemyIconType.None;
    public EnemyIconType NextType { get; private set; } = EnemyIconType.None;

    public bool Transitioning { get; private set; } = false;
    public float Move01 { get; private set; } = 0.0f; // 0..1
    public float Fade01 { get; private set; } = 0.0f; // 0..1

    // “Slots” captured from editor placement (positions + scales)
    public float CurrentSlotX { get; private set; }
    public float CurrentSlotY { get; private set; }
    public float CurrentSlotScaleX { get; private set; }
    public float CurrentSlotScaleY { get; private set; }

    public float NextSlotX { get; private set; }
    public float NextSlotY { get; private set; }
    public float NextSlotScaleX { get; private set; }
    public float NextSlotScaleY { get; private set; }

    private bool hasCurrentSlot = false;
    private bool hasNextSlot = false;

    // Swap token: increments when a transition finishes so icon scripts can “commit” the swap.
    public int SwapToken { get; private set; } = 0;

    private bool booted = false;

    public override void Update()
    {
        // first frame boot
        if (!booted)
        {
            Instance = this;
            booted = true;
            Console.WriteLine("[OrderSlipUI] Booted: Instance set");
            return;
        }

        if (!Transitioning) return;

        float dt = Time.DeltaTime;

        // advance fade + move
        Fade01 = (fadeDuration <= 0.0001f) ? 1.0f : (Fade01 + dt / fadeDuration);
        if (Fade01 > 1.0f) Fade01 = 1.0f;

        Move01 = (transitionDuration <= 0.0001f) ? 1.0f : (Move01 + dt / transitionDuration);
        if (Move01 > 1.0f) Move01 = 1.0f;

        if (Move01 >= 1.0f && Fade01 >= 1.0f)
        {
            Transitioning = false;
            SwapToken++; // tell icon scripts: “commit swap now”
            Console.WriteLine("[OrderSlipUI] Transition done");
        }
    }

    // Called once at game start (from a test driver / spawner / bootstrap)
    public void Init(EnemyIconType current, EnemyIconType upcoming)
    {
        CurrentType = current;
        NextType = upcoming;

        Transitioning = false;
        Move01 = 0.0f;
        Fade01 = 0.0f;

        Console.WriteLine($"[OrderSlipUI] Init Current={CurrentType}, Next={NextType}");
    }

    // Call on enemy defeat: provide the NEW upcoming enemy type
    public void AdvanceQueue(EnemyIconType newUpcoming)
    {
        if (Transitioning) return;

        // shift
        CurrentType = NextType;
        NextType = newUpcoming;

        Transitioning = true;
        Move01 = 0.0f;
        Fade01 = 0.0f;

        Console.WriteLine($"[OrderSlipUI] Advance -> Current={CurrentType}, Next={NextType}");
    }

    // Slot registration (called by icon scripts on their first Update)
    public void RegisterCurrentSlot(float x, float y, float sx, float sy)
    {
        CurrentSlotX = x; CurrentSlotY = y;
        CurrentSlotScaleX = sx; CurrentSlotScaleY = sy;
        hasCurrentSlot = true;
    }

    public void RegisterNextSlot(float x, float y, float sx, float sy)
    {
        NextSlotX = x; NextSlotY = y;
        NextSlotScaleX = sx; NextSlotScaleY = sy;
        hasNextSlot = true;
    }

    public bool SlotsReady() => hasCurrentSlot && hasNextSlot;

    // Helpers
    public float EaseSmooth(float t) => t * t * (3f - 2f * t);
    public float Lerp(float a, float b, float t) => a + (b - a) * t;

    // IMPORTANT: Use the names EXACTLY like your asset browser shows.
    // Based on your screenshot, your engine loads by sprite name WITHOUT ".png".
    public string GetVibrantSprite(EnemyIconType type)
    {
        switch (type)
        {
            case EnemyIconType.Rat: return "enemy icon rat";
            case EnemyIconType.Bat: return "enemy icon bat";
            case EnemyIconType.Gorilla: return "enemy icon gorilla";
            default: return "Icon_None";
        }
    }

    public string GetDimSprite(EnemyIconType type)
    {
        switch (type)
        {
            case EnemyIconType.Rat: return "enemy icon rat greyed";
            case EnemyIconType.Bat: return "enemy icon bat greyed";
            case EnemyIconType.Gorilla: return "enemy icon gorilla greyed";
            default: return "Icon_None";
        }
    }
}