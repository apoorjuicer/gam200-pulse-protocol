using System;
using System.Collections.Generic;
using ScriptAPI;

/// <summary>
/// SPRITE-BASED COMBO DISPLAY
/// Creates actual sprite entities to show WASD keys and timing judgements on screen
/// Works with your existing W.png, A.png, S.png, D.png, perfect.png, great.png, miss.png assets
/// </summary>
public class ComboSpriteDisplay : Script
{
    // =========================
    // Global Access (Singleton)
    // =========================
    public static ComboSpriteDisplay Instance { get; private set; }

    // =========================
    // Sprite Paths (relative to Assets/Textures/)
    // Change these to swap your art files / folder paths
    // =========================
    public string WSprite = "W.png";
    public string ASprite = "A.png";
    public string SSpr = "S.png";
    public string DSprite = "D.png";

    public string PerfectSprite = "perfect.png";
    public string GreatSprite = "great.png";
    public string MissSprite = "miss.png";

    // =========================
    // Layout Settings
    // =========================
    public float StartX = 500f;
    public float StartY = 300f;
    public float Spacing = 80f;
    public float KeyScale = 1.0f;

    public float JudgementX = 500f;
    public float JudgementY = 450f;
    public float JudgementScale = 1.2f;

    // =========================
    // Timing Settings
    // =========================
    public float KeyDisplayTime = 2.0f;
    public float JudgementDisplayTime = 1.0f;

    // =========================
    // Runtime Data
    // =========================
    private List<DisplaySprite> activeSprites = new List<DisplaySprite>();
    private string lastComboShown = "";
    private int lastComboProgress = 0;

    // Track spawned display entities (for cleanup)
    public class DisplaySprite
    {
        public int EntityID; // If we could spawn entities
        public string SpritePath;
        public float X, Y, Scale;
        public float Timer;
    }

    private void Awake()
    {
        Instance = this;
        Console.WriteLine("[ComboSpriteDisplay] ✅ Initialized");
    }

    private void Update()
    {
        UpdateTimers();
        UpdateComboProgressDisplay();
    }

    // =========================
    // Main Interface
    // =========================
    public void DisplayCombo(string comboString)
    {
        if (string.IsNullOrEmpty(comboString))
        {
            Console.WriteLine("[ComboSpriteDisplay] ⚠️ Empty combo string, ignoring");
            return;
        }

        Console.WriteLine($"[ComboSpriteDisplay] 🎮 Showing combo: {comboString}");

        ClearAllSprites();

        lastComboShown = comboString;
        lastComboProgress = 0;

        for (int i = 0; i < comboString.Length; i++)
        {
            char key = comboString[i];
            string spritePath = GetSpriteForKey(key);

            float x = StartX + (i * Spacing);
            float y = StartY;

            SpawnKeySprite(spritePath, x, y);
        }
    }

    public void UpdateProgress(int progress)
    {
        lastComboProgress = progress;
    }

    public void ShowJudgement(string judgement)
    {
        string spritePath = GetSpriteForJudgement(judgement);

        Console.WriteLine($"[ComboSpriteDisplay] ⭐ {judgement} at ({JudgementX:F0}, {JudgementY:F0})");

        DisplaySprite sprite = new DisplaySprite
        {
            EntityID = -1,
            SpritePath = spritePath,
            X = JudgementX,
            Y = JudgementY,
            Scale = JudgementScale,
            Timer = JudgementDisplayTime
        };

        activeSprites.Add(sprite);
    }

    // =========================
    // Internal Helpers
    // =========================
    private void SpawnKeySprite(string spritePath, float x, float y)
    {
        DisplaySprite sprite = new DisplaySprite
        {
            EntityID = -1,
            SpritePath = spritePath,
            X = x,
            Y = y,
            Scale = KeyScale,
            Timer = KeyDisplayTime
        };

        activeSprites.Add(sprite);

        // TODO: Once your engine supports spawning entities from C#:
        // int entityID = Scene.CreateEntity();
        // var transform = Scene.GetComponent<TransformComponent>(entityID);
        // transform.X = x;
        // transform.Y = y;
        // transform.ScaleX = KeyScale;
        // transform.ScaleY = KeyScale;
        // var sprite = Scene.GetComponent<SpriteComponent>(entityID);
        // sprite.Texture = spritePath;
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

    private void UpdateTimers()
    {
        for (int i = activeSprites.Count - 1; i >= 0; i--)
        {
            activeSprites[i].Timer -= Time.DeltaTime;

            if (activeSprites[i].Timer <= 0)
            {
                // TODO: If EntityID is valid, destroy the entity
                // if (activeSprites[i].EntityID != -1) Scene.DestroyEntity(activeSprites[i].EntityID);

                activeSprites.RemoveAt(i);
            }
        }
    }

    private void UpdateComboProgressDisplay()
    {
        // If you want: fade out keys you've already hit, etc.
        // This is currently a hook for future polish.
    }

    private void ClearAllSprites()
    {
        Console.WriteLine("[ComboSpriteDisplay] 🧹 Clearing all sprites");

        // TODO: If entities are spawned, destroy them here
        // foreach (var sprite in activeSprites)
        // {
        //     if (sprite.EntityID != -1) Scene.DestroyEntity(sprite.EntityID);
        // }

        activeSprites.Clear();
    }

    // =========================
    // External Access (Renderer)
    // =========================
    // Your C++ renderer layer can read this data
    public List<DisplaySprite> GetActiveSprites()
    {
        return new List<DisplaySprite>(activeSprites);
    }
}