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
    public float SpriteScaleX = 200f;
    public float SpriteScaleY = 200f;

    // === RHYTHM SETTINGS ===
    public RhythmType RhythmType = RhythmType.Syncopated;
    public float StepDistance = 50f;

    // === COMBAT SETTINGS ===
    public float AttackDamage = 12f;
    public float Health = 15f;
    public float AttackTriggerRange = 150f;

    // === WEAKNESS ===
    public string WeaknessCombo = "ASAS";

    // === REFERENCES (set these if available) ===
    public Conductor ConductorRef = null;
    public ComboSystem ComboRef = null;

    // === INTERNAL STATE ===
    private bool initialized = false;
    private int beatCounter = 0;
    private int lastBeat = -1;
    private bool canMove = true;
    private bool recentlyAttacked = false;
    private bool isDead = false;
    private EnemyState currentState = EnemyState.Idle;
    private float attackCooldownTimer = 0f;
    private float deathTimer = 0f;
    private bool waitingToDie = false;

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            initialized = true;
            return;
        }

        if (isDead)
        {
            if (waitingToDie)
            {
                deathTimer += Time.DeltaTime;
                if (deathTimer >= 2.5f)
                {
                    TransformComponent tf = GetTransform();
                    tf.IsVisible = false;
                }
            }
            return;
        }

        if (attackCooldownTimer > 0f)
        {
            attackCooldownTimer -= Time.DeltaTime;
            if (attackCooldownTimer <= 0f)
            {
                recentlyAttacked = false;
                canMove = true;
                if (currentState == EnemyState.Attacking)
                    SetState(EnemyState.Idle);
            }
        }

        if (ConductorRef != null)
        {
            int currentBeat = ConductorRef.GetCurrentBeat();
            if (currentBeat != lastBeat)
            {
                lastBeat = currentBeat;
                HandleBeat(currentBeat);
            }
        }
    }

    private void Initialize()
    {
        Console.WriteLine("[Enemy] Initializing...");
        TransformComponent tf = GetTransform();
        tf.ScaleX = SpriteScaleX;
        tf.ScaleY = SpriteScaleY;
        SetState(EnemyState.Idle);

        if (ConductorRef != null)
        {
            beatCounter = ConductorRef.GetCurrentBeat();
            lastBeat = beatCounter;
            Console.WriteLine("[Enemy] Conductor connected");
        }
        if (ComboRef != null)
        {
            ComboRef.SetEnemyWeakness(WeaknessCombo);
            Console.WriteLine($"[Enemy] Weakness set: {WeaknessCombo}");
        }
        Console.WriteLine($"[Enemy] Ready at ({tf.X}, {tf.Y})");
    }

    private void SetState(EnemyState newState)
    {
        if (currentState == newState) return;
        Console.WriteLine($"[Enemy] {currentState} ? {newState}");
        currentState = newState;
        UpdateSpriteSheet(newState);
    }

    private void UpdateSpriteSheet(EnemyState state)
    {
        Animation anim = GetAnimation();
        switch (state)
        {
            case EnemyState.Idle:
                anim.SetAnimation(IdleSpriteSheet, IdleRows, IdleColumns, IdleFrames, AnimationSpeed);
                break;
            case EnemyState.Moving:
                anim.SetAnimation(WalkSpriteSheet, WalkRows, WalkColumns, WalkFrames, AnimationSpeed);
                break;
            case EnemyState.Attacking:
                anim.SetAnimation(AttackSpriteSheet, AttackRows, AttackColumns, AttackFrames, AnimationSpeed);
                break;
            case EnemyState.Dead:
                anim.SetAnimation(HurtSpriteSheet, HurtRows, HurtColumns, HurtFrames, AnimationSpeed / 2f);
                break;
        }
    }

    private void HandleBeat(int beat)
    {
        beatCounter++;
        bool shouldMove = false;

        if (RhythmType == RhythmType.OneBeat)
            shouldMove = true;
        else if (RhythmType == RhythmType.TwoBeat && beatCounter % 2 == 0)
            shouldMove = true;
        else if (RhythmType == RhythmType.FourBeat && beatCounter % 4 == 0)
            shouldMove = true;
        else if (RhythmType == RhythmType.Syncopated)
            shouldMove = true;

        if (shouldMove && canMove)
            Step();
    }

    private void Step()
    {
        SetState(EnemyState.Moving);
        TransformComponent myTf = GetTransform();
        myTf.X -= StepDistance;

        if (myTf.X <= AttackTriggerRange && !recentlyAttacked)
        {
            canMove = false;
            AttackSequence();
        }
    }

    private void AttackSequence()
    {
        if (recentlyAttacked || isDead) return;
        SetState(EnemyState.Attacking);
        recentlyAttacked = true;
        attackCooldownTimer = AttackFrames / AnimationSpeed;

        bool playerSucceeded = (ComboRef != null && ComboRef.PlayerDidSuccessfulCombo);
        if (ComboRef != null) ComboRef.ResetCombo();

        if (!playerSucceeded)
            Console.WriteLine($"[Enemy] Player failed!");
        else
        {
            Console.WriteLine($"[Enemy] Player succeeded! Dying...");
            Die();
        }
    }

    public void TakeDamage(float dmg)
    {
        if (isDead) return;
        Health -= dmg;
        Console.WriteLine($"[Enemy] HP: {Health}");
        if (Health <= 0) Die();
    }

    private void Die()
    {
        if (isDead) return;
        isDead = true;
        waitingToDie = true;
        deathTimer = 0f;
        SetState(EnemyState.Dead);
        Console.WriteLine("[Enemy] Defeated!");
    }
}