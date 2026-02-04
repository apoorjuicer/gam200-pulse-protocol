using System;
using ScriptAPI;

public class HealthBarController : Script
{
    // =========================
    // Singleton Access
    // =========================
    public static HealthBarController Instance { get; private set; }

    // =========================
    // Config - Health Bar Sprites
    // =========================
    public string Bar9Sprite = "../../PulseEngine/Assets/Textures/UI/Bar_9.png";  // Full health
    public string Bar8Sprite = "../../PulseEngine/Assets/Textures/UI/Bar_8.png";
    public string Bar7Sprite = "../../PulseEngine/Assets/Textures/UI/Bar_7.png";
    public string Bar6Sprite = "../../PulseEngine/Assets/Textures/UI/Bar_6.png";
    public string Bar5Sprite = "../../PulseEngine/Assets/Textures/UI/Bar_5.png";
    public string Bar4Sprite = "../../PulseEngine/Assets/Textures/UI/Bar_4.png";
    public string Bar3Sprite = "../../PulseEngine/Assets/Textures/UI/Bar_3.png";
    public string Bar2Sprite = "../../PulseEngine/Assets/Textures/UI/Bar_2.png";
    public string Bar1Sprite = "../../PulseEngine/Assets/Textures/UI/Bar_1.png";  // Low health

    // =========================
    // Config - Reference to Player
    // =========================
    public string playerEntityName = "Player";

    // =========================
    // Internal State
    // =========================
    private bool initialized = false;
    private PlayerHealth playerHealth;
    private int lastKnownHP = -1;

    private void Awake()
    {
        // Set singleton
        if (Instance == null)
            Instance = this;

        Console.WriteLine("[HealthBarController] Initialized");
    }

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            initialized = true;
            return;
        }

        // Continuously check if player health has changed
        if (playerHealth != null)
        {
            int currentHP = playerHealth.GetCurrentHP();
            
            if (currentHP != lastKnownHP)
            {
                UpdateHealthBar(currentHP, playerHealth.maxHP);
                lastKnownHP = currentHP;
            }
        }
    }

    private void Initialize()
    {
        Console.WriteLine("[HealthBarController] Looking for player health system...");

        // Try to find the player entity by name
        // Note: You might need to adapt this based on how your engine finds entities
        // This assumes you have a way to get PlayerHealth component
        
        // For now, we'll try to get it directly (you may need to adjust this)
        playerHealth = GetPlayerHealthComponent();

        if (playerHealth != null)
        {
            Console.WriteLine("[HealthBarController] Found PlayerHealth component!");
            lastKnownHP = playerHealth.GetCurrentHP();
            UpdateHealthBar(lastKnownHP, playerHealth.maxHP);
        }
        else
        {
            Console.WriteLine("[HealthBarController] WARNING: Could not find PlayerHealth component!");
        }
    }

    /// <summary>
    /// Updates the health bar sprite based on current HP
    /// </summary>
    private void UpdateHealthBar(int currentHP, int maxHP)
    {
        // Calculate health percentage (0.0 to 1.0)
        float healthPercent = (float)currentHP / (float)maxHP;

        // Determine which sprite to show (9 stages)
        string newSprite = GetSpriteForHealth(healthPercent);

        // Update the render component
        RenderComponent render = GetRender();
        if (render != null)
        {
            render.spriteName = newSprite;
            Console.WriteLine($"[HealthBarController] HP: {currentHP}/{maxHP} ({healthPercent:P0}) - Sprite: {newSprite}");
        }
    }

    /// <summary>
    /// Returns the appropriate sprite based on health percentage
    /// </summary>
    private string GetSpriteForHealth(float healthPercent)
    {
        // 9 stages from full to empty
        if (healthPercent >= 0.99f)
            return Bar9Sprite;  // 100%
        else if (healthPercent >= 0.875f)
            return Bar8Sprite;  // ~88%
        else if (healthPercent >= 0.75f)
            return Bar7Sprite;  // ~75%
        else if (healthPercent >= 0.625f)
            return Bar6Sprite;  // ~62%
        else if (healthPercent >= 0.50f)
            return Bar5Sprite;  // ~50%
        else if (healthPercent >= 0.375f)
            return Bar4Sprite;  // ~37%
        else if (healthPercent >= 0.25f)
            return Bar3Sprite;  // ~25%
        else if (healthPercent >= 0.125f)
            return Bar2Sprite;  // ~12%
        else
            return Bar1Sprite;  // < 12%
    }

    /// <summary>
    /// Helper method to get PlayerHealth component
    /// You may need to modify this based on your engine's API
    /// </summary>
    private PlayerHealth GetPlayerHealthComponent()
    {
        // OPTION 1: If PlayerHealth has a singleton
        // return PlayerHealth.Instance;

        // OPTION 2: If you need to search for it
        // This is a placeholder - you'll need to implement entity search
        // based on your engine's capabilities
        
        // For now, return null and you'll need to manually wire this up
        // or implement entity search functionality
        Console.WriteLine("[HealthBarController] You need to implement GetPlayerHealthComponent()");
        Console.WriteLine("  Option A: Add 'public static PlayerHealth Instance' to PlayerHealth.cs");
        Console.WriteLine("  Option B: Implement entity search in your engine");
        
        return null;
    }

    /// <summary>
    /// Public method to manually set the player health reference
    /// Call this from PlayerHealth.cs if automatic detection doesn't work
    /// </summary>
    public void SetPlayerHealthReference(PlayerHealth health)
    {
        playerHealth = health;
        Console.WriteLine("[HealthBarController] Player health reference manually set");
        
        if (health != null)
        {
            lastKnownHP = health.GetCurrentHP();
            UpdateHealthBar(lastKnownHP, health.maxHP);
        }
    }

    /// <summary>
    /// Force an immediate health bar update
    /// </summary>
    public void ForceUpdate()
    {
        if (playerHealth != null)
        {
            int currentHP = playerHealth.GetCurrentHP();
            UpdateHealthBar(currentHP, playerHealth.maxHP);
        }
    }
}
