using System;
using ScriptAPI;

public class HealthBarController : Script
{
    public static HealthBarController Instance { get; private set; }

    private string[] healthFrames = new string[]
    {
        "../../PulseEngine/Assets/Textures/UI/healthbars/tile001.png",
        "../../PulseEngine/Assets/Textures/UI/healthbars/tile002.png",
        "../../PulseEngine/Assets/Textures/UI/healthbars/tile003.png",
        "../../PulseEngine/Assets/Textures/UI/healthbars/tile004.png",
        "../../PulseEngine/Assets/Textures/UI/healthbars/tile005.png",
        "../../PulseEngine/Assets/Textures/UI/healthbars/tile006.png",
        "../../PulseEngine/Assets/Textures/UI/healthbars/tile007.png",
        "../../PulseEngine/Assets/Textures/UI/healthbars/tile008.png",
        "../../PulseEngine/Assets/Textures/UI/healthbars/tile009.png",
        "../../PulseEngine/Assets/Textures/UI/healthbars/tile010.png",
        "../../PulseEngine/Assets/Textures/UI/healthbars/tile012.png"
    };

    // Match the inspector values
    private float forceScaleX = 400.0f;
    private float forceScaleY = 184.253f;

    private int totalFrames = 11;
    private bool initialized = false;
    private int lastFrame = -1;

    public override void Update()
    {
        if (!initialized)
        {
            if (Instance == null)
                Instance = this;

            var anim = GetAnimation();
            anim.SetAnimationSpeed(0f);

            // Show full health
            var sprite = GetSprite();
            if (sprite.HasSprite())
            {
                sprite.Texture = healthFrames[0];
            }

            // Force transform size
            var transform = GetTransform();
            transform.ScaleX = forceScaleX;
            transform.ScaleY = forceScaleY;

            lastFrame = 0;
            initialized = true;
            Console.WriteLine("[HealthBarController] Initialized");
            return;
        }

        var t = GetTransform();
        t.ScaleX = forceScaleX;
        t.ScaleY = forceScaleY;

        if (PlayerHealth.Instance != null)
        {
            float healthPercent = PlayerHealth.Instance.GetHealthPercent();

            int frame = (int)Math.Round((1.0f - healthPercent) * (totalFrames - 1));
            if (frame < 0) frame = 0;
            if (frame >= totalFrames) frame = totalFrames - 1;

            if (frame != lastFrame)
            {
                var sprite = GetSprite();
                if (sprite.HasSprite())
                {
                    sprite.Texture = healthFrames[frame];
                }

                t.ScaleX = forceScaleX;
                t.ScaleY = forceScaleY;

                lastFrame = frame;
                Console.WriteLine($"[HealthBarController] HP: {healthPercent * 100:F0}% -> Frame {frame}");
            }
        }
    }
}