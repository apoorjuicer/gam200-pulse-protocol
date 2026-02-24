using System;
using System.Collections.Generic;
using ScriptAPI;

public class EnemySpawner : Script
{
    // =========================
    // Singleton
    // =========================
    public static EnemySpawner Instance { get; private set; }

    // =========================
    // Spawn Settings
    // =========================
    public string EnemyPrefabName = "Rat_Enemy";
    public float SpawnX = 800f;
    public float SpawnY = 300f;

    public int EnemiesToSpawn = 3;
    public float SpawnInterval = 0.5f; // keep small for testing

    public bool SpawnImmediatelyOnStart = true;

    // =========================
    // UI / Queue
    // =========================
    private Queue<EnemyIconType> enemyQueue = new Queue<EnemyIconType>();
    private bool uiInitialized = false;
    private int framesSinceStart = 0;
    private const int WAIT_FRAMES = 5;

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private int enemiesSpawned = 0;
    private int enemiesDefeated = 0;
    private float timeSinceLastSpawn = 0f;
    private bool waveComplete = false;

    private bool pendingSpawn = false;

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            initialized = true;
            return;
        }

        // UI init after a few frames
        framesSinceStart++;
        if (!uiInitialized && framesSinceStart >= WAIT_FRAMES)
        {
            if (OrderSlipUIController.Instance != null)
            {
                InitializeOrderSlipUI();
                uiInitialized = true;
            }
        }

        if (waveComplete) return;

        // Stop when done
        if (enemiesSpawned >= EnemiesToSpawn)
        {
            waveComplete = true;
            Console.WriteLine($"[EnemySpawner] Wave complete! Spawned {enemiesSpawned} enemies");
            return;
        }

        // ONLY spawn when there is no active enemy
        if (EnemyRhythmController.Instance != null)
            return;

        timeSinceLastSpawn += Time.DeltaTime;

        bool intervalReady = (timeSinceLastSpawn >= SpawnInterval);
        if (pendingSpawn || intervalReady)
        {
            SpawnEnemy();
            pendingSpawn = false;
            timeSinceLastSpawn = 0f;
        }
    }

    private void Initialize()
    {
        if (Instance != null && Instance != this)
            Console.WriteLine("[EnemySpawner] WARNING: Multiple spawners detected - replacing Instance");

        Instance = this;

        Console.WriteLine("[EnemySpawner] Initialized");
        Console.WriteLine($"  Will spawn {EnemiesToSpawn} enemies");
        Console.WriteLine($"  Prefab: {EnemyPrefabName}");
        Console.WriteLine($"  Spawn position: ({SpawnX}, {SpawnY})");

        SetupEnemyQueue();

        pendingSpawn = SpawnImmediatelyOnStart;
    }

    private void SetupEnemyQueue()
    {
        enemyQueue.Clear();

        // Ensure Current + Next always exist for UI
        int totalIcons = Math.Max(EnemiesToSpawn + 2, 2);

        for (int i = 0; i < totalIcons; i++)
            enemyQueue.Enqueue(EnemyIconType.Rat);

        Console.WriteLine($"[EnemySpawner] Enemy queue setup with {enemyQueue.Count} enemies");
    }

    private void InitializeOrderSlipUI()
    {
        var ui = OrderSlipUIController.Instance;
        if (ui == null) return;

        if (enemyQueue.Count < 2) return;

        EnemyIconType current = PeekAtIndex(0);
        EnemyIconType next = PeekAtIndex(1);

        try
        {
            ui.Init(current, next);
            Console.WriteLine($"[EnemySpawner] UI init -> Current={current}, Next={next}");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[EnemySpawner] ERROR during UI Init: {ex.Message}");
        }
    }

    private void SpawnEnemy()
    {
        int enemyID = Factory.Instantiate(EnemyPrefabName);

        if (enemyID < 0)
        {
            Console.WriteLine($"[EnemySpawner] Failed to spawn enemy! Prefab '{EnemyPrefabName}' not found");
            return;
        }

        enemiesSpawned++;
        Console.WriteLine($"[EnemySpawner] Spawned enemy {enemiesSpawned}/{EnemiesToSpawn} (ID: {enemyID})");
    }

    public void OnEnemyDefeated()
    {
        enemiesDefeated++;
        Console.WriteLine($"[EnemySpawner] Enemy defeated! ({enemiesDefeated} total)");

        // Shift queue (remove current)
        if (enemyQueue.Count > 0)
            enemyQueue.Dequeue();

        // The newUpcoming is what should appear as "Next" after shifting
        EnemyIconType newUpcoming = EnemyIconType.None;
        if (enemyQueue.Count > 1)
            newUpcoming = PeekAtIndex(1);

        var ui = OrderSlipUIController.Instance;
        if (ui != null)
        {
            try
            {
                ui.AdvanceQueue(newUpcoming);
                Console.WriteLine($"[EnemySpawner] UI advanced -> Upcoming={newUpcoming}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[EnemySpawner] ERROR during AdvanceQueue: {ex.Message}");
            }
        }

        // Request the next enemy spawn (next Update, safe)
        pendingSpawn = true;
    }

    private EnemyIconType PeekAtIndex(int index)
    {
        if (index >= enemyQueue.Count)
            return EnemyIconType.None;

        EnemyIconType[] temp = enemyQueue.ToArray();
        return temp[index];
    }
}