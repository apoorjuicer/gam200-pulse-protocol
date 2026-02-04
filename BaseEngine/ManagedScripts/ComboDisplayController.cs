using System;
using ScriptAPI;

/// <summary>
/// PRACTICAL COMBO DISPLAY - Uses Pre-Created Child Entities
/// 
/// CONFIGURED FOR YOUR SCENE:
/// - ComboDisplay (Entity 33) - Parent entity
/// - KeySlot0 (Entity 34) - First key display
/// - KeySlot1 (Entity 35) - Second key display
/// - KeySlot2 (Entity 36) - Third key display
/// - KeySlot3 (Entity 37) - Fourth key display
/// - JudgementDisplay (Entity 38) - Shows perfect/great/miss
/// </summary>
public class ComboDisplayController : Script
{
    // =========================
    // Global Access (Singleton)
    // =========================
    public static ComboDisplayController Instance { get; private set; }

    // =========================
    // Scene Entity IDs - CONFIGURED FOR YOUR SCENE
    // =========================
    public int ComboDisplayParent_EntityID = 33;  // ComboDisplay entity

    public int KeySlot0_EntityID = 34;  // First key slot
    public int KeySlot1_EntityID = 35;  // Second key slot
    public int KeySlot2_EntityID = 36;  // Third key slot
    public int KeySlot3_EntityID = 37;  // Fourth key slot

    public int JudgementDisplay_EntityID = 38;  // Perfect/Great/Miss display

    // =========================
    // Sprite paths (relative to Assets/Textures/)
    // Change these to match your actual file paths
    // =========================
    public string WSprite = "W.png";
    public string ASprite = "A.png";
    public string SSpr = "S.png";
    public string DSprite = "D.png";

    public string PerfectSprite = "perfect.png";
    public string GreatSprite = "great.png";
    public string MissSprite = "miss.png";

    // =========================
    // Timing / Behaviour
    // =========================
    public float KeyDisplayDuration = 2.0f;
    public float JudgementDisplayDuration = 1.0f;

    private float keyTimer = 0f;
    private float judgementTimer = 0f;

    private string currentCombo = "";
    private int currentProgress = 0;

    private void Awake()
    {
        // Singleton protection - only allow one instance
        if (Instance != null && Instance != this)
        {
            Console.WriteLine("[ComboDisplay] ⚠️ WARNING: Multiple instances detected! This instance will not be used.");
            return;
        }

        Instance = this;
        Console.WriteLine("[ComboDisplay] ✅ Initialized");
        Console.WriteLine($"[ComboDisplay] Entity IDs configured:");
        Console.WriteLine($"  - Parent: {ComboDisplayParent_EntityID}");
        Console.WriteLine($"  - KeySlots: {KeySlot0_EntityID}, {KeySlot1_EntityID}, {KeySlot2_EntityID}, {KeySlot3_EntityID}");
        Console.WriteLine($"  - Judgement: {JudgementDisplay_EntityID}");
    }

    private void Update()
    {
        UpdateTimers();
    }

    // =========================
    // Main Interface
    // =========================
    public void DisplayCombo(string comboString)
    {
        currentCombo = comboString;
        currentProgress = 0;

        Console.WriteLine($"[ComboDisplay] 🎮 Displaying combo: {comboString}");

        for (int i = 0; i < 4; i++)
        {
            if (i < comboString.Length)
            {
                char key = comboString[i];
                string spritePath = GetSpriteForKey(key);

                ShowKeySlot(i, spritePath);
            }
            else
            {
                HideKeySlot(i);
            }
        }

        keyTimer = KeyDisplayDuration;
    }

    public void UpdateProgress(int progress)
    {
        currentProgress = progress;

        // TODO: Optional visual feedback: dim/disable completed key slots
        // For now, just log it.
        Console.WriteLine($"[ComboDisplay] Progress: {progress}/{currentCombo.Length}");
    }

    public void ShowJudgement(string judgement)
    {
        if (JudgementDisplay_EntityID == 0)
        {
            Console.WriteLine("[ComboDisplay] ⚠️ Judgement display entity not set");
            return;
        }

        string spritePath = GetSpriteForJudgement(judgement);

        Console.WriteLine($"[ComboDisplay] ⭐ Showing {judgement} -> {spritePath}");

        // TODO: Set sprite and make visible
        // Once your engine supports it, uncomment these lines:
        // var sprite = Scene.GetComponent<SpriteComponent>(JudgementDisplay_EntityID);
        // sprite.Texture = spritePath;
        // var transform = Scene.GetComponent<TransformComponent>(JudgementDisplay_EntityID);
        // transform.IsVisible = true;

        judgementTimer = JudgementDisplayDuration;
    }

    // =========================
    // Internal
    // =========================
    private void UpdateTimers()
    {
        if (keyTimer > 0)
        {
            keyTimer -= Time.DeltaTime;
            if (keyTimer <= 0)
            {
                ClearAllKeys();
            }
        }

        if (judgementTimer > 0)
        {
            judgementTimer -= Time.DeltaTime;
            if (judgementTimer <= 0)
            {
                HideJudgement();
            }
        }
    }

    private void ShowKeySlot(int slotIndex, string spritePath)
    {
        int entityID = GetKeySlotEntityID(slotIndex);
        if (entityID == 0)
        {
            Console.WriteLine($"[ComboDisplay] ⚠️ KeySlot{slotIndex} entity not set");
            return;
        }

        Console.WriteLine($"[ComboDisplay] Showing slot {slotIndex} (Entity {entityID}) sprite: {spritePath}");

        // TODO: Set sprite + make visible
        // Once your engine supports it, uncomment these lines:
        // var sprite = Scene.GetComponent<SpriteComponent>(entityID);
        // sprite.Texture = spritePath;
        // var transform = Scene.GetComponent<TransformComponent>(entityID);
        // transform.IsVisible = true;
    }

    private void HideKeySlot(int slotIndex)
    {
        int entityID = GetKeySlotEntityID(slotIndex);
        if (entityID == 0) return;

        Console.WriteLine($"[ComboDisplay] Hiding slot {slotIndex} (Entity {entityID})");

        // TODO: Hide the entity
        // Once your engine supports it, uncomment this line:
        // var transform = Scene.GetComponent<TransformComponent>(entityID);
        // transform.IsVisible = false;
    }

    private void HideJudgement()
    {
        if (JudgementDisplay_EntityID == 0) return;

        Console.WriteLine($"[ComboDisplay] Hiding judgement (Entity {JudgementDisplay_EntityID})");

        // TODO: Hide the entity
        // Once your engine supports it, uncomment this line:
        // var transform = Scene.GetComponent<TransformComponent>(JudgementDisplay_EntityID);
        // transform.IsVisible = false;
    }

    private void ClearAllKeys()
    {
        Console.WriteLine("[ComboDisplay] 🧹 Clearing all keys");

        for (int i = 0; i < 4; i++)
        {
            HideKeySlot(i);
        }
    }

    private int GetKeySlotEntityID(int slotIndex)
    {
        switch (slotIndex)
        {
            case 0: return KeySlot0_EntityID;  // Entity 34
            case 1: return KeySlot1_EntityID;  // Entity 35
            case 2: return KeySlot2_EntityID;  // Entity 36
            case 3: return KeySlot3_EntityID;  // Entity 37
            default:
                Console.WriteLine($"[ComboDisplay] ⚠️ Invalid slot index: {slotIndex}");
                return 0;
        }
    }

    private string GetSpriteForKey(char key)
    {
        switch (char.ToUpper(key))
        {
            case 'W': return WSprite;
            case 'A': return ASprite;
            case 'S': return SSpr;
            case 'D': return DSprite;
            default: return "";
        }
    }

    private string GetSpriteForJudgement(string judgement)
    {
        if (string.IsNullOrEmpty(judgement)) return MissSprite;

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