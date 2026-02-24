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
            tf.X = slotX;
            tf.Y = slotY;
            tf.RotX = 0.0f;
            tf.ScaleX = ui.currentScale;
            tf.ScaleY = ui.currentScale;

            SetSprite(ui.GetVibrantSprite(ui.CurrentType));
            wasTransitioning = false;
            return;
        }

        // During transition: fade out by shrinking
        wasTransitioning = true;

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

        SpriteComponent spr = GetSprite();
        if (spr.HasSprite())
        {
            spr.Texture = spriteName;
            Console.WriteLine($"[OrderSlipCurrent] Sprite -> {spriteName}");
        }
        else
        {
            Console.WriteLine("[OrderSlipCurrent] WARNING: No Renderable/Sprite component on this entity");
        }
    }
}