using System;
using ScriptAPI;


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
    private bool initialized = false;
    private bool isDead = false;

    // =========================
    // Public Entity ID (for collision checks by other scripts)
    // =========================
    public int PlayerEntityID { get; private set; }

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

        if (GetHealth().hp <= 0 && !isDead)
        {
            Die();
        }
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        //if (Instance == null)
        //{
        //    Instance = this;
        //}
        //else if (Instance != this)
        //{
        //    Console.WriteLine("[PlayerHealth] WARNING: Multiple PlayerHealth instances!");
        //    return;
        //}

        // ALWAYS MAKE A NEW ONE IN INIT
        Instance = this;
        isDead = false;
        PlayerEntityID = entityID;
        Console.WriteLine($"[PlayerHealth] Entity ID exposed: {PlayerEntityID}");

        HealthComponent h = GetHealth();
        h.hp = maxHP;
        h.isAlive = true;
        Console.WriteLine($"[PlayerHealth] Initialized with {maxHP} HP");

        initialized = true;
    }

    // =========================
    // Public API
    // =========================

    public void TakeDamage(int damage)
    {
        if (isDead) return;

        HealthComponent h = GetHealth();
        h.hp -= damage;
        if (h.hp < 0) h.hp = 0;

        Console.WriteLine($"[PlayerHealth] Took {damage} damage! HP: {h.hp}/{maxHP}");

        if (h.hp > 0)
        {
            FlashRed();

            // Trigger hurt animation on the player
            if (PlayerAnimationController.Instance != null)
            {
                PlayerAnimationController.Instance.PlayHurt();
            }
        }
    }

    public int GetCurrentHP()
    {
        return GetHealth().hp;
    }

    public int GetMaxHP()
    {
        return maxHP;
    }

    public bool IsAlive()
    {
        return GetHealth().isAlive;
    }

    public float GetHealthPercent()
    {
        return (float)GetHealth().hp / (float)maxHP;
    }

    // =========================
    // Death Handling
    // =========================
    private void Die()
    {
        if (isDead) return;

        isDead = true;
        HealthComponent hDie = GetHealth();
        hDie.isAlive = false;
        Console.WriteLine("[PlayerHealth] PLAYER DIED!");
        Console.WriteLine("==============================================");
        Console.WriteLine("GAME OVER");
        Console.WriteLine("==============================================");

        NavigationButtons.GoToGameLose(); // go to gamelose scene/overlap?? idk why its diff from the win.
        
        Application.SetPaused(true);
    }

    // =========================
    // Visual Feedback
    // =========================
    private void FlashRed()
    {
        if (HealthBGController.Instance != null)
        {
            HealthBGController.Instance.TriggerDamageFlash();
        }
    }

    // =========================
    // Debug
    // =========================
    public void DebugSetHP(int hp)
    {
        HealthComponent h = GetHealth();
        h.hp = hp;
        if (h.hp > maxHP) h.hp = maxHP;
        if (h.hp < 0) h.hp = 0;
        Console.WriteLine($"[PlayerHealth] DEBUG: HP set to {h.hp}");
    }
}