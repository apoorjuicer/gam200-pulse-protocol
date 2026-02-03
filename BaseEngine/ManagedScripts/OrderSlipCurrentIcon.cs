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

        if (!ui.Transitioning)
        {
            if (wasTransitioning)
            {
                // Transition just ended - stay at current slot with current appearance
                tf.X = slotX;
                tf.Y = slotY;
                tf.RotX = 0.0f;
                tf.ScaleX = ui.currentScale;
                tf.ScaleY = ui.currentScale;

                SetSprite(ui.GetVibrantSprite(ui.CurrentType));
                wasTransitioning = false;
            }
            else
            {
                // Normal state - positioned at current slot, vibrant sprite
                tf.X = slotX;
                tf.Y = slotY;
                tf.RotX = 0.0f;
                tf.ScaleX = ui.currentScale;
                tf.ScaleY = ui.currentScale;

                SetSprite(ui.GetVibrantSprite(ui.CurrentType));
            }
            return;
        }

        // During transition: fade out by shrinking
        wasTransitioning = true;

        // Keep position stable during fade
        tf.X = slotX;
        tf.Y = slotY;

        float s = ui.Lerp(ui.currentScale, ui.currentScale * 0.85f, ui.Fade01);
        tf.ScaleX = s;
        tf.ScaleY = s;

        SetSprite(ui.GetVibrantSprite(ui.CurrentType));
    }

    private void SetSprite(string spriteName)
    {
        if (spriteName == lastSprite) return;
        lastSprite = spriteName;
        Console.WriteLine($"[OrderSlipCurrent] Sprite -> {spriteName}");
        // TODO: Add actual sprite changing code here based on your engine's API
        // Example: GetSprite().SetTexture(spriteName); or similar
    }
}