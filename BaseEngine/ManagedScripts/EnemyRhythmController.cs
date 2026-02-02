using System;
using ScriptAPI;

public enum EnemyState { Idle, Moving, Attacking, Dead }
public enum RhythmType { OneBeat, TwoBeat, Syncopated, FourBeat }

public class EnemyRhythmController : Script
{
    // === SPRITE SHEET SETTINGS ===
    public string IdleSpriteSheet = "../Enemies/Rat_Idle_spritesheet_220x432";
    public string WalkSpriteSheet = "../Enemies/Rat_Walk_spritesheet_322x433";
    public string AttackSpriteSheet = "../Enemies/Rattack_spritesheet_634x431";
    public string HurtSpriteSheet = "../Enemies/Rat_Hurt_spritesheet_515x428";

    // === SPRITE SHEET FRAME DATA (CORRECTED) ===
    public int IdleFrames = 16;
    public int IdleColumns = 4;
    public int IdleRows = 4;

    public int WalkFrames = 16;
    public int WalkColumns = 4;
    public int WalkRows = 4;

    public int AttackFrames = 20;
    public int AttackColumns = 5;
    public int AttackRows = 4;

    public int HurtFrames = 15;
    public int HurtColumns = 5;
    public int HurtRows = 3;

    public float AnimationSpeed = 12f;

    // === RHYTHM SETTINGS ===
    public RhythmType RhythmType = RhythmType.OneBeat;
    public float StepDistance = 1f;
    public float MoveSpeed = 4f;

    // === COMBAT SETTINGS ===
    public float AttackRange = 3.0f;
    public float AttackDamage = 10f;
    public float Health = 10f;

    // === POSITIONING ===
    public float StopOffset = 1.0f;
    public float AttackTriggerRange = 1.5f;

    // === WEAKNESS ===
    public string WeaknessCombo = "WAWA";
    public string WeaknessWeapon = "Spear";

    // === INTERNAL STATE ===
    private bool initialized = false;
    private EnemyState currentState = EnemyState.Idle;

    // === UPDATE ===
    public override void Update()
    {
        // First frame initialization
        if (!initialized)
        {
            Initialize();
            initialized = true;
            return;
        }

        // TESTING: Keyboard controls to test sprite switching
        InputComponent input = GetInput();

        // Press 1 for Idle
        if (input.IsKeyTriggered(49)) // Key '1'
        {
            Console.WriteLine("[TEST] Switching to IDLE");
            SetStateVisuals(EnemyState.Idle);
        }

        // Press 2 for Walk
        if (input.IsKeyTriggered(50)) // Key '2'
        {
            Console.WriteLine("[TEST] Switching to WALK");
            SetStateVisuals(EnemyState.Moving);
        }

        // Press 3 for Attack
        if (input.IsKeyTriggered(51)) // Key '3'
        {
            Console.WriteLine("[TEST] Switching to ATTACK");
            SetStateVisuals(EnemyState.Attacking);
        }

        // Press 4 for Hurt
        if (input.IsKeyTriggered(52)) // Key '4'
        {
                public int HurtFrames = 15;
    public int HurtColumns = 5;
    public int HurtRows = 3;
            Console.WriteLine("[TEST] Switching to HURT");
            SetStateVisuals(EnemyState.Dead);
        }
    }

    private void Initialize()
    {
        // Set initial scale
        TransformComponent tf = GetTransform();
        tf.ScaleX = 200f;
        tf.ScaleY = 200f;

        // Set initial sprite
        SetStateVisuals(EnemyState.Idle);

        Console.WriteLine("[EnemyRhythmController] INITIALIZED! Press 1-4 to test animations:");
        Console.WriteLine("  1 = Idle (16 frames)");
        Console.WriteLine("  2 = Walk (16 frames)");
        Console.WriteLine("  3 = Attack (20 frames)");
        Console.WriteLine("  4 = Hurt (15 frames)");
    }

    private void SetStateVisuals(EnemyState state)
    {
        currentState = state;

        Animation anim = GetAnimation();

        switch (state)
        {
            case EnemyState.Idle:
                anim.SetAnimation(IdleSpriteSheet, IdleRows, IdleColumns, IdleFrames, AnimationSpeed);
                Console.WriteLine($"[Rat] ? IDLE: {IdleFrames} frames ({IdleColumns}x{IdleRows})");
                break;

            case EnemyState.Moving:
                anim.SetAnimation(WalkSpriteSheet, WalkRows, WalkColumns, WalkFrames, AnimationSpeed);
                Console.WriteLine($"[Rat] ? WALK: {WalkFrames} frames ({WalkColumns}x{WalkRows})");
                break;

            case EnemyState.Attacking:
                anim.SetAnimation(AttackSpriteSheet, AttackRows, AttackColumns, AttackFrames, AnimationSpeed);
                Console.WriteLine($"[Rat] ? ATTACK: {AttackFrames} frames ({AttackColumns}x{AttackRows})");
                break;

            case EnemyState.Dead:
                anim.SetAnimation(HurtSpriteSheet, HurtRows, HurtColumns, HurtFrames, AnimationSpeed / 2f);
                Console.WriteLine($"[Rat] ? HURT: {HurtFrames} frames ({HurtColumns}x{HurtRows})");
                break;
        }
    }

    // === PUBLIC METHODS FOR EXTERNAL CONTROL ===
    public void StartMoving()
    {
        SetStateVisuals(EnemyState.Moving);
    }

    public void StartAttacking()
    {
        SetStateVisuals(EnemyState.Attacking);
    }

    public void TakeDamage(float dmg)
    {
        Health -= dmg;
        SetStateVisuals(EnemyState.Dead);

        if (Health <= 0)
            Die();
    }

    private void Die()
    {
        SetStateVisuals(EnemyState.Dead);
        Console.WriteLine("[EnemyRhythmController] Enemy defeated!");
    }
}