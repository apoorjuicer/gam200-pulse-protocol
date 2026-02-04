using System;
using ScriptAPI;

/// <summary>
/// Health bar UI that displays player's current HP - FIXED VERSION
/// 
/// SETUP:
/// - Attach this script to your HEALTH BAR entity (the sprite that shows HP)
/// - The health bar entity needs a SpriteComponent and TransformComponent
/// - PlayerHealth.Instance must exist for this to work
/// - This script will continuously check for PlayerHealth and report its status
/// 
/// HOW IT WORKS:
/// - Reads PlayerHealth.Instance.GetHPPercent() every frame
/// - Scales the health bar horizontally based on HP percentage
/// - Changes color: Green > Yellow > Red as HP decreases
/// </summary>
public class HealthBarUI : Script
{
    // =========================
    // Config
    // =========================
    public float FullBarWidth = 200f;  // Width of health bar at 100% HP
    public bool ChangeColorWithHP = true;

    // Color thresholds
    public float YellowThreshold = 0.5f;  // Below 50% HP -> yellow
    public float RedThreshold = 0.25f;    // Below 25% HP -> red

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private float lastHPPercent = 1.0f;
    private int updateCount = 0;
    private int playerHealthCheckAttempts = 0;
    private bool playerHealthFound = false;

    // Store initial position and size
    private float initialX = 0f;
    private float initialY = 0f;

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

        // Keep checking for PlayerHealth if we haven't found it yet
        if (!playerHealthFound && playerHealthCheckAttempts < 300) // Check for 5 seconds at 60fps
        {
            playerHealthCheckAttempts++;

            if (PlayerHealth.Instance != null)
            {
                playerHealthFound = true;
                Console.WriteLine($"[HealthBarUI] ? PlayerHealth found after {playerHealthCheckAttempts} attempts!");
            }
            else if (playerHealthCheckAttempts % 60 == 0) // Print every second
            {
                Console.WriteLine($"[HealthBarUI] ? Still waiting for PlayerHealth... (attempt {playerHealthCheckAttempts}/300)");
            }

            if (playerHealthCheckAttempts == 300)
            {
                Console.WriteLine("[HealthBarUI] ? ERROR: PlayerHealth.Instance is STILL null after 5 seconds!");
                Console.WriteLine("[HealthBarUI] Make sure PlayerHealth script is attached to the Player entity!");
            }
        }

        // Check if PlayerHealth exists
        if (PlayerHealth.Instance == null)
        {
            return; // Can't update without player health
        }

        // Get current HP percentage
        float hpPercent = PlayerHealth.Instance.GetHPPercent();

        // Only update if HP changed (optimization + reduces log spam)
        if (Math.Abs(hpPercent - lastHPPercent) > 0.01f)
        {
            UpdateBar(hpPercent);
            lastHPPercent = hpPercent;
        }

        // Status report every 2 seconds
        if (updateCount % 120 == 0)
        {
            Console.WriteLine($"[HealthBarUI] Status: HP={hpPercent:P0}, Bar width={GetTransform().ScaleX:F1}");
        }
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        Console.WriteLine("[HealthBarUI] ???????????????????????????????");
        Console.WriteLine("[HealthBarUI] Initializing Health Bar UI...");
        Console.WriteLine("[HealthBarUI] ???????????????????????????????");

        // Store initial transform
        TransformComponent tf = GetTransform();
        initialX = tf.X;
        initialY = tf.Y;

        Console.WriteLine($"[HealthBarUI] ? Initial position: ({initialX}, {initialY})");
        Console.WriteLine($"[HealthBarUI] ? Full bar width: {FullBarWidth}");
        Console.WriteLine($"[HealthBarUI] ? Color changes: {ChangeColorWithHP}");
        Console.WriteLine($"[HealthBarUI] ? Yellow threshold: {YellowThreshold:P0}");
        Console.WriteLine($"[HealthBarUI] ? Red threshold: {RedThreshold:P0}");

        // Check for sprite component
        SpriteComponent sprite = GetSprite();
        if (sprite.HasSprite())
        {
            Console.WriteLine("[HealthBarUI] ? Sprite component found");
        }
        else
        {
            Console.WriteLine("[HealthBarUI] ? WARNING: No sprite found - health bar won't be visible!");
        }

        // Check for PlayerHealth
        if (PlayerHealth.Instance != null)
        {
            playerHealthFound = true;
            Console.WriteLine("[HealthBarUI] ? PlayerHealth found immediately!");
            float hpPercent = PlayerHealth.Instance.GetHPPercent();
            UpdateBar(hpPercent);
        }
        else
        {
            Console.WriteLine("[HealthBarUI] ? PlayerHealth not found yet - will keep checking...");
            Console.WriteLine("[HealthBarUI] Make sure PlayerHealth script is on the Player entity!");
        }

        Console.WriteLine("[HealthBarUI] ???????????????????????????????");
        Console.WriteLine("[HealthBarUI] Initialization complete!");
        Console.WriteLine("[HealthBarUI] ???????????????????????????????");

        initialized = true;
    }

    // =========================
    // Bar Update
    // =========================
    private void UpdateBar(float hpPercent)
    {
        // Clamp to 0-1
        hpPercent = Math.Max(0.0f, Math.Min(1.0f, hpPercent));

        Console.WriteLine($"[HealthBarUI] ?? Updating bar: HP={hpPercent:P0}");

        // Update bar width
        UpdateBarWidth(hpPercent);

        // Update bar color
        if (ChangeColorWithHP)
        {
            UpdateBarColor(hpPercent);
        }
    }

    private void UpdateBarWidth(float hpPercent)
    {
        // Scale the bar horizontally based on HP
        TransformComponent tf = GetTransform();
        float newWidth = FullBarWidth * hpPercent;
        tf.ScaleX = newWidth;

        Console.WriteLine($"[HealthBarUI] ? Bar width updated: {newWidth:F1} (was {tf.ScaleX:F1})");

        // Optional: Also move the bar left so it shrinks from the right
        // This keeps the left edge of the bar fixed and makes it drain from right to left
        // Uncomment if you want this behavior:
        // float offset = FullBarWidth * (1.0f - hpPercent) * 0.5f;
        // tf.X = initialX - offset;
    }

    private void UpdateBarColor(float hpPercent)
    {
        SpriteComponent sprite = GetSprite();
        if (!sprite.HasSprite())
        {
            Console.WriteLine("[HealthBarUI] ? Cannot update color - no sprite!");
            return;
        }

        string colorName = "";

        if (hpPercent > YellowThreshold)
        {
            // Green (healthy)
            sprite.SetTint(0.0f, 1.0f, 0.0f, 1.0f);
            colorName = "Green";
        }
        else if (hpPercent > RedThreshold)
        {
            // Yellow (warning)
            sprite.SetTint(1.0f, 1.0f, 0.0f, 1.0f);
            colorName = "Yellow";
        }
        else
        {
            // Red (critical)
            sprite.SetTint(1.0f, 0.0f, 0.0f, 1.0f);
            colorName = "Red";
        }

        Console.WriteLine($"[HealthBarUI] ? Color updated: {colorName}");
    }

    // =========================
    // Public API
    // =========================
    public bool IsPlayerHealthConnected()
    {
        return playerHealthFound && PlayerHealth.Instance != null;
    }

    public void ForceUpdate()
    {
        if (PlayerHealth.Instance != null)
        {
            float hpPercent = PlayerHealth.Instance.GetHPPercent();
            UpdateBar(hpPercent);
            lastHPPercent = hpPercent;
        }
    }
}