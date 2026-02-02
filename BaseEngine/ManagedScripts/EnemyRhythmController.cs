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

    // === SPRITE SHEET FRAME DATA ===
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
    public float StepDistance = 0.8f;

    // === COMBAT SETTINGS ===
    public float AttackRange = 2.5f;
    public float AttackDamage = 12f;
    public float Health = 15f;

    // === POSITIONING ===
    public float StopOffset = 1.0f;
    public float AttackTriggerRange = 1.5f;

    // === WEAKNESS ===
    public string WeaknessCombo = "ASAS";

    // === INTERNAL STATE ===
    private bool initialized = false;
    private int beatCounter = 0;
    private int lastBeat = -1;
    private bool canMove = true;
    private bool recentlyAttacked = false;
    private bool isDead = false;
    private EnemyState currentState = EnemyState.Idle;

    // Timers (manual implementation since no Timer API)
    private float attackCooldownTimer = 0f;
    private float deathTimer = 0f;
    private bool waitingToDie = false;

    // === UPDATE ===
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
            // Handle death timer manually
            if (waitingToDie)
            {
                deathTimer += Time.DeltaTime;
                float deathDuration = HurtFrames / (AnimationSpeed / 2f);

                if (deathTimer >= deathDuration)
                {
                    Console.WriteLine($"[Enemy] Destroying entity");
                    // Get entity ID through a component method
                    TransformComponent tf = GetTransform();
                    // Unfortunately we can't access the entityID field from TransformComponent either
                    // So we'll just have to not destroy for now, or ask teammate to add GetEntityID() method
                    // Factory.Destroy(???);

                    // WORKAROUND: Just hide the entity
                    tf.IsVisible = false;
                    Console.WriteLine("[Enemy] Hidden (destruction not available - need GetEntityID() API)");
                }
            }
            return;
        }

        // Handle attack cooldown timer
        if (attackCooldownTimer > 0f)
        {
            attackCooldownTimer -= Time.DeltaTime;

            if (attackCooldownTimer <= 0f)
            {
                recentlyAttacked = false;
                canMove = true;
                if (currentState == EnemyState.Attacking)
                {
                    SetState(EnemyState.Idle);
                }
            }
        }

        // Get conductor for beat tracking
        Conductor conductor = GetConductor();
        if (conductor != null)
        {
            int currentBeat = conductor.GetCurrentBeat();

            if (currentBeat != lastBeat)
            {
                lastBeat = currentBeat;
                HandleBeat(currentBeat);
            }
        }
    }

    private void Initialize()
    {
        // Get entity ID from component (since EntityID property might not be accessible in all contexts)
        TransformComponent tf = GetTransform();

        // Note: We can't directly access entityID from TransformComponent either
        // So we'll just not store it - use GetTransform().entityID when needed

        tf.ScaleX = SpriteScaleX;
        tf.ScaleY = SpriteScaleY;

        SetState(EnemyState.Idle);

        Conductor conductor = GetConductor();
        if (conductor != null)
        {
            beatCounter = conductor.GetCurrentBeat();
            lastBeat = beatCounter;
        }

        // Set weakness on ComboSystem
        ComboSystem combo = GetComboSystem();
        if (combo != null)
        {
            combo.SetEnemyWeakness(WeaknessCombo);
        }

        Console.WriteLine($"[Enemy] Initialized! Rhythm: {RhythmType}, Weakness: {WeaknessCombo}");
    }

    // === STATE MANAGEMENT ===
    private void SetState(EnemyState newState)
    {
        if (currentState == newState) return;

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

    // === BEAT HANDLING ===
    private void HandleBeat(int beat)
    {
        beatCounter++;

        // Determine if enemy should move this beat
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
        {
            Step();
        }
    }

    // === MOVEMENT ===
    private void Step()
    {
        SetState(EnemyState.Moving);

        TransformComponent myTf = GetTransform();

        // Move left (toward player assumed position)
        float newX = myTf.X - StepDistance;
        myTf.X = newX;

        // Check if close enough to attack (placeholder logic)
        if (myTf.X <= AttackTriggerRange && !recentlyAttacked)
        {
            canMove = false;
            AttackSequence();
        }
    }

    // === ATTACK ===
    private void AttackSequence()
    {
        if (recentlyAttacked || isDead) return;

        SetState(EnemyState.Attacking);
        recentlyAttacked = true;

        float attackDuration = (AttackFrames / AnimationSpeed);
        attackCooldownTimer = attackDuration;

        // Get combo result
        ComboSystem combo = GetComboSystem();
        bool playerSucceeded = false;

        if (combo != null)
        {
            playerSucceeded = combo.PlayerDidSuccessfulCombo;
            combo.ResetCombo();
        }

        if (!playerSucceeded)
        {
            // Player failed - deal damage
            Console.WriteLine($"[Enemy] Player failed combo! Dealing {AttackDamage} damage");
            // TODO: Deal damage to player
        }
        else
        {
            // Player succeeded - enemy dies
            Console.WriteLine($"[Enemy] Player succeeded combo! Dying...");

            // Set a short timer before death
            attackCooldownTimer = attackDuration * 0.3f;

            // Die when timer expires (handled in Update)
            float originalTimer = attackCooldownTimer;
            attackCooldownTimer = 0f; // Override to die immediately after animation
            Die();
        }
    }

    // === DAMAGE ===
    public void TakeDamage(float dmg)
    {
        if (isDead) return;

        Health -= dmg;
        Console.WriteLine($"[Enemy] Took {dmg} damage! Health: {Health}");

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

        Console.WriteLine($"[Enemy] Defeated!");

        // Note: We'll destroy in Update loop when timer expires
    }

    // === HELPERS ===
    private Conductor GetConductor()
    {
        // TODO: Replace with actual conductor reference
        // You need to find the conductor entity somehow
        return GetScriptLike<Conductor>();
    }

    private ComboSystem GetComboSystem()
    {
        // TODO: Replace with actual combo system reference
        return GetScriptLike<ComboSystem>();
    }

    // Placeholder until engine supports script lookup
    private T GetScriptLike<T>() where T : Script
    {
        return default;
    }

    // === PUBLIC API ===
    public void StartMoving()
    {
        if (!isDead) SetState(EnemyState.Moving);
    }

    public void StartAttacking()
    {
        if (!isDead) SetState(EnemyState.Attacking);
    }
}