using System;
using ScriptAPI;

public class ComboKeySlot : Script
{
    // =========================
    // Configuration
    // =========================
    public int SlotIndex = 0;

    public string WSprite = "../../PulseEngine/Assets/Textures/UI/W.png";
    public string ASprite = "../../PulseEngine/Assets/Textures/UI/A.png";
    public string SSprite = "../../PulseEngine/Assets/Textures/UI/S.png";
    public string DSprite = "../../PulseEngine/Assets/Textures/UI/D.png";

    public float DisplayDuration = 0.5f;

    // =========================
    // Internal State
    // =========================
    private float displayTimer = 0f;
    private bool isShowing = false;
    private bool hasRegistered = false;
    private bool IsEnemySlot = false;

    // =========================
    // Update Loop
    // =========================
    public override void Update()
    {
        if (hasRegistered && !ComboDisplayManager.IsSlotRegistered(SlotIndex, this))
        {
            hasRegistered = false;
        }

        if (!hasRegistered)
        {
            var sprite = GetSprite();

            if (!sprite.HasSprite())
                return;

            string texturePath = sprite.Texture;

            if (string.IsNullOrEmpty(texturePath))
                return;

            // =========================
            // Auto-Detect Slot Index and Row
            // =========================
            string texturePathLower = texturePath.ToLower();

            // Isolate just the filename (e.g. "w.png" or "ekeyw.png")
            string fileName = System.IO.Path.GetFileName(texturePathLower);

            // Detect which row — must check enemy BEFORE single letter
            // so "ekeyw.png" doesn't also match the "w" check below
            if (fileName == "ekeyw.png") { IsEnemySlot = true; SlotIndex = 0; }
            else if (fileName == "ekeya.png") { IsEnemySlot = true; SlotIndex = 1; }
            else if (fileName == "ekeys.png") { IsEnemySlot = true; SlotIndex = 2; }
            else if (fileName == "ekeyd.png") { IsEnemySlot = true; SlotIndex = 3; }
            else if (fileName == "w.png") { IsEnemySlot = false; SlotIndex = 0; }
            else if (fileName == "a.png") { IsEnemySlot = false; SlotIndex = 1; }
            else if (fileName == "s.png") { IsEnemySlot = false; SlotIndex = 2; }
            else if (fileName == "d.png") { IsEnemySlot = false; SlotIndex = 3; }
            else
            {
                Console.WriteLine($"[ComboKeySlot] ⚠️ Could not detect slot from '{fileName}'");
                IsEnemySlot = false;
                SlotIndex = 0;
            }

            // Register with manager (routes to enemy or player dict based on IsEnemySlot)
            ComboDisplayManager.RegisterKeySlot(SlotIndex, this);

            // Hide initially
            HideInstant();

            hasRegistered = true;
            Console.WriteLine($"[ComboKeySlot] ✓ Slot {SlotIndex} registered as {(IsEnemySlot ? "Enemy" : "Player")} slot");
        }

        // =========================
        // Display Timer
        // =========================
        if (isShowing)
        {
            displayTimer -= Time.DeltaTime;

            if (displayTimer <= 0)
                Hide();
        }
    }

    // =========================
    // Display Control
    // =========================

    // Shows key with auto-hide timer — used during player input phase
    public void ShowKey(char key)
    {
        var sprite = GetSprite();

        if (!sprite.HasSprite())
        {
            Console.WriteLine("[ComboKeySlot] ERROR: No sprite component!");
            return;
        }

        sprite.Texture = GetSpriteForKey(key);

        var fade = GetFade();
        fade.SetAlpha(1.0f);

        var transform = GetTransform();
        transform.IsVisible = true;

        isShowing = true;
        displayTimer = DisplayDuration;

        Console.WriteLine($"[ComboKeySlot] Slot {SlotIndex} ({(IsEnemySlot ? "Enemy" : "Player")}) showing {key}");
    }

    // Shows key and stays visible — used during beat reveal sequence
    public void ShowKeyPersistent(char key)
    {
        var sprite = GetSprite();

        if (!sprite.HasSprite())
        {
            Console.WriteLine("[ComboKeySlot] ERROR: No sprite component!");
            return;
        }

        sprite.Texture = GetSpriteForKey(key);

        // Reset alpha to full in case it was previously faded
        var fade = GetFade();
        fade.SetAlpha(1.0f);

        var transform = GetTransform();
        transform.IsVisible = true;

        // No timer — stays visible until explicitly hidden
        isShowing = false;
        displayTimer = 0f;

        Console.WriteLine($"[ComboKeySlot] Slot {SlotIndex} ({(IsEnemySlot ? "Enemy" : "Player")}) showing {key} (persistent)");
    }

    // Fades out over duration then hides via display timer
    public void FadeOutAndHide(float duration)
    {
        var fade = GetFade();
        fade.FadeOut(duration);

        // Timer matches fade duration so Hide() fires once fade completes
        isShowing = true;
        displayTimer = duration;

        Console.WriteLine($"[ComboKeySlot] Slot {SlotIndex} ({(IsEnemySlot ? "Enemy" : "Player")}) fading out over {duration}s");
    }

    // Hides immediately and resets alpha — used at start of reveal
    public void HideInstant()
    {
        var fade = GetFade();
        fade.SetAlpha(1.0f);

        var transform = GetTransform();
        transform.IsVisible = false;
        isShowing = false;
    }

    // Hides without alpha reset — used by display timer expiry
    public void Hide()
    {
        var transform = GetTransform();
        transform.IsVisible = false;
        isShowing = false;
    }

    // =========================
    // Helper Funcs
    // =========================
    public bool GetIsEnemySlot() => IsEnemySlot;
    
    private string GetSpriteForKey(char key)
    {
        switch (key)
        {
            case 'W': return WSprite;
            case 'A': return ASprite;
            case 'S': return SSprite;
            case 'D': return DSprite;
            default:
                Console.WriteLine($"[ComboKeySlot] Unknown key: {key}");
                return WSprite;
        }
    }
}