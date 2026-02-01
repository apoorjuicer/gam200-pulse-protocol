using System;
using ScriptAPI;

public class PauseBackgroundAnimator : Script
{
    private int currentFrame = 1;
    private float frameTimer = 0f;
    private float frameDelay = 0.08f;
    private bool initialized = false;

    public override void Update()
    {
        TransformComponent t = GetTransform();
        SpriteComponent sprite = GetSprite();
        bool isShowing = NavigationButtons.ShowPauseOverlay;
        
        t.IsVisible = isShowing;
        
        if (isShowing)
        {
            if (!initialized)
            {
                Random rand = new Random();
                currentFrame = rand.Next(1, 15);
                frameTimer = 0f;
                initialized = true;
            }
            
            float deltaTime = Time.DeltaTime;
            if (deltaTime == 0f)
                deltaTime = 0.016f;
            
            frameTimer += deltaTime;
            
            if (frameTimer >= frameDelay)
            {
                currentFrame++;
                if (currentFrame > 14)
                    currentFrame = 1;
                
                sprite.Texture = $"../../PulseEngine/Assets/Textures/UI/pause{currentFrame}.png";
                frameTimer = 0f;
            }
        }
        else
        {
            currentFrame = 1;
            frameTimer = 0f;
            initialized = false;
        }
    }
}