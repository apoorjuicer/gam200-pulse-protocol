using System;
using ScriptAPI;

public enum EnemyState { Idle, Moving, Attacking, Dead }
public enum RhythmType { OneBeat, TwoBeat, Syncopated, FourBeat }

public class EnemyRhythmController : Script
{
    // =========================
    // Global Access (Singleton)
    // =========================
    public static EnemyRhythmController Instance { get; private set; }

    // === SPRITE SHEET SETTINGS ===
    public string IdleSpriteSheet = "../Enemies/Rat_Idle_spritesheet_220x432";
    public string WalkSpriteSheet = "../Enemies/Rat_Walk_spritesheet_322x433";
    public string AttackSpriteSheet = "../Enemies/Rattack_spritesheet_634x431";
    public string HurtSpriteSheet = "../Enemies/Rat_Hurt_spritesheet_515x428";

    public int IdleFrames = 15;
    public int IdleColumns = 4;
    public int IdleRows = 4;

    public int WalkFrames = 15;
    public int WalkColumns = 4;
    public int WalkRows = 4;

    public int AttackFrames = 20;
    public int AttackColumns = 5;
    public int AttackRows = 4;

    public int HurtFrames = 15;
    public int HurtColumns = 4;
    public int HurtRows = 4;

    public float AnimationSpeed = 12f;

    // Base size for consistent visual appearance
    public float BaseWidth = 200f;
    public float BaseHeight = 200f;

    // === RHYTHM SETTINGS ===
    public RhythmType RhythmType = RhythmType.Syncopated;
    public float StepDistance = 50f;

    // === COMBAT SETTINGS ===
    public float AttackRange = 25f;
    public float AttackDamage = 12f;
    public float Health = 15f;

    // === PLAYER DETECTION ===
    public float PlayerX = 400f;
    public float PlayerY = 300f;

    // === WEAKNESS ===
    public string WeaknessCombo = "WASD";  // CHANGED FROM "ASAS" TO MATCH YOUR COMBO SYSTEM

    // === INTERNAL STATE ===
    private bool initialized = false;
    private int lastBeat = -1;
    private bool isDead = false;
    private EnemyState currentState = EnemyState.Idle;

    private float attackCooldownTimer = 0f;
    private float deathTimer = 0f;
    private bool waitingToDie = false;

    // Track movement during enemy turn
    private int movementStepsTaken = 0;
    private int maxMovementSteps = 4;

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
                if (currentState == EnemyState.Attacking)
                    SetState(EnemyState.Idle);
            }
        }

        // Check distance to player (for attack range detection)
        TransformComponent myTf = GetTransform();
        float distanceToPlayer = Math.Abs(myTf.X - PlayerX);

        // Movement logic - only during appropriate phases
        if (Conductor.Instance != null)
        {
            int currentBeat = Conductor.Instance.GetCurrentBeat();
            if (currentBeat != lastBeat)
            {
                lastBeat = currentBeat;
                HandleBeat(currentBeat);
            }
        }

        // Check if enemy reached player and should attack
        if (distanceToPlayer <= AttackRange && currentState != EnemyState.Attacking)
        {
            // Don't attack during enemy display or player input phases
            if (TurnManager.Instance != null)
            {
                TurnPhase phase = TurnManager.Instance.GetCurrentPhase();
                if (phase == TurnPhase.Resolution)
                {
                    AttackPlayer();
                }
            }
        }
    }

    private void Initialize()
    {
        // Set singleton
        if (Instance == null)
            Instance = this;

        Console.WriteLine("[Enemy] Initializing...");

        TransformComponent tf = GetTransform();
        SetScaleForSprite(IdleSpriteSheet, 220, 432);
        SetState(EnemyState.Idle);

        if (Conductor.Instance != null)
        {
            lastBeat = Conductor.Instance.GetCurrentBeat();
            Console.WriteLine("[Enemy] Conductor connected");
        }

        // CRITICAL: Set the weakness combo in the ComboSystem
        if (ComboSystem.Instance != null)
        {
            ComboSystem.Instance.SetEnemyWeakness(WeaknessCombo);
            Console.WriteLine($"[Enemy] Weakness set to: {WeaknessCombo}");
        }
        else
        {
            Console.WriteLine("[Enemy] WARNING: ComboSystem.Instance is null!");
        }

        Console.WriteLine($"[Enemy] Initialized at ({tf.X}, {tf.Y}) | Health: {Health}");
    }

    private void SetScaleForSprite(string spriteName, float spriteWidth, float spriteHeight)
    {
        TransformComponent tf = GetTransform();
        float targetSize = 200f;
        float scaleX = targetSize / spriteWidth;
        float scaleY = targetSize / spriteHeight;
        float uniformScale = Math.Min(scaleX, scaleY);

        tf.ScaleX = spriteWidth * uniformScale;
        tf.ScaleY = spriteHeight * uniformScale;
    }

    private void SetState(EnemyState newState)
    {
        if (currentState == newState) return;

        Console.WriteLine($"[Enemy] State: {currentState} ? {newState}");
        currentState = newState;
        UpdateSpriteSheet(newState);
    }

    private void UpdateSpriteSheet(EnemyState state)
    {
        Animation anim = GetAnimation();

        switch (state)
        {
            case EnemyState.Idle:
                SetScaleForSprite(IdleSpriteSheet, 220, 432);
                anim.SetAnimation(IdleSpriteSheet, IdleRows, IdleColumns, IdleFrames, AnimationSpeed);
                break;

            case EnemyState.Moving:
                SetScaleForSprite(WalkSpriteSheet, 322, 433);
                anim.SetAnimation(WalkSpriteSheet, WalkRows, WalkColumns, WalkFrames, AnimationSpeed);
                break;

            case EnemyState.Attacking:
                SetScaleForSprite(AttackSpriteSheet, 634, 431);
                anim.SetAnimation(AttackSpriteSheet, AttackRows, AttackColumns, AttackFrames, AnimationSpeed);
                break;

            case EnemyState.Dead:
                SetScaleForSprite(HurtSpriteSheet, 515, 428);
                anim.SetAnimation(HurtSpriteSheet, HurtRows, HurtColumns, HurtFrames, AnimationSpeed / 2f);
                break;
        }
    }

    // ========================================================================
    // CRITICAL FIX: This is where your turn-based movement logic happens
    // ========================================================================
    private void HandleBeat(int beat)
    {
        if (TurnManager.Instance == null)
            return;

        TurnPhase currentPhase = TurnManager.Instance.GetCurrentPhase();

        // ============================================================
        // PHASE 1: COUNTDOWN (First 4 beats)
        // Enemy stays IDLE, doesn't move
        // ============================================================
        if (currentPhase == TurnPhase.Countdown)
        {
            SetState(EnemyState.Idle);
            Console.WriteLine($"[Enemy] Countdown - staying idle (Beat {beat})");
            return;
        }

        // ============================================================
        // PHASE 2: ENEMY DISPLAY (Next 4 beats)
        // Enemy MOVES one step per beat (4 steps total)
        // Player input is LOCKED
        // ============================================================
        if (currentPhase == TurnPhase.EnemyDisplay)
        {
            // Only move if we haven't moved all 4 steps yet
            if (movementStepsTaken < maxMovementSteps)
            {
                Step();
                movementStepsTaken++;
                Console.WriteLine($"[Enemy] Enemy Display - moved step {movementStepsTaken}/{maxMovementSteps}");
            }
            return;
        }

        // ============================================================
        // PHASE 3: PLAYER INPUT (Next 4 beats)
        // Enemy FREEZES (doesn't move)
        // Player can now input combo
        // ============================================================
        if (currentPhase == TurnPhase.PlayerInput)
        {
            SetState(EnemyState.Idle);
            // Don't log every beat, just once when phase starts
            return;
        }

        // ============================================================
        // PHASE 4: RESOLUTION
        // Check combo result, handled by TurnManager
        // ============================================================
    }

    private void Step()
    {
        SetState(EnemyState.Moving);

        TransformComponent myTf = GetTransform();
        myTf.X -= StepDistance;  // Move LEFT towards player

        Console.WriteLine($"[Enemy] Stepped to X={myTf.X:F1}, Distance to player: {Math.Abs(myTf.X - PlayerX):F1}");
    }

    private void AttackPlayer()
    {
        if (isDead) return;

        Console.WriteLine($"[Enemy] ATTACKING PLAYER for {AttackDamage} damage!");
        SetState(EnemyState.Attacking);

        attackCooldownTimer = AttackFrames / AnimationSpeed;

        // TODO: Apply damage to player
        // if (PlayerHealthSystem.Instance != null)
        // {
        //     PlayerHealthSystem.Instance.TakeDamage(AttackDamage);
        // }
    }

    // ========================================================================
    // PUBLIC API - Called by TurnManager
    // ========================================================================

    public void TakeDamage(float dmg)
    {
        if (isDead) return;

        Health -= dmg;
        Console.WriteLine($"[Enemy] Took {dmg} damage! HP: {Health}/{Health + dmg}");

        if (Health <= 0)
        {
            Die();
        }
    }

    private void Die()
    {
        if (isDead) return;

        isDead = true;
        waitingToDie = true;
        deathTimer = 0f;
        SetState(EnemyState.Dead);

        Console.WriteLine("[Enemy] ?? DEFEATED!");
        Console.WriteLine("==============================================");
        Console.WriteLine("VICTORY! Enemy defeated!");
        Console.WriteLine("==============================================");
    }

    // Called by TurnManager when player FAILS combo
    public void ExecuteFailurePunishment()
    {
        Console.WriteLine("[Enemy] ExecuteFailurePunishment() called");

        // Calculate how many steps we DIDN'T move during enemy display
        int remainingSteps = maxMovementSteps - movementStepsTaken;

        Console.WriteLine($"[Enemy] Player failed! Moving {remainingSteps} remaining steps");

        // Move those remaining steps NOW
        for (int i = 0; i < remainingSteps; i++)
        {
            Step();
        }

        // Then attack
        AttackPlayer();
    }

    // Called by TurnManager at start of EACH enemy display phase
    public void ResetMovementForNewTurn()
    {
        Console.WriteLine("[Enemy] ResetMovementForNewTurn() - resetting step counter");
        movementStepsTaken = 0;
        SetState(EnemyState.Idle);
    }
}