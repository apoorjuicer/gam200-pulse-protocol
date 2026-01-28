using System;
using ScriptAPI;

public class PlayerHealth : Script
{
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
    }

    // =========================
    // Public API
    // =========================
    public void TakeDamage(int dmg)
    {
        EnsureInit();

        if (dmg <= 0) return;

        HealthComponent hc = GetHealth();
        hc.hp -= dmg;

        if (hc.hp < 0)
            hc.hp = 0;

        if (hc.hp <= 0)
            Die();
    }

    public void Heal(int amount)
    {
        EnsureInit();

        if (amount <= 0) return;

        HealthComponent hc = GetHealth();
        hc.hp += amount;

        if (hc.hp > maxHP)
            hc.hp = maxHP;
    }

    public void Die()
    {
        EnsureInit();

        HealthComponent hc = GetHealth();
        hc.isAlive = false;

        Console.WriteLine("Player defeated!");

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
