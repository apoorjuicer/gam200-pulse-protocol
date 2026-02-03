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
    public float BaseWidth = 200f;   // Target visual width
    public float BaseHeight = 200f;  // Target visual height

    // === RHYTHM SETTINGS ===
    public RhythmType RhythmType = RhythmType.Syncopated;
    public float StepDistance = 50f;  // 50 pixels per beat = 8 beats to reach player

    // === COMBAT SETTINGS ===
    public float AttackRange = 25f;   // Attack when within 50 pixels (very close)
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

    // === INTERNAL STATE ===
    private bool initialized = false;
    private bool gameStarted = false;
    private int beatCounter = 0;
    private int lastBeat = -1;
    private bool canMove = true;
    private bool recentlyAttacked = false;
    private bool isDead = false;
    private EnemyState currentState = EnemyState.Idle;

    private float attackCooldownTimer = 0f;
    private float deathTimer = 0f;
    private bool waitingToDie = false;

    // For auto-movement testing
    private float moveTimer = 0f;
    private float moveInterval = 0.5f;

    // For time-based fallback
    private float elapsedTime = 0f;

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            initialized = true;
            return;
        }

        elapsedTime += Time.DeltaTime;

        if (!gameStarted)
        {
            if (CheckGameStarted())
            {
                gameStarted = true;
                Console.WriteLine("[Enemy] Game started! Beginning movement.");
            }
            else
            {
                return;
            }
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

        // Check distance to player
        TransformComponent myTf = GetTransform();
        float distanceToPlayer = Math.Abs(myTf.X - PlayerX);

        // If in attack range, attack
        if (distanceToPlayer <= AttackRange && !recentlyAttacked && canMove)
        {
            canMove = false;
            AttackSequence();
            return;
        }

        // Movement logic
        if (ConductorRef != null)
        {
            int currentBeat = ConductorRef.GetCurrentBeat();
            if (currentBeat != lastBeat)
            {
                lastBeat = currentBeat;
                HandleBeat(currentBeat);
            }
        }
        else
        {
            moveTimer += Time.DeltaTime;
            if (moveTimer >= moveInterval && canMove)
            {
                moveTimer = 0f;
                Step();
            }
        }
    }

    private void Initialize()
    {
        Console.WriteLine("[Enemy] Initializing...");

        TransformComponent tf = GetTransform();

        // Set initial scale for idle sprite
        SetScaleForSprite(IdleSpriteSheet, 220, 432);

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
            Console.WriteLine($"[Enemy] Weakness: {WeaknessCombo}");
        }

        Console.WriteLine($"[Enemy] Initialized at ({tf.X}, {tf.Y})");
        Console.WriteLine("[Enemy] Waiting for countdown to finish...");
    }

    private void SetScaleForSprite(string spriteName, float spriteWidth, float spriteHeight)
    {
        TransformComponent tf = GetTransform();

        // Target bounding box - all rats should fit in this area
        float targetSize = 200f;  // Adjust this to control overall size

        // Calculate scale factor to fit within the box
        float scaleX = targetSize / spriteWidth;
        float scaleY = targetSize / spriteHeight;

        // Use the SMALLER scale to ensure sprite fits entirely
        float uniformScale = Math.Min(scaleX, scaleY);

        // Apply uniform scaling (maintains aspect ratio)
        tf.ScaleX = spriteWidth * uniformScale;
        tf.ScaleY = spriteHeight * uniformScale;

        Console.WriteLine($"[Enemy] Scale set for {spriteName}: ({tf.ScaleX:F1}, {tf.ScaleY:F1})");
    }

    private bool CheckGameStarted()
    {
        if (ConductorRef != null)
        {
            int currentBeat = ConductorRef.GetCurrentBeat();
            return currentBeat >= 3;  // Wait until Beat 3
        }
        return elapsedTime >= 3.0f;
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
        if (!canMove) return;

        SetState(EnemyState.Moving);

        TransformComponent myTf = GetTransform();
        myTf.X -= StepDistance;

        Console.WriteLine($"[Enemy] Stepped to X={myTf.X}, Distance to player: {Math.Abs(myTf.X - PlayerX)}");
    }

    private void AttackSequence()
    {
        if (recentlyAttacked || isDead) return;

        Console.WriteLine("[Enemy] ATTACKING!");
        SetState(EnemyState.Attacking);
        recentlyAttacked = true;

        attackCooldownTimer = AttackFrames / AnimationSpeed;

        bool playerSucceeded = false;
        if (ComboRef != null)
        {
            playerSucceeded = ComboRef.PlayerDidSuccessfulCombo;
            ComboRef.ResetCombo();
        }

        if (!playerSucceeded)
        {
            Console.WriteLine($"[Enemy] Player failed! Dealing {AttackDamage} damage");
        }
        else
        {
            Console.WriteLine("[Enemy] Player succeeded! Dying...");
            Die();
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
}