using System;
using ScriptAPI;

public class SwordSlashVFX : Script
{
    public static float SpawnX = 0f;
    public static float SpawnY = 0f;
    // Duration of the attack animation VFX lives for exactly this long
    public static float SpawnDuration = 0.5f;

    private const int VFXTotalFrames = 10;
    private const int VFXRows = 5;
    private const int VFXCols = 2;

    private int initFrames = 0;
    private float elapsed = 0f;

    public override void Update()
    {
        initFrames++;
        if (initFrames < 2) return;

        if (initFrames == 2)
        {
            var tr = GetTransform();
            tr.X = SpawnX;
            tr.Y = SpawnY;
            tr.ScaleX = 1200f;
            tr.ScaleY = 450f;

            // Scale speed so all frames finish in exactly SpawnDuration seconds
            float speed = VFXTotalFrames / SpawnDuration;
            Animation anim = GetAnimation();
            anim.SetAnimation("sword_slash_vfx", VFXRows, VFXCols, VFXTotalFrames, speed);
            // fall through — start counting elapsed this same frame
        }

        elapsed += Time.DeltaTime;
        if (elapsed >= SpawnDuration)
        {
            var tr = GetTransform();
            tr.IsVisible = false;
            var health = GetHealth();
            health.isAlive = false;
        }
    }
}
