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
    public string WeaknessCombo = "ASAS";

    // === REFERENCES ===
    public Conductor ConductorRef = null;
    public ComboSystem ComboRef = null;
    public TurnManager TurnManagerRef = null;

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
    private int remainingPunishmentSteps = 0;

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

        // Check distance to player
        TransformComponent myTf = GetTransform();
        float distanceToPlayer = Math.Abs(myTf.X - PlayerX);

        // If in attack range, attack
        if (distanceToPlayer <= AttackRange && currentState != EnemyState.Attacking)
        {
            AttackSequence();
            return;
        }

        // Movement logic - only during enemy turn
        if (ConductorRef != null && TurnManagerRef != null)
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
        SetScaleForSprite(IdleSpriteSheet, 220, 432);
        SetState(EnemyState.Idle);

        if (ConductorRef != null)
        {
            lastBeat = ConductorRef.GetCurrentBeat();
            Console.WriteLine("[Enemy] Conductor connected");
        }

        if (ComboRef != null)
        {
            ComboRef.SetEnemyWeakness(WeaknessCombo);
            Console.WriteLine($"[Enemy] Weakness: {WeaknessCombo}");
        }

        Console.WriteLine($"[Enemy] Initialized at ({tf.X}, {tf.Y})");
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

        Console.WriteLine($"[Enemy] Scale set for {spriteName}: ({tf.ScaleX:F1}, {tf.ScaleY:F1})");
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
        if (TurnManagerRef == null)
            return;

        TurnPhase currentPhase = TurnManagerRef.GetCurrentPhase();

        // During countdown - stay idle
        if (currentPhase == TurnPhase.Countdown)
        {
            SetState(EnemyState.Idle);
            return;
        }

        // During enemy display - move
        if (currentPhase == TurnPhase.EnemyDisplay)
        {
            if (movementStepsTaken < maxMovementSteps)
            {
                Step();
                movementStepsTaken++;
            }
            return;
        }

        // During player input - freeze
        if (currentPhase == TurnPhase.PlayerInput)
        {
            SetState(EnemyState.Idle);
            return;
        }

        // During resolution - execute punishment steps if needed
        if (currentPhase == TurnPhase.Resolution)
        {
            if (remainingPunishmentSteps > 0)
            {
                Step();
                remainingPunishmentSteps--;
            }
        }
    }

    private void Step()
    {
        SetState(EnemyState.Moving);

        TransformComponent myTf = GetTransform();
        myTf.X -= StepDistance;

        Console.WriteLine($"[Enemy] Stepped to X={myTf.X}, Distance to player: {Math.Abs(myTf.X - PlayerX)}");
    }

    private void AttackSequence()
    {
        if (isDead) return;

        Console.WriteLine("[Enemy] ATTACKING!");
        SetState(EnemyState.Attacking);

        attackCooldownTimer = AttackFrames / AnimationSpeed;

        bool playerSucceeded = false;
        if (ComboRef != null)
        {
            playerSucceeded = ComboRef.PlayerDidSuccessfulCombo;
        }

        if (!playerSucceeded)
        {
            Console.WriteLine($"[Enemy] Player failed! Dealing {AttackDamage} damage");
            // Apply damage to player here
        }
        else
        {
            Console.WriteLine("[Enemy] Player succeeded! Taking damage...");
        }
    }

    public void TakeDamage(float dmg)
    {
        if (isDead) return;
        Health -= dmg;
        Console.WriteLine($"[Enemy] Took {dmg} damage! HP: {Health}");
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

    // Called by TurnManager when player fails
    public void ExecuteFailurePunishment()
    {
        remainingPunishmentSteps = maxMovementSteps - movementStepsTaken;
        Console.WriteLine($"[Enemy] Player failed! Moving {remainingPunishmentSteps} remaining steps and attacking!");

        // Reset for next turn
        movementStepsTaken = 0;
    }

    // Called by TurnManager at start of enemy turn
    public void ResetMovementForNewTurn()
    {
        movementStepsTaken = 0;
        remainingPunishmentSteps = 0;
        SetState(EnemyState.Idle);
    }
}