using System;
using ScriptAPI;

public class OrderSlipCurrentIcon : Script
{
    private bool initialized = false;

    private float slotX, slotY;
    private bool wasTransitioning = false;
    private string lastSprite = "";

    public override void Update()
    {
        if (!initialized)
        {
            TransformComponent tf0 = GetTransform();
            slotX = tf0.X;
            slotY = tf0.Y;

            // Wait for controller to exist before registering
            if (OrderSlipUIController.Instance != null)
            {
                OrderSlipUIController.Instance.RegisterCurrentSlot(slotX, slotY);
                initialized = true;
            }
            return;
        }

        var ui = OrderSlipUIController.Instance;
        if (ui == null) return;

        TransformComponent tf = GetTransform();
        SpriteComponent sprite = GetSprite();

        // Always stay at current slot position
        tf.X = slotX;
        tf.Y = slotY;
        tf.ScaleX = ui.currentScale;
        tf.ScaleY = ui.currentScale;

        if (!ui.Transitioning)
        {
            // Normal state: fully visible, vibrant sprite
            if (sprite.HasSprite())
            {
                sprite.TintA = 1.0f;
            }

            // Only update sprite if UI has been initialized with real enemy types
            if (ui.CurrentType != EnemyIconType.None)
            {
                SetSprite(ui.GetVibrantSprite(ui.CurrentType));
            }

            wasTransitioning = false;
            return;
        }

        // During transition: fade out
        wasTransitioning = true;

        // Fade out: alpha goes from 1.0 to 0.0
        float alpha = 1.0f - ui.CurrentFadeOut;
        if (sprite.HasSprite())
        {
            sprite.TintA = alpha;
        }

        // Keep showing the old current type sprite (only if initialized)
        if (ui.CurrentType != EnemyIconType.None)
        {
            SetSprite(ui.GetVibrantSprite(ui.CurrentType));
        }
    }

    private void SetSprite(string spriteName)
    {
        // Safety checks
        if (spriteName == lastSprite) return;
        if (spriteName == "Icon_None")
        {
            Console.WriteLine("[OrderSlipCurrent] WARNING: Attempted to load Icon_None - UI not initialized?");
            return;
        }

        SpriteComponent sprite = GetSprite();
        if (!sprite.HasSprite())
        {
            Console.WriteLine("[OrderSlipCurrent] WARNING: Entity has no sprite component");
            return;
        }

        try
        {
            lastSprite = spriteName;
            sprite.Texture = spriteName;
            Console.WriteLine($"[OrderSlipCurrent] Sprite -> {spriteName}");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[OrderSlipCurrent] ERROR: Failed to load sprite '{spriteName}': {ex.Message}");
            Console.WriteLine($"  Make sure this file exists in your Assets/Textures/ folder");
        }
    }
}