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
            slotX = tf0.X; slotY = tf0.Y;

            if (OrderSlipUIController.Instance != null)
                OrderSlipUIController.Instance.RegisterNextSlot(slotX, slotY);

            initialized = true;
            return;
        }

        var ui = OrderSlipUIController.Instance;
        if (ui == null) return;

        TransformComponent tf = GetTransform();

        if (!ui.Transitioning)
        {
            if (wasTransitioning)
            {
                // snap as new current
                if (ui.SlotsReady())
                {
                    tf.X = ui.CurrentSlotX;
                    tf.Y = ui.CurrentSlotY;
                }

                tf.RotX = 0.0f;
                tf.ScaleX = ui.currentScale;
                tf.ScaleY = ui.currentScale;

                SetSprite(ui.GetVibrantSprite(ui.CurrentType));
            }
            else
            {
                // normal: sit at next slot as dim
                tf.X = slotX;
                tf.Y = slotY;

                tf.RotX = 0.0f;
                tf.ScaleX = ui.nextScale;
                tf.ScaleY = ui.nextScale;

                SetSprite(ui.GetDimSprite(ui.NextType));
            }

            wasTransitioning = false;
            capturedStart = false;
            return;
        }

        // Transitioning: move+arc+rotate into current
        wasTransitioning = true;

        if (!capturedStart)
        {
            startX = tf.X;
            startY = tf.Y;
            startRotX = tf.RotX;
            capturedStart = true;
        }

        float eased = ui.EaseSmooth(ui.Move01);

        float x = ui.Lerp(startX, ui.CurrentSlotX, eased);
        float y = ui.Lerp(startY, ui.CurrentSlotY, eased);

        float arc = ui.arcHeight * 4f * eased * (1f - eased);
        y += arc;

        tf.X = x;
        tf.Y = y;

        tf.RotX = ui.Lerp(startRotX, startRotX + ui.rotateDegrees, eased);

        float sc = ui.Lerp(ui.nextScale, ui.currentScale, eased);
        tf.ScaleX = sc;
        tf.ScaleY = sc;

        if (eased > 0.6f) SetSprite(ui.GetVibrantSprite(ui.CurrentType));
        else SetSprite(ui.GetDimSprite(ui.NextType));
    }

    private void SetSprite(string spriteName)
    {
        if (spriteName == lastSprite) return;
        lastSprite = spriteName;
        Console.WriteLine($"[OrderSlipNext] Sprite -> {spriteName}");
    }
}
