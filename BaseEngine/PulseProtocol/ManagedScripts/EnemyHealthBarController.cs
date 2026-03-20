using System;
using ScriptAPI;

public class EnemyHealthBarController : Script
{
    // Generated from enemy health bar atlas (enemy health bar_176x33.png).
    // Each set is "full -> empty" for a fixed block count.
    private readonly string[] healthFrames2 =
    {
        "../../PulseProtocol/Assets/Textures/UI/enemy_hp2_00.png", // full
        "../../PulseProtocol/Assets/Textures/UI/enemy_hp2_01.png",
        "../../PulseProtocol/Assets/Textures/UI/enemy_hp2_02.png"  // empty
    };

    private readonly string[] healthFrames3 =
    {
        "../../PulseProtocol/Assets/Textures/UI/enemy_hp3_00.png",
        "../../PulseProtocol/Assets/Textures/UI/enemy_hp3_01.png",
        "../../PulseProtocol/Assets/Textures/UI/enemy_hp3_02.png",
        "../../PulseProtocol/Assets/Textures/UI/enemy_hp3_03.png"
    };

    private readonly string[] healthFrames4 =
    {
        "../../PulseProtocol/Assets/Textures/UI/enemy_hp4_00.png",
        "../../PulseProtocol/Assets/Textures/UI/enemy_hp4_01.png",
        "../../PulseProtocol/Assets/Textures/UI/enemy_hp4_02.png",
        "../../PulseProtocol/Assets/Textures/UI/enemy_hp4_03.png",
        "../../PulseProtocol/Assets/Textures/UI/enemy_hp4_04.png"
    };

    // Global nudges if needed.
    public float GlobalOffsetX = 0f;
    public float GlobalOffsetY = 0f;
    public float ScaleX = 176f;
    public float ScaleY = 33f;
    public float MinScaleX = 176f;
    public float MaxScaleX = 260f;
    public float ScaleXFromEnemyFactor = 0.23f;

    private int lastFrame = -1;
    private int lastBlocks = -1;
    private bool initialized = false;

    public override void Update()
    {
        if (!initialized)
        {
            initialized = true;
            var anim = GetAnimation();
            anim.SetAnimationSpeed(0f);
        }

        var barTf = GetTransform();
        var barSprite = GetSprite();

        if (EnemyRhythmController.Instance == null || EnemyRhythmController.Instance.IsDying)
        {
            barTf.IsVisible = false;
            lastFrame = -1;
            lastBlocks = -1;
            return;
        }

        barTf.IsVisible = true;

        // Follow active enemy (centered-under enemy with per-type tuning).
        var enemyTf = EnemyRhythmController.Instance.GetTransform();
        float offsetX =
            GlobalOffsetX +
            EnemyRhythmController.Instance.GetHealthBarOffsetX() +
            (enemyTf.ScaleX * EnemyRhythmController.Instance.GetHealthBarScaleOffsetXFactor());

        float offsetY =
            GlobalOffsetY +
            EnemyRhythmController.Instance.GetHealthBarOffsetY() -
            (enemyTf.ScaleY * EnemyRhythmController.Instance.GetHealthBarScaleOffsetYFactor());

        barTf.X = enemyTf.X + offsetX;
        barTf.Y = enemyTf.Y + offsetY;

        float autoScaleX = enemyTf.ScaleX * ScaleXFromEnemyFactor;
        if (autoScaleX < MinScaleX) autoScaleX = MinScaleX;
        if (autoScaleX > MaxScaleX) autoScaleX = MaxScaleX;

        // Preserve original texture aspect ratio (176x33).
        float autoScaleY = autoScaleX * (33f / 176f);
        barTf.ScaleX = autoScaleX;
        barTf.ScaleY = autoScaleY;

        int blocks = EnemyRhythmController.Instance.GetHealthBarBlockCount();
        if (blocks < 2) blocks = 2;
        if (blocks > 4) blocks = 4;

        float healthPercent = EnemyRhythmController.Instance.GetHealthPercent();
        int missingBlocks = (int)Math.Round((1.0f - healthPercent) * blocks);
        if (missingBlocks < 0) missingBlocks = 0;
        if (missingBlocks > blocks) missingBlocks = blocks;

        string[] activeFrames = healthFrames4;
        if (blocks == 2) activeFrames = healthFrames2;
        else if (blocks == 3) activeFrames = healthFrames3;

        if ((missingBlocks != lastFrame || blocks != lastBlocks) && barSprite.HasSprite())
        {
            barSprite.Texture = activeFrames[missingBlocks];
            lastFrame = missingBlocks;
            lastBlocks = blocks;
        }
    }
}
