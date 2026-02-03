using System;
using ScriptAPI;

public class PauseBackgroundAnimator : Script
{
    private int currentFrame = 1;
    private float frameTimer = 0f;
    private float frameDelay = 0.12f;
    private bool reverse = false;
    private Random rand = new Random();

    public override void Update()
    {
        TransformComponent t = GetTransform();
        SpriteComponent sprite = GetSprite();
        bool isShowing = NavigationButtons.ShowPauseOverlay;
        
        t.IsVisible = isShowing;
        
        if (!isShowing) return;
        
        float deltaTime = Time.DeltaTime;
        if (deltaTime == 0f)
            deltaTime = 0.016f;
        
        frameTimer += deltaTime;
        
        if (frameTimer >= frameDelay)
        {
            // Ping-pong animation
            if (reverse)
            {
                currentFrame--;
                if (currentFrame < 1)
                {
                    currentFrame = 2;
                    reverse = false;
                }
            }
            else
            {
                currentFrame++;
                if (currentFrame > 14)
                {
                    currentFrame = 13;
                    reverse = true;
                }
            }
            
            sprite.Texture = $"../../PulseEngine/Assets/Textures/UI/pause{currentFrame}.png";
            
            // Slight random variation in timing
            frameDelay = 0.10f + (float)(rand.NextDouble() * 0.05f);
            frameTimer = 0f;
        }
    }
}