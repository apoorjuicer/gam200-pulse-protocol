using System;
using ScriptAPI;

public class SwordSlashVFX : Script
{
    public static float SpawnX = 0f;
    public static float SpawnY = 0f;
    // Duration of the attack animation — VFX lives for exactly this long
    public static float SpawnDuration = 0.5f;

    public const int VFXTotalFrames = 10;
    public const int VFXRows = 5;
    public const int VFXCols = 2;

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

            // Scale speed so all frames finish in exactly SpawnDuration seconds
            float speed = VFXTotalFrames / SpawnDuration;
            Animation anim = GetAnimation();

            switch (EnemySpawner.enemyQueue.Peek())
            { 
                case EnemyIconType.Rat:
                    anim.SetAnimation("sword_slash_vfx", VFXRows, VFXCols, VFXTotalFrames, speed);
                    break;

                case EnemyIconType.BlueGorilla:
                    anim.SetAnimation("spear_attack_vfx", 4, 4, 13, speed);
                    break;

                case EnemyIconType.Bat:
                    anim.SetAnimation("bat_attack_vfx", 5, 5, 22, speed);
                    break;

                case EnemyIconType.Boss:
                    break;

                default:
                    anim.SetAnimation("sword_slash_vfx", VFXRows, VFXCols, VFXTotalFrames, speed);
                    // fall through — start counting elapsed this same frame
                    break;
            }

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