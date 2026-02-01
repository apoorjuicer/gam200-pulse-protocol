using System;
using ScriptAPI;

public class OrderSlipCurrentIcon : Script
{
    private bool booted = false;
    private bool isCurrentSlot = true; // this script starts as current slot
    private int lastSwapToken = 0;

    public override void Update()
    {
        var ui = OrderSlipUIController.Instance;
        if (ui == null) return;

        // One-time: register this entity’s editor placement as “current slot”
        if (!booted)
        {
            booted = true;

            TransformComponent tf = GetTransform();
            ui.RegisterCurrentSlot(tf.X, tf.Y, tf.ScaleX, tf.ScaleY);

            ApplySprite(ui.GetVibrantSprite(ui.CurrentType));
            Console.WriteLine($"[OrderSlipCurrent] Sprite -> {ui.GetVibrantSprite(ui.CurrentType)}");
            return;
        }

        // If transition finished, commit swap when SwapToken changes
        if (ui.SwapToken != lastSwapToken)
        {
            lastSwapToken = ui.SwapToken;
            isCurrentSlot = !isCurrentSlot; // swap roles

            TransformComponent tf = GetTransform();

            if (isCurrentSlot)
            {
                // now I'm the current slot
                tf.X = ui.CurrentSlotX;
                tf.Y = ui.CurrentSlotY;
                tf.ScaleX = ui.CurrentSlotScaleX;
                tf.ScaleY = ui.CurrentSlotScaleY;
                tf.RotZ = 0.0f;

                ApplySprite(ui.GetVibrantSprite(ui.CurrentType));
                Console.WriteLine($"[OrderSlipCurrent] (now current) -> {ui.GetVibrantSprite(ui.CurrentType)}");
            }
            else
            {
                // now I'm the next slot
                tf.X = ui.NextSlotX;
                tf.Y = ui.NextSlotY;
                tf.ScaleX = ui.NextSlotScaleX;
                tf.ScaleY = ui.NextSlotScaleY;
                tf.RotZ = 0.0f;

                ApplySprite(ui.GetDimSprite(ui.NextType));
                Console.WriteLine($"[OrderSlipCurrent] (now next) -> {ui.GetDimSprite(ui.NextType)}");
            }
        }

        // During transition: current slot can “fade” (if you don't have alpha, just keep it)
        // IMPORTANT: do NOT force scale here unless you're intentionally animating.
        // We leave the scale as whatever the editor set (cached in controller).
    }

    private void ApplySprite(string spriteName)
    {
        // Most consistent with your engine bridge: SpriteComponent exists as GetSprite()
        SpriteComponent sp = GetSprite();
        sp.SpriteName = spriteName;
    }
}