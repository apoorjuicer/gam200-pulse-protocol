using System;
using System.Collections.Generic;
using ScriptAPI;

public class EnemySpawner : Script
{
    // =========================
    // Config
    // =========================
    public float enemyDistanceFromPlayer = 8.0f;
    public bool spawnOnStart = true;
    public int totalEnemiesToWin = 5;

    // Prefab names (match engine prefab names)
    public string redEnemyPrefab = "RedEnemy";
    public string yellowEnemyPrefab = "YellowEnemy";
    public string blueEnemyPrefab = "BlueEnemy";

    // =========================
    // State
    // =========================
    private List<string> spawnSequence = new List<string>();
    private int enemiesDefeated = 0;
    private int currentEnemyId = -1;

    private bool waitingForGameStart = true;
    private bool waitingToSpawn = false;

    private float spawnDelayTimer = 0.0f;
    private const float SPAWN_DELAY = 2.0f;

    // =========================
    // Lifecycle
    // =========================
    public override void Update()
    {
        float dt = Time.DeltaTime;

        // Wait until game actually starts
        if (waitingForGameStart)
        {
            if (IsGamePlaying())
            {
                waitingForGameStart = false;
                BuildSpawnSequence();

                if (spawnOnStart)
                    SpawnNextEnemy();
            }
            return;
        }

        // Handle delayed spawn
        if (waitingToSpawn)
        {
            spawnDelayTimer -= dt;
            if (spawnDelayTimer <= 0.0f)
            {
                waitingToSpawn = false;
                SpawnNextEnemy();
            }
        }
    }

    // =========================
    // Spawn Logic
    // =========================
    private void BuildSpawnSequence()
    {
        spawnSequence.Clear();

        // 2 red, 2 yellow, 1 blue
        spawnSequence.Add(redEnemyPrefab);
        spawnSequence.Add(redEnemyPrefab);
        spawnSequence.Add(yellowEnemyPrefab);
        spawnSequence.Add(yellowEnemyPrefab);
        spawnSequence.Add(blueEnemyPrefab);
    }

    private void SpawnNextEnemy()
    {
        if (currentEnemyId != -1)
            return;

        int index = Math.Min(enemiesDefeated, spawnSequence.Count - 1);
        string prefabName = spawnSequence[index];

        currentEnemyId = Factory.Instantiate(prefabName);

        Console.WriteLine($"Spawned enemy #{enemiesDefeated + 1}: {prefabName}");
    }


    // =========================
    // Called externally when enemy dies
    // =========================
    public void NotifyEnemyDefeated()
    {
        currentEnemyId = -1;
        enemiesDefeated++;

        // Win condition
        if (enemiesDefeated >= totalEnemiesToWin)
        {
            Console.WriteLine("All enemies defeated!");
            SetGameWin();
            return;
        }

        // Delay before next spawn
        waitingToSpawn = true;
        spawnDelayTimer = SPAWN_DELAY;
    }

    // =========================
    // Game State Helpers
    // =========================
    private bool IsGamePlaying()
    {
        // Temporary assumption until script lookup exists
        return true;
    }

    private void SetGameWin()
    {
        // Temporary placeholder — GameStateManager will handle this later
        Scene.LoadScene("WinScene");
    }
}
