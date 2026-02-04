using System;
using System.Collections.Generic;
using ScriptAPI;

/// <summary>
/// ENEMY QUEUE MANAGER (NOT A SPAWNER!)
/// This script manages the enemy encounter queue and controls ONE pre-existing enemy entity.
/// It does NOT spawn new entities - it updates the existing enemy's properties and UI.
/// </summary>
public class EnemySpawner : Script
{
    // =========================
    // Global Access (Singleton)
    // =========================
    public static EnemySpawner Instance { get; private set; }

    // =========================
    // Enemy Queue Definition
    // =========================
    [Serializable]
    public class EnemyData
    {
        public EnemyIconType IconType;
        public string WeaknessCombo;
        public float Health;
        public float AttackDamage;
        public float StepDistance;
        public string IdleSpriteSheet;
        public string WalkSpriteSheet;
        public string AttackSpriteSheet;
        public string HurtSpriteSheet;

        public EnemyData(EnemyIconType type, string combo, float hp, float dmg, float step,
                        string idle, string walk, string attack, string hurt)
        {
            IconType = type;
            WeaknessCombo = combo;
            Health = hp;
            AttackDamage = dmg;
            StepDistance = step;
            IdleSpriteSheet = idle;
            WalkSpriteSheet = walk;
            AttackSpriteSheet = attack;
            HurtSpriteSheet = hurt;
        }
    }

    // =========================
    // State
    // =========================
    private List<EnemyData> enemyQueue = new List<EnemyData>();
    private int currentEnemyIndex = 0;
    private bool initialized = false;

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
            Console.WriteLine("[EnemySpawner] WARNING: Multiple EnemySpawner instances detected!");
            return;
        }

        Console.WriteLine("[EnemySpawner] ==========================================");
        Console.WriteLine("[EnemySpawner] Initializing Enemy Queue Manager");
        Console.WriteLine("[EnemySpawner] ==========================================");

        // Build the enemy queue for this level
        BuildEnemyQueue();

        // Wait a frame for OrderSlipUIController to initialize
        Console.WriteLine("[EnemySpawner] Waiting for UI to initialize...");

        initialized = true;

        // Start the first enemy after initialization
        StartNextEnemy();
    }

    // =========================
    // Queue Building
    // =========================
    private void BuildEnemyQueue()
    {
        // Clear any existing queue
        enemyQueue.Clear();

        // Define Level 1 enemy sequence
        // RAT -> RAT -> BAT -> RAT -> GORILLA

        // Rat #1
        enemyQueue.Add(new EnemyData(
            EnemyIconType.Rat,
            "WASD",
            15f,
            12f,
            50f,
            "../Enemies/Rat_Idle_spritesheet_220x432",
            "../Enemies/Rat_Walk_spritesheet_322x433",
            "../Enemies/Rattack_spritesheet_634x431",
            "../Enemies/Rat_Hurt_spritesheet_515x428"
        ));

        // Rat #2
        enemyQueue.Add(new EnemyData(
            EnemyIconType.Rat,
            "ASWD",
            15f,
            12f,
            50f,
            "../Enemies/Rat_Idle_spritesheet_220x432",
            "../Enemies/Rat_Walk_spritesheet_322x433",
            "../Enemies/Rattack_spritesheet_634x431",
            "../Enemies/Rat_Hurt_spritesheet_515x428"
        ));

        // Bat
        enemyQueue.Add(new EnemyData(
            EnemyIconType.Bat,
            "DDSA",
            20f,
            15f,
            60f,
            "../Enemies/Bat_Idle_spritesheet_220x220",
            "../Enemies/Bat_Walk_spritesheet_220x220",
            "../Enemies/Bat_Attack_spritesheet_220x220",
            "../Enemies/Bat_Hurt_spritesheet_220x220"
        ));

        // Rat #3
        enemyQueue.Add(new EnemyData(
            EnemyIconType.Rat,
            "SWAD",
            15f,
            12f,
            50f,
            "../Enemies/Rat_Idle_spritesheet_220x432",
            "../Enemies/Rat_Walk_spritesheet_322x433",
            "../Enemies/Rattack_spritesheet_634x431",
            "../Enemies/Rat_Hurt_spritesheet_515x428"
        ));

        // Gorilla (BOSS)
        enemyQueue.Add(new EnemyData(
            EnemyIconType.Gorilla,
            "WASDWASD",  // 8-key combo for boss!
            50f,
            25f,
            40f,
            "../Enemies/Gorilla_Idle_spritesheet_400x400",
            "../Enemies/Gorilla_Walk_spritesheet_400x400",
            "../Enemies/Gorilla_Attack_spritesheet_400x400",
            "../Enemies/Gorilla_Hurt_spritesheet_400x400"
        ));

        Console.WriteLine($"[EnemySpawner] Queue built with {enemyQueue.Count} enemies");
    }

    // =========================
    // Enemy Management
    // =========================
    private void StartNextEnemy()
    {
        if (currentEnemyIndex >= enemyQueue.Count)
        {
            Console.WriteLine("[EnemySpawner] ==========================================");
            Console.WriteLine("[EnemySpawner] 🎉 ALL ENEMIES DEFEATED! LEVEL COMPLETE!");
            Console.WriteLine("[EnemySpawner] ==========================================");
            OnLevelComplete();
            return;
        }

        EnemyData current = enemyQueue[currentEnemyIndex];

        Console.WriteLine("[EnemySpawner] ==========================================");
        Console.WriteLine($"[EnemySpawner] Starting Enemy {currentEnemyIndex + 1}/{enemyQueue.Count}");
        Console.WriteLine($"[EnemySpawner] Type: {current.IconType}");
        Console.WriteLine($"[EnemySpawner] Weakness: {current.WeaknessCombo}");
        Console.WriteLine($"[EnemySpawner] HP: {current.Health}");
        Console.WriteLine("[EnemySpawner] ==========================================");

        // Configure the existing enemy entity with new stats
        if (EnemyRhythmController.Instance != null)
        {
            EnemyRhythmController.Instance.ConfigureEnemy(current);
        }
        else
        {
            Console.WriteLine("[EnemySpawner] ⚠️ WARNING: EnemyRhythmController not found!");
        }

        // Update the UI
        UpdateOrderSlipUI();
    }

    private void UpdateOrderSlipUI()
    {
        if (OrderSlipUIController.Instance == null)
        {
            Console.WriteLine("[EnemySpawner] ⚠️ WARNING: OrderSlipUIController not found!");
            return;
        }

        EnemyIconType currentType = EnemyIconType.None;
        EnemyIconType nextType = EnemyIconType.None;

        // Get current enemy type
        if (currentEnemyIndex < enemyQueue.Count)
        {
            currentType = enemyQueue[currentEnemyIndex].IconType;
        }

        // Get next enemy type
        if (currentEnemyIndex + 1 < enemyQueue.Count)
        {
            nextType = enemyQueue[currentEnemyIndex + 1].IconType;
        }

        // Initialize UI if this is the first call
        if (currentEnemyIndex == 0)
        {
            OrderSlipUIController.Instance.Init(currentType, nextType);
            Console.WriteLine($"[EnemySpawner] UI Initialized: Current={currentType}, Next={nextType}");
        }
        else
        {
            // Advance the UI queue
            OrderSlipUIController.Instance.AdvanceQueue(nextType);
            Console.WriteLine($"[EnemySpawner] UI Advanced: Current={currentType}, Next={nextType}");
        }
    }

    // =========================
    // Public API - Called by EnemyRhythmController
    // =========================
    public void OnEnemyDefeated()
    {
        Console.WriteLine($"[EnemySpawner] Enemy {currentEnemyIndex + 1} defeated!");

        // Move to next enemy
        currentEnemyIndex++;

        // Small delay before starting next enemy (optional)
        // For now, start immediately
        StartNextEnemy();
    }

    // =========================
    // Level Completion
    // =========================
    private void OnLevelComplete()
    {
        // TODO: Load victory scene or show victory UI
        Console.WriteLine("[EnemySpawner] Loading victory scene...");
        // Scene.LoadScene("VictoryScene");
    }

    // =========================
    // Utility
    // =========================
    public int GetCurrentEnemyIndex()
    {
        return currentEnemyIndex;
    }

    public int GetTotalEnemyCount()
    {
        return enemyQueue.Count;
    }

    public EnemyData GetCurrentEnemy()
    {
        if (currentEnemyIndex < enemyQueue.Count)
            return enemyQueue[currentEnemyIndex];
        return null;
    }
}