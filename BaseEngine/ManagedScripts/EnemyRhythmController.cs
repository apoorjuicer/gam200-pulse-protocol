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
    public string WeaknessCombo = "WASD";

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

    // CRITICAL FIX: Track if we've tried to set weakness
    private bool weaknessSet = false;
    private int weaknessRetryCount = 0;
    private const int maxWeaknessRetries = 100; // Try for ~100 frames

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            initialized = true;
            return;
        }

        // CRITICAL FIX: Keep trying to set weakness until ComboSystem is ready
        if (!weaknessSet && weaknessRetryCount < maxWeaknessRetries)
        {
            TrySetWeakness();
            weaknessRetryCount++;
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

        // Try to set weakness immediately
        TrySetWeakness();

        Console.WriteLine($"[Enemy] Initialized at ({tf.X}, {tf.Y}) | Health: {Health}");
    }

    // CRITICAL FIX: Separate method that keeps trying until ComboSystem exists
    private void TrySetWeakness()
    {
        if (weaknessSet)
            return;

        if (ComboSystem.Instance != null)
        {
            ComboSystem.Instance.SetEnemyWeakness(WeaknessCombo);
            weaknessSet = true;
            Console.WriteLine($"[Enemy] ? Weakness successfully set to: {WeaknessCombo}");
        }
        else if (weaknessRetryCount == 0)
        {
            Console.WriteLine("[Enemy] ?? ComboSystem.Instance is null - will retry...");
        }
        else if (weaknessRetryCount == maxWeaknessRetries - 1)
        {
            Console.WriteLine("[Enemy] ? ERROR: ComboSystem never initialized!");
        }
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

    private void HandleBeat(int beat)
    {
        if (TurnManager.Instance == null)
            return;

        TurnPhase currentPhase = TurnManager.Instance.GetCurrentPhase();

        // PHASE 1: COUNTDOWN
        if (currentPhase == TurnPhase.Countdown)
        {
            SetState(EnemyState.Idle);
            Console.WriteLine($"[Enemy] Countdown - staying idle (Beat {beat})");
            return;
        }

        // PHASE 2: ENEMY DISPLAY
        if (currentPhase == TurnPhase.EnemyDisplay)
        {
            if (movementStepsTaken < maxMovementSteps)
            {
                Step();
                movementStepsTaken++;
                Console.WriteLine($"[Enemy] Enemy Display - moved step {movementStepsTaken}/{maxMovementSteps}");
            }
            return;
        }

        // PHASE 3: PLAYER INPUT
        if (currentPhase == TurnPhase.PlayerInput)
        {
            SetState(EnemyState.Idle);
            return;
        }
    }

    private void Step()
    {
        SetState(EnemyState.Moving);

        TransformComponent myTf = GetTransform();
        myTf.X -= StepDistance;

        float distanceToPlayer = Math.Abs(myTf.X - PlayerX);
        Console.WriteLine($"[Enemy] Stepped to X={myTf.X:F1}, Distance to player: {distanceToPlayer:F1}");
    }

    private void AttackPlayer()
    {
        if (isDead) return;

        Console.WriteLine($"[Enemy] ?? ATTACKING PLAYER for {AttackDamage} damage!");
        SetState(EnemyState.Attacking);

        attackCooldownTimer = AttackFrames / AnimationSpeed;

        // TODO: Apply damage to player
        // if (PlayerHealth.Instance != null)
        // {
        //     PlayerHealth.Instance.TakeDamage((int)AttackDamage);
        // }
    }

    // ========================================================================
    // PUBLIC API - Called by EnemySpawner
    // ========================================================================

    public void ConfigureEnemy(EnemySpawner.EnemyData data)
    {
        Console.WriteLine($"[Enemy] Configuring as {data.IconType}...");

        // Update all properties from data
        WeaknessCombo = data.WeaknessCombo;
        Health = data.Health;
        AttackDamage = data.AttackDamage;
        StepDistance = data.StepDistance;

        IdleSpriteSheet = data.IdleSpriteSheet;
        WalkSpriteSheet = data.WalkSpriteSheet;
        AttackSpriteSheet = data.AttackSpriteSheet;
        HurtSpriteSheet = data.HurtSpriteSheet;

        // Reset state
        isDead = false;
        waitingToDie = false;
        deathTimer = 0f;
        attackCooldownTimer = 0f;
        movementStepsTaken = 0;

        // Reset weakness flag so we update ComboSystem
        weaknessSet = false;
        weaknessRetryCount = 0;

        // Make visible and reset position
        TransformComponent tf = GetTransform();
        tf.IsVisible = true;
        tf.X = 1200f; // Starting X position (right side of screen)
        tf.Y = PlayerY; // Same Y as player

        // Set initial sprite
        SetScaleForSprite(IdleSpriteSheet, 220, 432);
        SetState(EnemyState.Idle);

        // Try to set weakness immediately
        TrySetWeakness();

        Console.WriteLine($"[Enemy] ? Configured! HP={Health}, Combo={WeaknessCombo}");
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
        Console.WriteLine("?? VICTORY! Enemy defeated!");
        Console.WriteLine("==============================================");

        // Notify the spawner that this enemy was defeated
        if (EnemySpawner.Instance != null)
        {
            EnemySpawner.Instance.OnEnemyDefeated();
            Console.WriteLine("[Enemy] ?? Notified spawner - UI should advance to next enemy");
        }
        else
        {
            Console.WriteLine("[Enemy] ?? WARNING: EnemySpawner.Instance is null - UI won't update!");
        }
    }

    // Called by TurnManager when player FAILS combo
    public void ExecuteFailurePunishment()
    {
        Console.WriteLine("[Enemy] ExecuteFailurePunishment() called");

        int remainingSteps = maxMovementSteps - movementStepsTaken;
        Console.WriteLine($"[Enemy] Player failed! Moving {remainingSteps} remaining steps");

        for (int i = 0; i < remainingSteps; i++)
        {
            Step();
        }

        TransformComponent myTf = GetTransform();
        float distanceToPlayer = Math.Abs(myTf.X - PlayerX);

        if (distanceToPlayer <= AttackRange)
        {
            Console.WriteLine($"[Enemy] In range ({distanceToPlayer:F1} <= {AttackRange}) - attacking!");
            AttackPlayer();
        }
        else
        {
            Console.WriteLine($"[Enemy] Not in range yet ({distanceToPlayer:F1} > {AttackRange}) - waiting...");
        }
    }

    // Called by TurnManager at start of EACH enemy display phase
    public void ResetMovementForNewTurn()
    {
        Console.WriteLine("[Enemy] ResetMovementForNewTurn() - resetting step counter");
        movementStepsTaken = 0;

        if (!weaknessSet)
        {
            weaknessRetryCount = 0;
            TrySetWeakness();
        }

        SetState(EnemyState.Idle);
    }
}