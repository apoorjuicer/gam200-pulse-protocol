using System;
using ScriptAPI;

public class PlayerAnimationController : Script
{
    private Animation animation;
    private SpriteComponent sprite;
    private bool wasAnimating = false;
    private bool hasTransformed = false;  // Track if we've already transformed

    // Sprite paths
    private string normalSprite = "hero";
    private string transformedSprite = "hero_spritesheet";

    public override void Update()
    {
        animation = GetAnimation();
        sprite = GetSprite();

        // Check for successful combo and transform
        if (!hasTransformed && ComboSystem.Instance != null)
        {
            if (ComboSystem.Instance.PlayerDidSuccessfulCombo)
            {
                TransformPlayer();
            }
        }

        // When game is paused, stop the animation
        if (Application.IsPaused())
        {
            if (wasAnimating)
            {
                wasAnimating = false;
            }
            return;
        }

        // When game is playing, animate normally
        wasAnimating = true;

        // Your normal player animation logic here
        // (jumping, etc.)
    }

    private void TransformPlayer()
    {
        hasTransformed = true;

        // Change the sprite texture
        sprite.SetTexture($"../../PulseEngine/Assets/Textures/Characters/{transformedSprite}.png");

        Console.WriteLine($"[PlayerAnimation] Player transformed! Sprite changed from {normalSprite} to {transformedSprite}");
    }  // FIXED: Added missing closing brace

    public void ResetTransformation()
    {
        hasTransformed = false;
        sprite.SetTexture($"../../PulseEngine/Assets/Textures/Characters/{normalSprite}.png");
        Console.WriteLine("[PlayerAnimation] Player reset to normal sprite");
    }
}