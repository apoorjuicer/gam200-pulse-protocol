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
    public float SizeMultiplier = 2.0f;

    // === RHYTHM SETTINGS ===
    public RhythmType RhythmType = RhythmType.Syncopated;
    public float StepDistance = 50f;

    // === COMBAT SETTINGS ===
    public int AttackDamage = 5;
    public float Health = 15f;

    // === COLLISION SETTINGS ===
    // "AABB"     = box collision (uses AABB components, may be affected by BroadPhase filtering)
    // "Edge"     = edge contact with threshold (uses AABB components)
    // "Distance" = radius-based via engine (uses transforms only, bypasses BroadPhase)
    // "Manual"   = uses engine CheckDistance with ManualAttackRange as radius (simplest, most reliable)
    public string CollisionMode = "Manual";

    // For Distance mode
    public float SelfCollisionRadius = 25f;
    public float PlayerCollisionRadius = 25f;

    // For Edge mode
    public float EdgeContactThreshold = 5f;

    // For Manual mode - radius in world units for CheckDistance
    // This determines how far from the player's center the enemy stops.
    // Player scale is ~300 wide, enemy is ~110 wide.
    // ~350 means the enemy stops when its center is 350 units from the player center,
    // which visually places it right in front of the player sprite.
    // Increase if enemy still overlaps, decrease if it stops too far away.
    public float ManualAttackRange = 350f;

    // === PLAYER REFERENCE ===
    private int playerEntityID = -1;
    private bool playerFound = false;

    // === WEAKNESS ===
    public string WeaknessCombo = "WAWA";

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

    // Once the enemy reaches the player, it STOPS moving entirely
    private bool hasReachedPlayer = false;

    // Track if we've tried to set weakness
    private bool weaknessSet = false;
    private int weaknessRetryCount = 0;
    private const int maxWeaknessRetries = 100;

    // Debug
    private int collisionCheckCount = 0;

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            initialized = true;
            return;
        }

        if (!playerFound)
        {
            TryFindPlayer();
        }

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
        if (Instance == null)
            Instance = this;

        Console.WriteLine("[Enemy] Initializing...");

        TransformComponent tf = GetTransform();

        if (SizeMultiplier != 1.0f && (tf.ScaleX == 1.0f || tf.ScaleY == 1.0f))
        {
            tf.ScaleX = 220f * SizeMultiplier;
            tf.ScaleY = 432f * SizeMultiplier;
            Console.WriteLine($"[Enemy] Applied size multiplier {SizeMultiplier}x -> Scale: ({tf.ScaleX}, {tf.ScaleY})");
        }
        else
        {
            Console.WriteLine($"[Enemy] Using scene-configured scale: ({tf.ScaleX}, {tf.ScaleY})");
        }

        SetState(EnemyState.Idle);

        if (Conductor.Instance != null)
        {
            lastBeat = Conductor.Instance.GetCurrentBeat();
            Console.WriteLine("[Enemy] Conductor connected");
        }

        TryFindPlayer();
        TrySetWeakness();

        Console.WriteLine($"[Enemy] Initialized at ({tf.X:F1}, {tf.Y:F1}) | Health: {Health}");
        Console.WriteLine($"[Enemy] Collision mode: {CollisionMode} | ManualAttackRange: {ManualAttackRange}");
        Console.WriteLine($"[Enemy] AttackDamage: {AttackDamage}");
        Console.WriteLine($"[Enemy] Own entity ID: {entityID}");
    }

    // =========================
    // Player Discovery
    // =========================
    private void TryFindPlayer()
    {
        if (playerFound) return;

        if (PlayerHealth.Instance != null)
        {
            playerEntityID = PlayerHealth.Instance.PlayerEntityID;

            if (playerEntityID >= 0)
            {
                playerFound = true;
                Console.WriteLine($"[Enemy] Found player entity ID: {playerEntityID}");
                RunCollisionDiagnostic();
            }
            else
            {
                Console.WriteLine("[Enemy] PlayerHealth exists but entity ID not set yet");
            }
        }
    }

    // =========================
    // Diagnostic: test collision modes once at startup
    // =========================
    private void RunCollisionDiagnostic()
    {
        Console.WriteLine("[Enemy] === COLLISION DIAGNOSTIC ===");
        TransformComponent myTf = GetTransform();
        Console.WriteLine($"[Enemy] Enemy pos: ({myTf.X:F1}, {myTf.Y:F1}), scale: ({myTf.ScaleX:F1}, {myTf.ScaleY:F1})");
        Console.WriteLine($"[Enemy] Player entity ID: {playerEntityID}");

        CollisionComponent col = GetCollision();

        bool bigRadius = col.CheckDistance(50000f, playerEntityID, 50000f);
        Console.WriteLine($"[Enemy] Test CheckDistance(50000, 50000): {bigRadius}");

        bool manualTest = col.CheckDistance(ManualAttackRange, playerEntityID, 0f);
        Console.WriteLine($"[Enemy] Test CheckDistance({ManualAttackRange}, 0): {manualTest}");

        bool aabbTest = col.CheckAABB(playerEntityID);
        Console.WriteLine($"[Enemy] Test CheckAABB: {aabbTest}");

        Console.WriteLine("[Enemy] === END DIAGNOSTIC ===");
    }

    // =========================
    // Collision Detection
    // =========================
    private bool IsCollidingWithPlayer()
    {
        if (!playerFound || playerEntityID < 0)
            return false;

        collisionCheckCount++;
        CollisionComponent col = GetCollision();

        switch (CollisionMode)
        {
            case "AABB":
                {
                    bool result = col.CheckAABB(playerEntityID);
                    if (collisionCheckCount % 4 == 1)
                        Console.WriteLine($"[Enemy] AABB check #{collisionCheckCount}: {result}");
                    return result;
                }

            case "Edge":
                {
                    bool result = col.CheckEdgeContact(playerEntityID, EdgeContactThreshold);
                    if (result)
                        Console.WriteLine("[Enemy] Edge contact detected!");
                    return result;
                }

            case "Distance":
                {
                    bool result = col.CheckDistance(SelfCollisionRadius, playerEntityID, PlayerCollisionRadius);
                    if (collisionCheckCount % 4 == 1)
                        Console.WriteLine($"[Enemy] Distance check #{collisionCheckCount}: {result} (radii: {SelfCollisionRadius}+{PlayerCollisionRadius})");
                    return result;
                }

            case "Manual":
                {
                    TransformComponent myTf = GetTransform();
                    bool result = col.CheckDistance(ManualAttackRange, playerEntityID, 0f);

                    if (collisionCheckCount % 4 == 1)
                        Console.WriteLine($"[Enemy] Manual check #{collisionCheckCount} at X={myTf.X:F1}: {result} (range: {ManualAttackRange})");
                    if (result)
                        Console.WriteLine($"[Enemy] IN RANGE at X={myTf.X:F1}!");
                    return result;
                }

            default:
                Console.WriteLine($"[Enemy] Unknown collision mode: {CollisionMode}");
                return false;
        }
    }

    private bool IsPlayerInAttackRange()
    {
        if (playerFound && playerEntityID >= 0)
            return IsCollidingWithPlayer();

        Console.WriteLine("[Enemy] Cannot check attack range - player entity not found!");
        return false;
    }

    private void TrySetWeakness()
    {
        if (weaknessSet) return;

        if (ComboSystem.Instance != null)
        {
            ComboSystem.Instance.SetEnemyWeakness(WeaknessCombo);
            weaknessSet = true;
            Console.WriteLine($"[Enemy] Weakness successfully set to: {WeaknessCombo}");
        }
        else if (weaknessRetryCount == 0)
        {
            Console.WriteLine("[Enemy] ComboSystem.Instance is null - will retry...");
        }
        else if (weaknessRetryCount == maxWeaknessRetries - 1)
        {
            Console.WriteLine("[Enemy] ERROR: ComboSystem never initialized!");
        }
    }

    private void SetState(EnemyState newState)
    {
        if (currentState == newState) return;
        Console.WriteLine($"[Enemy] State: {currentState} -> {newState}");
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
        if (TurnManager.Instance == null) return;

        TurnPhase currentPhase = TurnManager.Instance.GetCurrentPhase();

        if (currentPhase == TurnPhase.Countdown)
        {
            SetState(EnemyState.Idle);
            return;
        }

        if (currentPhase == TurnPhase.EnemyDisplay)
        {
            // If enemy has already reached the player, do NOT move anymore
            if (hasReachedPlayer)
            {
                Console.WriteLine("[Enemy] Already at player - skipping movement");
                return;
            }

            if (movementStepsTaken < maxMovementSteps)
            {
                // Check collision BEFORE stepping - if already in range, don't step further
                if (IsPlayerInAttackRange())
                {
                    Console.WriteLine("[Enemy] Already in range before step - stopping and attacking!");
                    hasReachedPlayer = true;
                    AttackPlayer();
                    return;
                }

                Step();
                movementStepsTaken++;
                Console.WriteLine($"[Enemy] Enemy Display - moved step {movementStepsTaken}/{maxMovementSteps}");

                // Check collision AFTER stepping
                if (IsPlayerInAttackRange())
                {
                    Console.WriteLine("[Enemy] Reached player after step - stopping and attacking!");
                    hasReachedPlayer = true;
                    AttackPlayer();
                    return;
                }
            }
            return;
        }

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
        Console.WriteLine($"[Enemy] Stepped to X={myTf.X:F1}");
    }

    private void AttackPlayer()
    {
        if (isDead) return;

        Console.WriteLine($"[Enemy] ATTACKING PLAYER for {AttackDamage} damage!");
        SetState(EnemyState.Attacking);

        attackCooldownTimer = AttackFrames / AnimationSpeed;

        if (PlayerHealth.Instance != null)
        {
            PlayerHealth.Instance.TakeDamage(AttackDamage);
            Console.WriteLine("[Enemy] Successfully damaged player");
        }
        else
        {
            Console.WriteLine("[Enemy] WARNING: PlayerHealth.Instance is null!");
        }
    }

    public void TakeDamage(float dmg)
    {
        if (isDead) return;

        Health -= dmg;
        Console.WriteLine($"[Enemy] Took {dmg} damage! HP: {Health}/{Health + dmg}");

        if (Health <= 0)
            Die();
    }

    private void Die()
    {
        if (isDead) return;

        isDead = true;
        waitingToDie = true;
        deathTimer = 0f;
        SetState(EnemyState.Dead);

        Console.WriteLine("[Enemy] DEFEATED!");
        Console.WriteLine("==============================================");
        Console.WriteLine("VICTORY! Enemy defeated!");
        Console.WriteLine("==============================================");
    }

    public void ExecuteFailurePunishment()
    {
        Console.WriteLine("[Enemy] ExecuteFailurePunishment() called");

        // If already at the player, just attack (don't move further)
        if (hasReachedPlayer)
        {
            Console.WriteLine("[Enemy] Already at player - attacking immediately!");
            AttackPlayer();
            return;
        }

        int remainingSteps = maxMovementSteps - movementStepsTaken;
        Console.WriteLine($"[Enemy] Player failed! Moving {remainingSteps} remaining steps");

        for (int i = 0; i < remainingSteps; i++)
        {
            // Check BEFORE stepping
            if (IsPlayerInAttackRange())
            {
                Console.WriteLine($"[Enemy] In range before punishment step {i + 1} - stopping!");
                hasReachedPlayer = true;
                AttackPlayer();
                return;
            }

            Step();

            // Check AFTER stepping
            if (IsPlayerInAttackRange())
            {
                Console.WriteLine($"[Enemy] Reached player during punishment step {i + 1}!");
                hasReachedPlayer = true;
                AttackPlayer();
                return;
            }
        }

        // Final check after all steps
        if (IsPlayerInAttackRange())
        {
            Console.WriteLine("[Enemy] In range after all punishment steps - attacking!");
            hasReachedPlayer = true;
            AttackPlayer();
        }
        else
        {
            Console.WriteLine("[Enemy] Not in range yet after punishment steps - waiting...");
        }
    }

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