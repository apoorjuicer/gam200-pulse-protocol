using System;
using ScriptAPI;

public class OrderSlipNextIcon : Script
{
    private bool booted = false;
    private bool isCurrentSlot = false; // this script starts as next slot
    private int lastSwapToken = 0;

    // For anim
    private float startX, startY, startRotZ;
    private float startSX, startSY;

    public override void Update()
    {
        var ui = OrderSlipUIController.Instance;
        if (ui == null) return;

        // One-time: register this entity’s editor placement as “next slot”
        if (!booted)
        {
            booted = true;

            TransformComponent tf = GetTransform();
            ui.RegisterNextSlot(tf.X, tf.Y, tf.ScaleX, tf.ScaleY);

            ApplySprite(ui.GetDimSprite(ui.NextType));
            Console.WriteLine($"[OrderSlipNext] Sprite -> {ui.GetDimSprite(ui.NextType)}");
            return;
        }

        // If we're transitioning and I'm the next slot, animate into current slot
        if (ui.Transitioning && !isCurrentSlot && ui.SlotsReady())
        {
            TransformComponent tf = GetTransform();

            // Capture start values at the beginning of movement (Move01 == 0)
            if (ui.Move01 <= 0.0001f)
            {
                startX = tf.X;
                startY = tf.Y;
                startRotZ = tf.RotZ;
                startSX = tf.ScaleX;
                startSY = tf.ScaleY;
            }

            float eased = ui.EaseSmooth(ui.Move01);

            // move next -> current
            float x = ui.Lerp(startX, ui.CurrentSlotX, eased);
            float y = ui.Lerp(startY, ui.CurrentSlotY, eased);

            // arc upward illusion
            float arc = ui.arcHeight * 4f * eased * (1f - eased);
            y += arc;

            tf.X = x;
            tf.Y = y;

            // rotate up
            tf.RotZ = ui.Lerp(startRotZ, startRotZ + ui.rotateDegrees, eased);

            // scale toward current slot scale (editor-defined)
            tf.ScaleX = ui.Lerp(startSX, ui.CurrentSlotScaleX, eased);
            tf.ScaleY = ui.Lerp(startSY, ui.CurrentSlotScaleY, eased);
        }

        // Commit swap when controller finishes (SwapToken changes)
        if (ui.SwapToken != lastSwapToken)
        {
            lastSwapToken = ui.SwapToken;
            isCurrentSlot = !isCurrentSlot;

            TransformComponent tf = GetTransform();

            if (isCurrentSlot)
            {
                tf.X = ui.CurrentSlotX;
                tf.Y = ui.CurrentSlotY;
                tf.ScaleX = ui.CurrentSlotScaleX;
                tf.ScaleY = ui.CurrentSlotScaleY;
                tf.RotZ = 0.0f;

                ApplySprite(ui.GetVibrantSprite(ui.CurrentType));
                Console.WriteLine($"[OrderSlipNext] (now current) -> {ui.GetVibrantSprite(ui.CurrentType)}");
            }
            else
            {
                tf.X = ui.NextSlotX;
                tf.Y = ui.NextSlotY;
                tf.ScaleX = ui.NextSlotScaleX;
                tf.ScaleY = ui.NextSlotScaleY;
                tf.RotZ = 0.0f;

                ApplySprite(ui.GetDimSprite(ui.NextType));
                Console.WriteLine($"[OrderSlipNext] (now next) -> {ui.GetDimSprite(ui.NextType)}");
            }
        }
    }

    private void ApplySprite(string spriteName)
    {
        SpriteComponent sp = GetSprite();
        sp.SpriteName = spriteName;
    }
}