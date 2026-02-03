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

        if (!ui.Transitioning)
        {
            if (wasTransitioning)
            {
                // Transition just ended - snap to current slot with vibrant appearance
                if (ui.SlotsReady())
                {
                    tf.X = ui.CurrentSlotX;
                    tf.Y = ui.CurrentSlotY;
                }

                tf.RotX = 0.0f;
                tf.ScaleX = ui.currentScale;
                tf.ScaleY = ui.currentScale;

                SetSprite(ui.GetVibrantSprite(ui.CurrentType));

                wasTransitioning = false;
                capturedStart = false;
            }
            else
            {
                // Normal state - positioned at next slot, dim sprite
                tf.X = slotX;
                tf.Y = slotY;
                tf.RotX = 0.0f;
                tf.ScaleX = ui.nextScale;
                tf.ScaleY = ui.nextScale;

                SetSprite(ui.GetDimSprite(ui.NextType));
            }
            return;
        }

        // During transition: move from next slot to current slot with arc and rotation
        wasTransitioning = true;

        if (!capturedStart)
        {
            startX = tf.X;
            startY = tf.Y;
            startRotX = tf.RotX;
            capturedStart = true;
        }

        float eased = ui.EaseSmooth(ui.Move01);

        // Interpolate position
        float x = ui.Lerp(startX, ui.CurrentSlotX, eased);
        float y = ui.Lerp(startY, ui.CurrentSlotY, eased);

        // Add parabolic arc
        float arc = ui.arcHeight * 4f * eased * (1f - eased);
        y += arc;

        tf.X = x;
        tf.Y = y;

        // Rotate during transition
        tf.RotX = ui.Lerp(startRotX, startRotX + ui.rotateDegrees, eased);

        // Scale from next size to current size
        float sc = ui.Lerp(ui.nextScale, ui.currentScale, eased);
        tf.ScaleX = sc;
        tf.ScaleY = sc;

        // Switch to vibrant sprite partway through transition
        if (eased > 0.6f)
            SetSprite(ui.GetVibrantSprite(ui.CurrentType));
        else
            SetSprite(ui.GetDimSprite(ui.NextType));
    }

    private void SetSprite(string spriteName)
    {
        if (spriteName == lastSprite) return;
        lastSprite = spriteName;
        Console.WriteLine($"[OrderSlipNext] Sprite -> {spriteName}");
        // TODO: Add actual sprite changing code here based on your engine's API
        // Example: GetSprite().SetTexture(spriteName); or similar
    }
}