using System;
using ScriptAPI;

/// <summary>
/// Player health management - FIXED VERSION with extensive debugging
/// 
/// SETUP:
/// - Attach this script to your PLAYER entity (the entity at x=-970, y=-236 in Level1.json)
/// - Player entity must have a Health component
/// - For UI, create separate HealthBarUI script on the health bar entity
/// </summary>
public class PlayerHealth : Script
{
    // =========================
    // Singleton Access
    // =========================
    public static PlayerHealth Instance { get; private set; }

    // =========================
    // Config
    // =========================
    public int MaxHP = 100;
    public int StartingHP = 100;

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private bool isDead = false;
    private int updateCount = 0; // Track how many times Update() has been called

    // =========================
    // Update Loop
    // =========================
    public override void Update()
    {
        updateCount++;

        if (!initialized)
        {
            Initialize();
            return;
        }

        // Print status every 60 frames (roughly once per second at 60fps)
        if (updateCount % 60 == 0)
        {
            HealthComponent health = GetHealth();
            Console.WriteLine($"[PlayerHealth] Status update - HP: {health.hp}/{MaxHP} ({GetHPPercent():P0})");
        }

        // Check for death
        HealthComponent health2 = GetHealth();
        if (health2.hp <= 0 && !isDead)
        {
            Die();
        }
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        // Set singleton FIRST
        if (Instance == null)
        {
            Instance = this;
            Console.WriteLine("[PlayerHealth] ? Singleton instance set");
        }
        else
        {
            Console.WriteLine("[PlayerHealth] ? WARNING: Multiple PlayerHealth instances detected!");
            return;
        }

        Console.WriteLine("[PlayerHealth] ???????????????????????????????");
        Console.WriteLine("[PlayerHealth] Initializing Player Health System...");
        Console.WriteLine("[PlayerHealth] ???????????????????????????????");

        // Initialize health component
        HealthComponent health = GetHealth();
        health.hp = StartingHP;
        health.isAlive = true;

        Console.WriteLine($"[PlayerHealth] ? Initial HP set to: {health.hp}/{MaxHP}");

        // Get and log player position for debugging
        TransformComponent tf = GetTransform();
        Console.WriteLine($"[PlayerHealth] ? Player position: ({tf.X}, {tf.Y})");
        Console.WriteLine($"[PlayerHealth] ? Player scale: ({tf.ScaleX}, {tf.ScaleY})");
        Console.WriteLine($"[PlayerHealth] ? Player visible: {tf.IsVisible}");

        Console.WriteLine("[PlayerHealth] ???????????????????????????????");
        Console.WriteLine("[PlayerHealth] Initialization complete!");
        Console.WriteLine("[PlayerHealth] ???????????????????????????????");

        initialized = true;
    }

    // =========================
    // Public API
    // =========================
    public void TakeDamage(int dmg)
    {
        if (isDead)
        {
            Console.WriteLine("[PlayerHealth] ? Cannot take damage - player is dead");
            return;
        }

        HealthComponent health = GetHealth();
        int oldHP = health.hp;
        health.hp = Math.Max(0, health.hp - dmg);

        Console.WriteLine("??????????????????????????????????????");
        Console.WriteLine($"?  ?? PLAYER TOOK {dmg} DAMAGE!        ?");
        Console.WriteLine("??????????????????????????????????????");
        Console.WriteLine($"?  HP: {oldHP} ? {health.hp} / {MaxHP}");
        Console.WriteLine($"?  Remaining: {GetHPPercent():P0}");
        Console.WriteLine("??????????????????????????????????????");

        // Play hurt sound
        try
        {
            GetAudio().PlaySFX("SFX_PlayerHurt");
            Console.WriteLine("[PlayerHealth] ? Played hurt sound");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[PlayerHealth] ? Could not play hurt sound: {ex.Message}");
        }

        // Flash red
        FlashDamage();

        if (health.hp <= 0)
        {
            Die();
        }
    }

    public void Heal(int amount)
    {
        if (isDead)
        {
            Console.WriteLine("[PlayerHealth] ? Cannot heal - player is dead");
            return;
        }

        HealthComponent health = GetHealth();
        int oldHP = health.hp;
        health.hp = Math.Min(MaxHP, health.hp + amount);

        int actualHealing = health.hp - oldHP;
        if (actualHealing > 0)
        {
            Console.WriteLine($"[PlayerHealth] ?? Healed {actualHealing}! HP: {health.hp}/{MaxHP}");

            try
            {
                GetAudio().PlaySFX("SFX_Heal");
            }
            catch { }
        }
    }

    public int GetCurrentHP()
    {
        if (!initialized)
        {
            Console.WriteLine("[PlayerHealth] ? WARNING: GetCurrentHP() called before initialization!");
            return StartingHP;
        }
        return GetHealth().hp;
    }

    public int GetMaxHP()
    {
        return MaxHP;
    }

    public float GetHPPercent()
    {
        if (!initialized)
        {
            Console.WriteLine("[PlayerHealth] ? WARNING: GetHPPercent() called before initialization!");
            return 1.0f;
        }

        int currentHP = GetHealth().hp;
        float percent = (float)currentHP / (float)MaxHP;
        return percent;
    }

    public bool IsDead()
    {
        return isDead;
    }

    // =========================
    // Death Handling
    // =========================
    private void Die()
    {
        if (isDead) return;

        isDead = true;
        HealthComponent health = GetHealth();
        health.isAlive = false;
        health.hp = 0;

        Console.WriteLine("\n\n");
        Console.WriteLine("??????????????????????????????????????");
        Console.WriteLine("?                                    ?");
        Console.WriteLine("?        ?? PLAYER DEFEATED! ??      ?");
        Console.WriteLine("?                                    ?");
        Console.WriteLine("?          GAME OVER                 ?");
        Console.WriteLine("?                                    ?");
        Console.WriteLine("??????????????????????????????????????");
        Console.WriteLine("\n\n");

        // Play death sound
        try
        {
            GetAudio().PlaySFX("SFX_PlayerDeath");
        }
        catch { }

        // Make player invisible
        TransformComponent tf = GetTransform();
        tf.IsVisible = false;

        // Notify TurnManager if it exists
        if (TurnManager.Instance != null)
        {
            // TurnManager should handle game over logic
            Console.WriteLine("[PlayerHealth] Notifying TurnManager of player death");
        }
    }

    // =========================
    // Visual Feedback
    // =========================
    private void FlashDamage()
    {
        try
        {
            // Flash player sprite red briefly
            SpriteComponent sprite = GetSprite();
            if (sprite.HasSprite())
            {
                // Set red tint
                sprite.SetTint(1.0f, 0.3f, 0.3f, 1.0f);
                Console.WriteLine("[PlayerHealth] ? Applied red damage tint");

                // TODO: Reset tint after 0.2 seconds
                // For now, it stays red (you'd need a timer system)
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[PlayerHealth] ? Could not apply damage tint: {ex.Message}");
        }
    }
}