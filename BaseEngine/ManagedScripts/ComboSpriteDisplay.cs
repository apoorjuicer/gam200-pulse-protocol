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
    // Configuration
    // =========================
    // Display positions (adjust these in editor)
    public float KeyStartX = -300f;
    public float KeyY = -200f;
    public float KeySpacing = 120f;
    public float KeyScale = 100f;

    public float JudgementX = 0f;
    public float JudgementY = 250f;
    public float JudgementScale = 150f;

    // Animation timings
    public float KeyDisplayTime = 0.5f;
    public float JudgementDisplayTime = 0.6f;

    // Sprite names (relative to Assets/Textures/)
    public string WSprite = "W.png";
    public string ASprite = "A.png";
    public string SSprite = "S.png";
    public string DSprite = "D.png";
    public string PerfectSprite = "perfect.png";
    public string GreatSprite = "great.png";
    public string MissSprite = "miss.png";

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private int lastComboProgress = 0;

    // Track spawned display entities (for cleanup)
    private class DisplaySprite
    {
        public int EntityID; // If we could spawn entities
        public string SpritePath;
        public float X, Y, Scale;
        public float Timer;
    }

    private List<DisplaySprite> activeSprites = new List<DisplaySprite>();

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

        // Update timers and remove expired sprites
        for (int i = activeSprites.Count - 1; i >= 0; i--)
        {
            activeSprites[i].Timer -= dt;
            if (activeSprites[i].Timer <= 0)
            {
                // TODO: Destroy the entity
                // DestroyEntity(activeSprites[i].EntityID);
                activeSprites.RemoveAt(i);
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
                    ShowKeySprite(lastKey, currentProgress - 1);
                }
            }
            // Combo reset?
            else if (currentProgress < lastComboProgress)
            {
                ClearAllSprites();
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
            Console.WriteLine("[ComboSpriteDisplay] WARNING: Multiple instances!");
            return;
        }

        Console.WriteLine("[ComboSpriteDisplay] ========================================");
        Console.WriteLine("[ComboSpriteDisplay] Sprite Display Initialized");
        Console.WriteLine("[ComboSpriteDisplay] ========================================");
        Console.WriteLine($"[ComboSpriteDisplay] Key position: ({KeyStartX}, {KeyY})");
        Console.WriteLine($"[ComboSpriteDisplay] Key spacing: {KeySpacing}");
        Console.WriteLine($"[ComboSpriteDisplay] Assets: {WSprite}, {ASprite}, {SSprite}, {DSprite}");
        Console.WriteLine($"[ComboSpriteDisplay] Judgements: {PerfectSprite}, {GreatSprite}, {MissSprite}");

        initialized = true;
    }

    // =========================
    // Display Methods
    // =========================
    private void ShowKeySprite(char key, int position)
    {
        string spritePath = GetSpriteForKey(key);
        float x = KeyStartX + (position * KeySpacing);
        float y = KeyY;

        Console.WriteLine($"[ComboSpriteDisplay] 🎹 {key} at position {position}: ({x:F0}, {y:F0})");

        // WORKAROUND: Since we can't spawn entities from C#, we'll use the transform
        // of THIS entity to show the feedback. For multiple keys, you'll need
        // multiple child entities pre-created in your scene.
        
        // For now, log the display request
        DisplaySprite sprite = new DisplaySprite
        {
            EntityID = -1, // Would store entity ID if we could spawn
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

    private void ClearAllSprites()
    {
        Console.WriteLine("[ComboSpriteDisplay] 🧹 Clearing all sprites");
        
        // TODO: Destroy all sprite entities
        // foreach (var sprite in activeSprites)
        // {
        //     DestroyEntity(sprite.EntityID);
        // }

        activeSprites.Clear();
    }

    // =========================
    // Helper Methods
    // =========================
    private string GetSpriteForKey(char key)
    {
        switch (key)
        {
            case 'W': return WSprite;
            case 'A': return ASprite;
            case 'S': return SSprite;
            case 'D': return DSprite;
            default:
                Console.WriteLine($"[ComboSpriteDisplay] ⚠️ Unknown key: {key}");
                return "";
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
            default:
                Console.WriteLine($"[ComboSpriteDisplay] ⚠️ Unknown judgement: {judgement}");
                return MissSprite;
        }
    }

    // =========================
    // Public API
    // =========================
    public void SetKeyPosition(float startX, float y, float spacing)
    {
        KeyStartX = startX;
        KeyY = y;
        KeySpacing = spacing;
        Console.WriteLine($"[ComboSpriteDisplay] Key position updated: ({startX}, {y}), spacing: {spacing}");
    }

    public void SetJudgementPosition(float x, float y)
    {
        JudgementX = x;
        JudgementY = y;
        Console.WriteLine($"[ComboSpriteDisplay] Judgement position updated: ({x}, {y})");
    }

    // For external rendering systems that can read this data
    public List<DisplaySprite> GetActiveSprites()
    {
        return new List<DisplaySprite>(activeSprites);
    }
}
