using System;
using ScriptAPI;

/// <summary>
/// Attach this script to EACH KeySlot entity (KeySlot0, KeySlot1, KeySlot2, KeySlot3)
/// Each slot manages its own display independently
/// 
/// IMPORTANT: Set SlotIndex in editor (0-3)
/// </summary>
public class ComboKeySlot : Script
{
    // Configuration
    public int SlotIndex = 0; // SET THIS IN EDITOR: 0 for KeySlot0, 1 for KeySlot1, etc.

    public string WSprite = "W.png";
    public string ASprite = "A.png";
    public string SSprite = "S.png";
    public string DSprite = "D.png";

    public float DisplayDuration = 0.5f;

    // State
    private float displayTimer = 0f;
    private bool isShowing = false;
    private bool hasRegistered = false;

    public override void Update()
    {
        // Register with manager on first update
        if (!hasRegistered)
        {
            ComboDisplayManager.RegisterKeySlot(SlotIndex, this);

            // Hide initially
            Hide();

            hasRegistered = true;
            Console.WriteLine($"[ComboKeySlot] Slot {SlotIndex} initialized and hidden");
        }

        // Continue with normal update
        if (isShowing)
        {
            displayTimer -= Time.DeltaTime;

            if (displayTimer <= 0)
            {
                Hide();
            }
        }
    }

    public void ShowKey(char key)
    {
        // Get the sprite component attached to THIS entity
        var sprite = GetSprite();
        if (!sprite.HasSprite())
        {
            Console.WriteLine("[ComboKeySlot] ERROR: No sprite component!");
            return;
        }

        // Set the appropriate texture based on key
        string texturePath = GetSpriteForKey(key);
        sprite.Texture = texturePath;

        // Make visible
        var transform = GetTransform();
        transform.IsVisible = true;

        // Start timer
        isShowing = true;
        displayTimer = DisplayDuration;

        Console.WriteLine($"[ComboKeySlot] Showing {key} sprite: {texturePath}");
    }

    public void Hide()
    {
        var transform = GetTransform();
        transform.IsVisible = false;
        isShowing = false;

        Console.WriteLine("[ComboKeySlot] Hidden");
    }

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