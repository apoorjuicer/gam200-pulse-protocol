using System;
using ScriptAPI;

public class ComboKeySlot : Script
{
    public int SlotIndex = 0;

    // Sprite paths
    public string WSprite = "../../PulseEngine/Assets/Textures/UI/W.png";
    public string ASprite = "../../PulseEngine/Assets/Textures/UI/A.png";
    public string SSprite = "../../PulseEngine/Assets/Textures/UI/S.png";
    public string DSprite = "../../PulseEngine/Assets/Textures/UI/D.png";

    public float DisplayDuration = 0.5f;

    // State
    private float displayTimer = 0f;
    private bool isShowing = false;
    private bool hasRegistered = false;

    public override void Update()
    {
        if (!hasRegistered)
        {
            var sprite = GetSprite();
           
            if (!sprite.HasSprite())
            {
              return;
            }
            
            string texturePath = sprite.Texture;
            
            if (string.IsNullOrEmpty(texturePath))
            {
                return;
            }
            
            string texturePathLower = texturePath.ToLower();
            
            if (texturePathLower.Contains("w.png"))
            {
                SlotIndex = 0;
            }
            else if (texturePathLower.Contains("a.png"))
            {
                SlotIndex = 1;
            }
            else if (texturePathLower.Contains("s.png"))
            {
                SlotIndex = 2;
            }
            else if (texturePathLower.Contains("d.png"))
            {
                SlotIndex = 3;
            }
            else
            {
                Console.WriteLine($"[ComboKeySlot] ⚠️ Could not detect SlotIndex from '{texturePath}'");
                SlotIndex = 0; // Default fallback
            }
            
            // Register with manager
            ComboDisplayManager.RegisterKeySlot(SlotIndex, this);

            // Hide initially
            Hide();

            // Mark as registered
            hasRegistered = true;
            Console.WriteLine($"[ComboKeySlot] ✓ Slot {SlotIndex} registered");
        }

        // Update display timer
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
        // Get sprite component
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

        Console.WriteLine($"[ComboKeySlot] Slot {SlotIndex} showing {key}");
    }

    public void Hide()
    {
        var transform = GetTransform();
        transform.IsVisible = false;
        isShowing = false;
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