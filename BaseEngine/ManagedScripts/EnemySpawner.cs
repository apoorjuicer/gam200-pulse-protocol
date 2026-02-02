using System;
using System.Collections.Generic;
using ScriptAPI;

public class EnemySpawner : Script
{
    // === SPAWN SETTINGS ===
    public string EnemyPrefabName = "Rat_Enemy";  // Name of prefab in Assets/Prefabs/ folder
    public float SpawnX = 800f;                    // Where enemies spawn (X position)
    public float SpawnY = 300f;                    // Where enemies spawn (Y position)

    // === WAVE SETTINGS ===
    public int EnemiesToSpawn = 3;
    public float SpawnInterval = 2f;               // Seconds between spawns

    // === INTERNAL STATE ===
    private bool initialized = false;
    private List<int> spawnedEnemies = new List<int>();
    private int enemiesSpawned = 0;
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
    }

    private void SpawnEnemy()
    {
        // Spawn enemy from prefab
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

    // === PUBLIC API ===
    public void SpawnWave(int count)
    {
        EnemiesToSpawn = count;
        enemiesSpawned = 0;
        timeSinceLastSpawn = 0f;
        waveComplete = false;
        spawnedEnemies.Clear();

        Console.WriteLine($"[EnemySpawner] Starting new wave: {count} enemies");
    }

    public int GetSpawnedCount()
    {
        return spawnedEnemies.Count;
    }
}