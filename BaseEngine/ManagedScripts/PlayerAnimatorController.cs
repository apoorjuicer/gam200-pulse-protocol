using System;
using ScriptAPI;

public class PlayerAnimationController : Script
{
    private Animation animation;
    private bool wasAnimating = false;

    public override void Update()
    {
        animation = GetAnimation();
        
        // When game is paused, stop the animation
        if (Application.IsPaused())
        {
            if (wasAnimating)
            {
                // Animation will freeze because DeltaTime = 0
                // Nothing needed here, it just works
                wasAnimating = false;
            }
            return;
        }
        
        // When game is playing, animate normally
        wasAnimating = true;
        
        // Your normal player animation logic here
        // (jumping, etc.)
    }
}