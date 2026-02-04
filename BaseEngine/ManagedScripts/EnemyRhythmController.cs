using System;
using ScriptAPI;

public enum EnemyState { Idle, Moving, Attacking, Hurt, Dead }
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
    public float AttackRange = 500f;  // Increased for better detection
    public float AttackDamage = 12f;
    public float MaxHealth = 30f;

    // === WEAKNESS ===
    public string WeaknessCombo = "WASD";

    // === PLAYER POSITION OVERRIDE ===
    // FIX: Since GetPlayerPosition() returns wrong values, we'll use the actual position from Level1.json
    // Player is at x=-970 in the scene file
    private float PLAYER_X_POSITION = -970f;

    // === INTERNAL STATE ===
    private bool initialized = false;
    private int lastBeat = -1;
    private bool isDead = false;
    private EnemyState currentState = EnemyState.Idle;
    private float currentHealth;

    private float attackCooldownTimer = 0f;
    private float hurtAnimationTimer = 0f;
    private float deathAnimationTimer = 0f;
    private const float hurtAnimationDuration = 0.5f;
    private const float deathAnimationDuration = 2.0f;

    // Track movement during enemy turn
    private int movementStepsTaken = 0;
    private int maxMovementSteps = 4;

    private bool weaknessSet = false;
    private int weaknessRetryCount = 0;
    private const int maxWeaknessRetries = 100;

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            initialized = true;
            return;
        }

        // Keep trying to set weakness until ComboSystem is ready
        if (!weaknessSet && weaknessRetryCount < maxWeaknessRetries)
        {
            TrySetWeakness();
            weaknessRetryCount++;
        }

        // Handle timers
        float dt = Time.DeltaTime;

        // Hurt animation timer
        if (hurtAnimationTimer > 0f)
        {
            hurtAnimationTimer -= dt;
            if (hurtAnimationTimer <= 0f)
            {
                // Return to idle after hurt animation
                SetState(EnemyState.Idle);
            }
        }

        // Death animation timer
        if (isDead)
        {
            deathAnimationTimer -= dt;
            if (deathAnimationTimer <= 0f)
            {
                // Hide entity after death animation completes
                TransformComponent tf = GetTransform();
                tf.IsVisible = false;
                Console.WriteLine("[Enemy] Fading out...");
            }
            return; // Don't process anything else if dead
        }

        // Attack cooldown timer
        if (attackCooldownTimer > 0f)
        {
            attackCooldownTimer -= dt;
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

        // Initialize health
        currentHealth = MaxHealth;

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

        Console.WriteLine($"[Enemy] Initialized at ({tf.X}, {tf.Y}) | Health: {currentHealth}/{MaxHealth}");
        Console.WriteLine($"[Enemy] Player position (hardcoded): {PLAYER_X_POSITION}");

        // Calculate initial distance
        float distanceToPlayer = Math.Abs(tf.X - PLAYER_X_POSITION);
        Console.WriteLine($"[Enemy] Initial distance to player: {distanceToPlayer:F1}");
    }

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
            Console.WriteLine("[Enemy] ? ComboSystem.Instance is null - will retry...");
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

            case EnemyState.Hurt:
                SetScaleForSprite(HurtSpriteSheet, 515, 428);
                anim.SetAnimation(HurtSpriteSheet, HurtRows, HurtColumns, HurtFrames, AnimationSpeed);
                break;

            case EnemyState.Dead:
                SetScaleForSprite(HurtSpriteSheet, 515, 428);
                anim.SetAnimation(HurtSpriteSheet, HurtRows, HurtColumns, HurtFrames, AnimationSpeed / 2f);
                break;
        }
    }

    private void HandleBeat(int beat)
    {
        if (isDead) return;

        if (TurnManager.Instance == null)
            return;

        TurnPhase currentPhase = TurnManager.Instance.GetCurrentPhase();

        // PHASE 1: COUNTDOWN
        if (currentPhase == TurnPhase.Countdown)
        {
            SetState(EnemyState.Idle);
            return;
        }

        // PHASE 2: ENEMY DISPLAY - Move one step per beat
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

        // PHASE 3: PLAYER INPUT - Freeze in idle
        if (currentPhase == TurnPhase.PlayerInput)
        {
            // Only set to idle if not playing hurt animation
            if (currentState != EnemyState.Hurt)
                SetState(EnemyState.Idle);
            return;
        }
    }

    private void Step()
    {
        SetState(EnemyState.Moving);

        TransformComponent myTf = GetTransform();
        myTf.X -= StepDistance;

        // Use hardcoded player position
        float distanceToPlayer = Math.Abs(myTf.X - PLAYER_X_POSITION);

        Console.WriteLine($"[Enemy] Stepped to X={myTf.X:F1} | Player at X={PLAYER_X_POSITION:F1} | Distance: {distanceToPlayer:F1}");
    }

    private void AttackPlayer()
    {
        if (isDead) return;

        Console.WriteLine($"[Enemy] ?? ATTACKING PLAYER for {AttackDamage} damage!");
        SetState(EnemyState.Attacking);

        attackCooldownTimer = AttackFrames / AnimationSpeed;

        // Apply damage to player
        if (PlayerHealth.Instance != null)
        {
            PlayerHealth.Instance.TakeDamage((int)AttackDamage);
            Console.WriteLine("[Enemy] ? Damage applied to player!");
        }
        else
        {
            Console.WriteLine("[Enemy] ? ERROR: PlayerHealth.Instance is null - cannot damage player!");
        }
    }

    // ========================================================================
    // PUBLIC API - Called by TurnManager
    // ========================================================================

    public void TakeDamage(float dmg)
    {
        if (isDead) return;

        currentHealth -= dmg;
        Console.WriteLine($"[Enemy] ?? Took {dmg} damage! HP: {currentHealth}/{MaxHealth}");

        // Play hurt animation
        SetState(EnemyState.Hurt);
        hurtAnimationTimer = hurtAnimationDuration;

        // Check if dead
        if (currentHealth <= 0)
        {
            Die();
        }
    }

    private void Die()
    {
        if (isDead) return;

        isDead = true;
        deathAnimationTimer = deathAnimationDuration;
        SetState(EnemyState.Dead);

        Console.WriteLine("??????????????????????????????????????");
        Console.WriteLine("?     ?? ENEMY DEFEATED!         ?");
        Console.WriteLine("??????????????????????????????????????");
        Console.WriteLine("?        ?? VICTORY! ??          ?");
        Console.WriteLine("??????????????????????????????????????");

        // Notify EnemySpawner if it exists
        if (EnemySpawner.Instance != null)
        {
            EnemySpawner.Instance.OnEnemyDefeated();
        }
    }

    // Called by TurnManager when player FAILS combo
    public void ExecuteFailurePunishment()
    {
        if (isDead) return;

        Console.WriteLine("[Enemy] ExecuteFailurePunishment() called");

        // Calculate remaining steps
        int remainingSteps = maxMovementSteps - movementStepsTaken;
        Console.WriteLine($"[Enemy] Player failed! Moving {remainingSteps} remaining steps");

        // Move those remaining steps
        for (int i = 0; i < remainingSteps; i++)
        {
            Step();
        }

        // NOW check if we're in range AFTER all movement is done
        TransformComponent myTf = GetTransform();
        float distanceToPlayer = Math.Abs(myTf.X - PLAYER_X_POSITION);

        if (distanceToPlayer <= AttackRange)
        {
            Console.WriteLine($"[Enemy] ? In range ({distanceToPlayer:F1} <= {AttackRange}) - ATTACKING!");
            AttackPlayer();
        }
        else
        {
            Console.WriteLine($"[Enemy] ?? Not in range yet ({distanceToPlayer:F1} > {AttackRange})");
            SetState(EnemyState.Idle);
        }
    }

    // Called by TurnManager at start of EACH enemy display phase
    public void ResetMovementForNewTurn()
    {
        if (isDead)
        {
            Console.WriteLine("[Enemy] Already dead, not resetting");
            return;
        }

        Console.WriteLine("[Enemy] ResetMovementForNewTurn() - resetting step counter");
        movementStepsTaken = 0;

        // Reset weakness flag for new game sessions
        if (!weaknessSet)
        {
            weaknessRetryCount = 0;
            TrySetWeakness();
        }

        // Only set to idle if not playing hurt animation
        if (currentState != EnemyState.Hurt)
            SetState(EnemyState.Idle);
    }

    // ========================================================================
    // Public Getters
    // ========================================================================
    public float GetCurrentHealth()
    {
        return currentHealth;
    }

    public float GetHealthPercent()
    {
        return currentHealth / MaxHealth;
    }

    public bool IsDead()
    {
        return isDead;
    }
}