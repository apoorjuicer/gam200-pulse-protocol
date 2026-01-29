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
            slotX = tf0.X; slotY = tf0.Y;

            if (OrderSlipUIController.Instance != null)
                OrderSlipUIController.Instance.RegisterCurrentSlot(slotX, slotY);

            initialized = true;
            return;
        }

        var ui = OrderSlipUIController.Instance;
        if (ui == null) return;

        TransformComponent tf = GetTransform();

        // Always anchored to current slot
        tf.X = slotX;
        tf.Y = slotY;

        if (!ui.Transitioning)
        {
            if (wasTransitioning)
            {
                // after transition ends, this becomes the "next" icon
                if (ui.SlotsReady())
                {
                    tf.X = ui.NextSlotX;
                    tf.Y = ui.NextSlotY;
                }

                tf.RotX = 0.0f;
                tf.ScaleX = ui.nextScale;
                tf.ScaleY = ui.nextScale;

                SetSprite(ui.GetDimSprite(ui.NextType));
            }
            else
            {
                tf.RotX = 0.0f;
                tf.ScaleX = ui.currentScale;
                tf.ScaleY = ui.currentScale;

                SetSprite(ui.GetVibrantSprite(ui.CurrentType));
            }

            wasTransitioning = false;
            return;
        }

        // Transitioning: "fade" via shrink
        wasTransitioning = true;
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
        // replace with real sprite setter later
    }
}
