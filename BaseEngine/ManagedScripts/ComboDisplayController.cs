using System;
using ScriptAPI;

/// <summary>
/// PRACTICAL COMBO DISPLAY - Uses Pre-Created Child Entities
/// 
/// SETUP IN YOUR SCENE:
/// 1. Create a "ComboDisplay" parent entity
/// 2. Attach THIS script to it
/// 3. Create 4 child entities for keys (KeySlot0, KeySlot1, KeySlot2, KeySlot3)
/// 4. Create 1 child entity for judgement (JudgementDisplay)
/// 5. Set their IsVisible = false initially
/// 6. This script will control their visibility and sprites
/// </summary>
public class ComboDisplayController : Script
{
    // =========================
    // Global Access (Singleton)
    // =========================
    public static ComboDisplayController Instance { get; private set; }

    // =========================
    // Configuration - Set these in your scene/editor
    // =========================
    // Entity IDs of child entities (set these manually!)
    public int KeySlot0_EntityID = 0;
    public int KeySlot1_EntityID = 0;
    public int KeySlot2_EntityID = 0;
    public int KeySlot3_EntityID = 0;
    public int JudgementDisplay_EntityID = 0;

    // Sprite paths
    public string WSprite = "W.png";
    public string ASprite = "A.png";
    public string SSprite = "S.png";
    public string DSprite = "D.png";
    public string PerfectSprite = "perfect.png";
    public string GreatSprite = "great.png";
    public string MissSprite = "miss.png";

    // Display timings
    public float KeyDisplayDuration = 0.5f;
    public float JudgementDisplayDuration = 0.6f;

    // =========================
    // Internal State
    // =========================
    private bool initialized = false;
    private int lastComboProgress = 0;

    // Timers for each slot
    private float[] keyTimers = new float[4];
    private float judgementTimer = 0f;

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

        float dt = Time.DeltaTime;

        // Update key display timers
        for (int i = 0; i < 4; i++)
        {
            if (keyTimers[i] > 0)
            {
                keyTimers[i] -= dt;
                if (keyTimers[i] <= 0)
                {
                    HideKeySlot(i);
                }
            }
        }

        // Update judgement timer
        if (judgementTimer > 0)
        {
            judgementTimer -= dt;
            if (judgementTimer <= 0)
            {
                HideJudgement();
            }
        }

        // Monitor combo progress
        if (ComboSystem.Instance != null)
        {
            int currentProgress = ComboSystem.Instance.GetComboProgress();
            
            // New key pressed?
            if (currentProgress > lastComboProgress)
            {
                string combo = ComboSystem.Instance.GetCurrentComboString();
                if (combo.Length > 0)
                {
                    char lastKey = combo[combo.Length - 1];
                    ShowKey(lastKey, currentProgress - 1);
                }
            }
            // Combo reset?
            else if (currentProgress < lastComboProgress)
            {
                ClearAllKeys();
            }

            lastComboProgress = currentProgress;
        }
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        if (Instance == null)
        {
            Instance = this;
        }
        else if (Instance != this)
        {
            Console.WriteLine("[ComboDisplay] WARNING: Multiple instances!");
            return;
        }

        Console.WriteLine("[ComboDisplay] ========================================");
        Console.WriteLine("[ComboDisplay] Combo Display Controller Initialized");
        Console.WriteLine("[ComboDisplay] ========================================");

        // Verify child entities are set
        if (KeySlot0_EntityID == 0 || KeySlot1_EntityID == 0 || 
            KeySlot2_EntityID == 0 || KeySlot3_EntityID == 0)
        {
            Console.WriteLine("[ComboDisplay] ⚠️ WARNING: Key slot entity IDs not set!");
            Console.WriteLine("[ComboDisplay] Please set KeySlot0-3_EntityID in the editor");
        }

        if (JudgementDisplay_EntityID == 0)
        {
            Console.WriteLine("[ComboDisplay] ⚠️ WARNING: Judgement display entity ID not set!");
        }

        // Hide all displays initially
        ClearAllKeys();
        HideJudgement();

        Console.WriteLine("[ComboDisplay] Ready to display combo feedback!");
        initialized = true;
    }

    // =========================
    // Display Control
    // =========================
    public void ShowKey(char key, int slotIndex)
    {
        if (slotIndex < 0 || slotIndex > 3)
        {
            Console.WriteLine($"[ComboDisplay] ⚠️ Invalid slot index: {slotIndex}");
            return;
        }

        int entityID = GetKeySlotEntityID(slotIndex);
        if (entityID == 0)
        {
            Console.WriteLine($"[ComboDisplay] ⚠️ Slot {slotIndex} entity ID not set");
            return;
        }

        string spritePath = GetSpriteForKey(key);
        
        Console.WriteLine($"[ComboDisplay] 🎹 Showing {key} in slot {slotIndex}");

        // TODO: Set the sprite and make visible
        // Since your engine doesn't expose entity manipulation from C# yet,
        // this is a placeholder that shows the INTENT
        
        // What this WOULD do:
        // var sprite = Scene.GetComponent<SpriteComponent>(entityID);
        // sprite.Texture = spritePath;
        // var transform = Scene.GetComponent<TransformComponent>(entityID);
        // transform.IsVisible = true;

        keyTimers[slotIndex] = KeyDisplayDuration;
    }

    public void ShowJudgement(string judgement)
    {
        if (JudgementDisplay_EntityID == 0)
        {
            Console.WriteLine("[ComboDisplay] ⚠️ Judgement display entity not set");
            return;
        }

        string spritePath = GetSpriteForJudgement(judgement);
        
        Console.WriteLine($"[ComboDisplay] ⭐ Showing {judgement}");

        // TODO: Set sprite and make visible
        // var sprite = Scene.GetComponent<SpriteComponent>(JudgementDisplay_EntityID);
        // sprite.Texture = spritePath;
        // var transform = Scene.GetComponent<TransformComponent>(JudgementDisplay_EntityID);
        // transform.IsVisible = true;

        judgementTimer = JudgementDisplayDuration;
    }

    private void HideKeySlot(int slotIndex)
    {
        int entityID = GetKeySlotEntityID(slotIndex);
        if (entityID == 0) return;

        Console.WriteLine($"[ComboDisplay] Hiding slot {slotIndex}");

        // TODO: Hide the entity
        // var transform = Scene.GetComponent<TransformComponent>(entityID);
        // transform.IsVisible = false;
    }

    private void HideJudgement()
    {
        if (JudgementDisplay_EntityID == 0) return;

        Console.WriteLine("[ComboDisplay] Hiding judgement");

        // TODO: Hide the entity
        // var transform = Scene.GetComponent<TransformComponent>(JudgementDisplay_EntityID);
        // transform.IsVisible = false;
    }

    private void ClearAllKeys()
    {
        Console.WriteLine("[ComboDisplay] 🧹 Clearing all keys");
        
        for (int i = 0; i < 4; i++)
        {
            keyTimers[i] = 0;
            HideKeySlot(i);
        }
    }

    // =========================
    // Helper Methods
    // =========================
    private int GetKeySlotEntityID(int slotIndex)
    {
        switch (slotIndex)
        {
            case 0: return KeySlot0_EntityID;
            case 1: return KeySlot1_EntityID;
            case 2: return KeySlot2_EntityID;
            case 3: return KeySlot3_EntityID;
            default: return 0;
        }
    }

    private string GetSpriteForKey(char key)
    {
        switch (key)
        {
            case 'W': return WSprite;
            case 'A': return ASprite;
            case 'S': return SSprite;
            case 'D': return DSprite;
            default: return "";
        }
    }

    private string GetSpriteForJudgement(string judgement)
    {
        switch (judgement.ToUpper())
        {
            case "PERFECT": return PerfectSprite;
            case "GOOD":
            case "GREAT": return GreatSprite;
            case "MISS":
            case "LATE": return MissSprite;
            default: return MissSprite;
        }
    }
}
