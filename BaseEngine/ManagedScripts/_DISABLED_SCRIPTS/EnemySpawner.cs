using System;
using System.Collections.Generic;
using ScriptAPI;

public class EnemySpawner : Script
{
    // === SPAWN SETTINGS ===
    public string EnemyPrefabName = "Rat_Enemy";
    public float SpawnX = 800f;
    public float SpawnY = 300f;

    // === WAVE SETTINGS ===
    public int EnemiesToSpawn = 3;
    public float SpawnInterval = 2f;

    // === UI INTEGRATION ===
    private Queue<EnemyIconType> enemyQueue = new Queue<EnemyIconType>();
    private bool uiInitialized = false;
    private int framesSinceStart = 0;
    private const int WAIT_FRAMES = 5; // Wait 5 frames before initializing UI

    // === INTERNAL STATE ===
    private bool initialized = false;
    private List<int> spawnedEnemies = new List<int>();
    private int enemiesSpawned = 0;
    private int enemiesDefeated = 0;
    private float timeSinceLastSpawn = 0f;
    private bool waveComplete = false;

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            initialized = true;
            return;
        }

        // Wait a few frames before trying to initialize UI
        // This gives the icon entities time to fully initialize
        framesSinceStart++;

        if (!uiInitialized && framesSinceStart >= WAIT_FRAMES)
        {
            if (OrderSlipUIController.Instance != null)
            {
                Console.WriteLine($"[EnemySpawner] Attempting UI initialization (frame {framesSinceStart})...");
                InitializeOrderSlipUI();
                uiInitialized = true;
            }
            else
            {
                Console.WriteLine($"[EnemySpawner] UI Controller not ready yet (frame {framesSinceStart})");
            }
        }

        if (waveComplete) return;

        // Spawn enemies over time
        if (enemiesSpawned < EnemiesToSpawn)
        {
            timeSinceLastSpawn += Time.DeltaTime;

            if (timeSinceLastSpawn >= SpawnInterval)
            {
                SpawnEnemy();
                timeSinceLastSpawn = 0f;
            }
        }
        else if (!waveComplete)
        {
            waveComplete = true;
            Console.WriteLine($"[EnemySpawner] Wave complete! Spawned {enemiesSpawned} enemies");
        }
    }

    private void Initialize()
    {
        Console.WriteLine("[EnemySpawner] Initialized");
        Console.WriteLine($"  Will spawn {EnemiesToSpawn} enemies");
        Console.WriteLine($"  Prefab: {EnemyPrefabName}");
        Console.WriteLine($"  Spawn position: ({SpawnX}, {SpawnY})");

        SetupEnemyQueue();
    }

    private void SetupEnemyQueue()
    {
        enemyQueue.Clear();

        // Setup enemy sequence
        enemyQueue.Enqueue(EnemyIconType.Rat);
        enemyQueue.Enqueue(EnemyIconType.Rat);
        enemyQueue.Enqueue(EnemyIconType.Rat);
        enemyQueue.Enqueue(EnemyIconType.Rat);
        enemyQueue.Enqueue(EnemyIconType.Rat);

        Console.WriteLine($"[EnemySpawner] Enemy queue setup with {enemyQueue.Count} enemies");
    }

    private void InitializeOrderSlipUI()
    {
        var ui = OrderSlipUIController.Instance;
        if (ui == null)
        {
            Console.WriteLine("[EnemySpawner] ERROR: UI Controller is null!");
            return;
        }

        if (enemyQueue.Count < 2)
        {
            Console.WriteLine("[EnemySpawner] ERROR: Not enough enemies in queue");
            return;
        }

        // Check if slots are ready
        if (!ui.SlotsReady())
        {
            Console.WriteLine("[EnemySpawner] WARNING: UI slots not registered yet - icon scripts may not be initialized");
            Console.WriteLine($"  Slots ready: {ui.SlotsReady()}");
            // Still try to initialize, it might work
        }

        EnemyIconType current = PeekAtIndex(0);
        EnemyIconType next = PeekAtIndex(1);

        Console.WriteLine($"[EnemySpawner] Initializing UI with Current={current}, Next={next}");

        try
        {
            ui.Init(current, next);
            Console.WriteLine("[EnemySpawner] UI initialization successful!");
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
            Console.WriteLine($"  Make sure {EnemyPrefabName}.json exists in PulseEngine/Assets/Prefabs/");
            return;
        }

        spawnedEnemies.Add(enemyID);
        enemiesSpawned++;

        Console.WriteLine($"[EnemySpawner] Spawned enemy {enemiesSpawned}/{EnemiesToSpawn} (ID: {enemyID})");
    }

    public void OnEnemyDefeated()
    {
        enemiesDefeated++;
        Console.WriteLine($"[EnemySpawner] Enemy defeated! ({enemiesDefeated} total)");

        var ui = OrderSlipUIController.Instance;
        if (ui != null && enemyQueue.Count > 2)
        {
            if (enemyQueue.Count > 0)
                enemyQueue.Dequeue();

            if (enemyQueue.Count > 1)
            {
                EnemyIconType newNext = PeekAtIndex(1);

                try
                {
                    ui.AdvanceQueue(newNext);
                    Console.WriteLine($"[EnemySpawner] UI advanced to show next enemy: {newNext}");
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[EnemySpawner] ERROR during AdvanceQueue: {ex.Message}");
                }
            }
        }
    }

    private EnemyIconType PeekAtIndex(int index)
    {
        if (index >= enemyQueue.Count)
            return EnemyIconType.None;

        EnemyIconType[] temp = enemyQueue.ToArray();
        return temp[index];
    }

    public void SpawnWave(int count)
    {
        EnemiesToSpawn = count;
        enemiesSpawned = 0;
        enemiesDefeated = 0;
        timeSinceLastSpawn = 0f;
        waveComplete = false;
        spawnedEnemies.Clear();

        Console.WriteLine($"[EnemySpawner] Starting new wave: {count} enemies");
    }

    public int GetSpawnedCount()
    {
        return spawnedEnemies.Count;
    }

    public int GetDefeatedCount()
    {
        return enemiesDefeated;
    }
}