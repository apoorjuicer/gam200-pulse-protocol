using System;
using ScriptAPI;

/// <summary>
/// Manages player health and death
/// IMPORTANT: Attach this to your Player entity
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
    public int maxHP = 100;

    // =========================
    // State
    // =========================
    private int currentHP;
    private bool initialized = false;
    private bool isDead = false;

    // =========================
    // Update Loop
    // =========================
    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            return;
        }

        // Check for death
        if (currentHP <= 0 && !isDead)
        {
            Die();
        }
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        // Set singleton
        if (Instance == null)
        {
            Instance = this;
        }
        else if (Instance != this)
        {
            Console.WriteLine("[PlayerHealth] WARNING: Multiple PlayerHealth instances!");
            return;
        }

        // Initialize health
        currentHP = maxHP;

        Console.WriteLine($"[PlayerHealth] Initialized with {maxHP} HP");

        // Register with HealthBarController if it exists
        if (HealthBarController.Instance != null)
        {
            HealthBarController.Instance.SetPlayerHealthReference(this);
            Console.WriteLine("[PlayerHealth] Registered with HealthBarController");
        }

        initialized = true;
    }

    // =========================
    // Public API
    // =========================

    /// <summary>
    /// Apply damage to the player
    /// </summary>
    public void TakeDamage(int damage)
    {
        if (isDead) return;

        currentHP -= damage;
        if (currentHP < 0) currentHP = 0;

        Console.WriteLine($"[PlayerHealth] Took {damage} damage! HP: {currentHP}/{maxHP}");

        // Trigger visual feedback
        if (currentHP > 0)
        {
            // TODO: Play hurt animation/sound
            FlashRed();
        }
    }

    /// <summary>
    /// Heal the player
    /// </summary>
    public void Heal(int amount)
    {
        if (isDead) return;

        currentHP += amount;
        if (currentHP > maxHP) currentHP = maxHP;

        Console.WriteLine($"[PlayerHealth] Healed {amount}! HP: {currentHP}/{maxHP}");
    }

    /// <summary>
    /// Get current HP
    /// </summary>
    public int GetCurrentHP()
    {
        return currentHP;
    }

    /// <summary>
    /// Get max HP
    /// </summary>
    public int GetMaxHP()
    {
        return maxHP;
    }

    /// <summary>
    /// Check if player is alive
    /// </summary>
    public bool IsAlive()
    {
        return !isDead;
    }

    /// <summary>
    /// Get health as percentage (0.0 to 1.0)
    /// </summary>
    public float GetHealthPercent()
    {
        return (float)currentHP / (float)maxHP;
    }

    // =========================
    // Death Handling
    // =========================
    private void Die()
    {
        if (isDead) return;

        isDead = true;
        Console.WriteLine("[PlayerHealth] ☠️ PLAYER DIED!");
        Console.WriteLine("==============================================");
        Console.WriteLine("💀 GAME OVER");
        Console.WriteLine("==============================================");

        // TODO: Trigger game over screen
        // Scene.LoadScene("GameOver");

        // For now, just pause
        Application.SetPaused(true);
    }

    // =========================
    // Visual Feedback
    // =========================
    private void FlashRed()
    {
        // Flash the player sprite red briefly
        var sprite = GetSprite();
        if (sprite.HasSprite())
        {
            // TODO: Implement tween/coroutine to flash red then fade back
            // For now, just log
            Console.WriteLine("[PlayerHealth] 🩸 Player hurt - flashing red");
        }
    }

    // =========================
    // Debug
    // =========================
    public void DebugSetHP(int hp)
    {
        currentHP = hp;
        if (currentHP > maxHP) currentHP = maxHP;
        if (currentHP < 0) currentHP = 0;
        Console.WriteLine($"[PlayerHealth] DEBUG: HP set to {currentHP}");
    }
}