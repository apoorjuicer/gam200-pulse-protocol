/******************************************************************************/
/**
* @file        PauseAnimatorBackground.cs
* @project     Pulse Protocol
* @author      Chloe Lau Rey En
* @brief       Animates the pause screen background with randomised frame timing,
*              including occasional freezes and flickers for an organic feel.
*
* @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/

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
            // 10% chance to freeze on current frame for a bit
            if (rand.NextDouble() < 0.10)
            {
                frameDelay = 0.5f + (float)(rand.NextDouble() * 0.5f); // hold 0.5 - 1.0s
                frameTimer = 0f;
                return;
            }

            // 10% chance to flicker quickly
            if (rand.NextDouble() < 0.10)
            {
                frameDelay = 0.03f; // very fast flicker
            }
            else
            {
                // Normal range but much wider: 0.06 to 0.24s
                frameDelay = 0.06f + (float)(rand.NextDouble() * 0.18f);
            }

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
            frameTimer = 0f;
        }
    }
}