using System;
using ScriptAPI;

public class OrderSlipNextIcon : Script
{
    private bool initialized = false;

    private float slotX, slotY;
    private bool wasTransitioning = false;

    private float startX, startY, startRotX;
    private bool capturedStart = false;

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
                OrderSlipUIController.Instance.RegisterNextSlot(slotX, slotY);
                initialized = true;
            }
            return;
        }

        var ui = OrderSlipUIController.Instance;
        if (ui == null) return;

        TransformComponent tf = GetTransform();
        SpriteComponent sprite = GetSprite();

        if (!ui.Transitioning)
        {
            if (wasTransitioning)
            {
                // Just finished transitioning - become the new current icon
                tf.X = ui.CurrentSlotX;
                tf.Y = ui.CurrentSlotY;
                tf.ScaleX = ui.currentScale;
                tf.ScaleY = ui.currentScale;

                if (sprite.HasSprite())
                {
                    sprite.TintA = 1.0f;
                }

                // Only update sprite if UI has been initialized
                if (ui.CurrentType != EnemyIconType.None)
                {
                    SetSprite(ui.GetVibrantSprite(ui.CurrentType));
                }

                wasTransitioning = false;
                capturedStart = false;
            }
            else
            {
                // Normal state: sit at next slot as dim
                tf.X = slotX;
                tf.Y = slotY;
                tf.ScaleX = ui.nextScale;
                tf.ScaleY = ui.nextScale;

                if (sprite.HasSprite())
                {
                    sprite.TintA = 1.0f;
                }

                // Only update sprite if UI has been initialized
                if (ui.NextType != EnemyIconType.None)
                {
                    SetSprite(ui.GetDimSprite(ui.NextType));
                }
            }
            return;
        }

        // During transition: move to current slot and fade in
        wasTransitioning = true;

        // Move to current slot position
        tf.X = ui.CurrentSlotX;
        tf.Y = ui.CurrentSlotY;
        tf.ScaleX = ui.currentScale;
        tf.ScaleY = ui.currentScale;

        // Fade in: alpha goes from 0.0 to 1.0
        float alpha = ui.NextFadeIn;
        if (sprite.HasSprite())
        {
            sprite.TintA = alpha;
        }

        // Show vibrant sprite (it's becoming the current) - only if initialized
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
            Console.WriteLine("[OrderSlipNext] WARNING: Attempted to load Icon_None - UI not initialized?");
            return;
        }

        SpriteComponent sprite = GetSprite();
        if (!sprite.HasSprite())
        {
            Console.WriteLine("[OrderSlipNext] WARNING: Entity has no sprite component");
            return;
        }

        try
        {
            lastSprite = spriteName;
            sprite.Texture = spriteName;
            Console.WriteLine($"[OrderSlipNext] Sprite -> {spriteName}");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[OrderSlipNext] ERROR: Failed to load sprite '{spriteName}': {ex.Message}");
            Console.WriteLine($"  Make sure this file exists in your Assets/Textures/ folder");
        }
    }
}