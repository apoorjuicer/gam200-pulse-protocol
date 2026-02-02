using System;
using ScriptAPI;

public class EnemyRhythmController : Script
{
    public string IdleSpriteSheet = "../Enemies/Rat_Idle_spritesheet_220x432";
    public string WalkSpriteSheet = "../Enemies/Rat_Walk_spritesheet_322x433";
    public float AnimationSpeed = 12f;
    public float StepDistance = 50f;  // Move 50 pixels every step

    private bool initialized = false;
    private float moveTimer = 0f;

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            initialized = true;
        }

        // Move every 0.5 seconds automatically (for testing)
        moveTimer += Time.DeltaTime;
        if (moveTimer >= 0.5f)
        {
            moveTimer = 0f;

            TransformComponent tf = GetTransform();
            tf.X -= StepDistance;
            Console.WriteLine($"[Enemy] Moved to X={tf.X}");

            // Switch to walk animation when moving
            Animation anim = GetAnimation();
            anim.SetAnimation(WalkSpriteSheet, 4, 4, 16, AnimationSpeed);
        }
    }

    private void Initialize()
    {
        Console.WriteLine("[Enemy] Init - Will move left automatically");

        TransformComponent tf = GetTransform();
        tf.ScaleX = 200f;
        tf.ScaleY = 200f;

        Animation anim = GetAnimation();
        anim.SetAnimation(IdleSpriteSheet, 4, 4, 16, AnimationSpeed);

        Console.WriteLine($"[Enemy] Starting at X={tf.X}, Y={tf.Y}");
    }
}