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

    // Optional: set this to whatever your lose scene is called
    public string loseSceneName = "GameLose";

    // =========================
    // Internal
    // =========================
    private bool initialized = false;

    private void Awake()
    {
        // Set singleton
        if (Instance == null)
            Instance = this;

        Console.WriteLine("[PlayerHealth] Singleton initialized");
    }

    public override void Update()
    {
        EnsureInit();
    }

    private void EnsureInit()
    {
        if (initialized) return;

        // Your engine already has HealthComponent (from TestScript)
        HealthComponent hc = GetHealth();

        // If hp was never set, initialize it
        if (hc.hp <= 0)
            hc.hp = maxHP;

        // You can also clamp hp if prefabs set it above max
        if (hc.hp > maxHP)
            hc.hp = maxHP;

        hc.isAlive = true;

        initialized = true;

        // Notify health bar controller if it exists
        if (HealthBarController.Instance != null)
        {
            HealthBarController.Instance.SetPlayerHealthReference(this);
        }

        Console.WriteLine($"[PlayerHealth] Initialized with {hc.hp}/{maxHP} HP");
    }

    // =========================
    // Public API
    // =========================
    public void TakeDamage(int dmg)
    {
        EnsureInit();

        if (dmg <= 0) return;

        HealthComponent hc = GetHealth();
        int oldHP = hc.hp;
        hc.hp -= dmg;

        if (hc.hp < 0)
            hc.hp = 0;

        Console.WriteLine($"[PlayerHealth] Took {dmg} damage! HP: {oldHP} → {hc.hp}");

        // Notify health bar
        if (HealthBarController.Instance != null)
        {
            HealthBarController.Instance.ForceUpdate();
        }

        if (hc.hp <= 0)
            Die();
    }

    public void Heal(int amount)
    {
        EnsureInit();

        if (amount <= 0) return;

        HealthComponent hc = GetHealth();
        int oldHP = hc.hp;
        hc.hp += amount;

        if (hc.hp > maxHP)
            hc.hp = maxHP;

        Console.WriteLine($"[PlayerHealth] Healed {amount}! HP: {oldHP} → {hc.hp}");

        // Notify health bar
        if (HealthBarController.Instance != null)
        {
            HealthBarController.Instance.ForceUpdate();
        }
    }

    public void Die()
    {
        EnsureInit();

        HealthComponent hc = GetHealth();
        hc.isAlive = false;

        Console.WriteLine("[PlayerHealth] ☠️ Player defeated!");

        // Notify health bar one last time
        if (HealthBarController.Instance != null)
        {
            HealthBarController.Instance.ForceUpdate();
        }

        // Engine-safe lose handling (no GameStateManager singleton assumed)
        Scene.LoadScene(loseSceneName);
    }

    // =========================
    // Convenience
    // =========================
    public int GetCurrentHP()
    {
        EnsureInit();
        return GetHealth().hp;
    }

    public float GetHP01()
    {
        EnsureInit();
        int hp = GetHealth().hp;
        return (maxHP <= 0) ? 0.0f : (float)hp / (float)maxHP;
    }
}