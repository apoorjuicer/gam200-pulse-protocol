using System;
using System.Collections.Generic;
using ScriptAPI;

public class EnemySpawner : Script
{
    // =========================
    // Singleton
    // =========================
    public static EnemySpawner? Instance { get; private set; }

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
    private Queue<EnemyIconType> enemyQueue = new Queue<EnemyIconType>(); // public enum EnemyIconType { None, Rat, Bat, Gorilla }
    public static int enemyQueueID = 0;
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
    private bool EnemySpawnedAlr = false; // Stop spawning as soon as u spawn an enemy

    // For preloading assets
    public static bool LoadingComplete            = false;                        // To check if u finish preloading assets
    
    private List<string>         PendingTextures  = new List<string>();           // list of textures that will be preloaded.
    private int                  PreloadPos       = 0;                            // current pos for container.

    private Queue<EnemyIconType> PendingSpawn  = new Queue<EnemyIconType>();   // container for those that need 2 be created then made invise.(only for first time)
    public static Dictionary<EnemyIconType, int> HiddenPreloads = new Dictionary<EnemyIconType, int>(); //map of enemy that will be hiddne

    //Container of enemy 2 spawn. 
    // private List<EnemyIconType> EnemyContainer = new List<EnemyIconType>
    // {
    //     EnemyIconType.Rat,
    //     EnemyIconType.Bat,
    //     EnemyIconType.Gorilla,
    //     EnemyIconType.Rat
    // };

    private List<EnemyIconType> EnemyContainer = new List<EnemyIconType> 
    {
        EnemyIconType.Rat,
        EnemyIconType.Gorilla,
        EnemyIconType.Rat,
        EnemyIconType.Gorilla
    };

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            initialized = true;
            return;
        }

        // Load assets first.
        if (!LoadingComplete)
        {
            // keep preloading all the assets that need to be preloaded
            if (PreloadPos < PendingTextures.Count)
            {
                Animation.PreloadTexture(PendingTextures[PreloadPos++]);
            }
            
            // then now create all those that need to be spawned and hidden.
            else if (PendingSpawn.Count > 0)
            {
                EnemyIconType type = PendingSpawn.Dequeue();
                int id = Factory.Instantiate(EnumtoString(type));
                if (id >= 0)
                {
                    HiddenPreloads[type] = id;
                }
            }
            
            else
            {
                LoadingComplete = true;
            }

            return; 
        }

        // UI init after a few frames
        framesSinceStart++;
        if (!uiInitialized && framesSinceStart >= WAIT_FRAMES)
        {
            if (OrderSlipUIController.Instance != null)
            {
                if (OrderSlipUIController.Instance.SlotsReady()) // WAIT UR TURN
                {
                    InitializeOrderSlipUI();
                    uiInitialized = true;
                }
            }
        }

        if (waveComplete) return;

        // Stop when we’re done
        if (enemiesSpawned >= EnemiesToSpawn)
        {
            waveComplete = true;
            Console.WriteLine($"[EnemySpawner] Wave complete! Spawned {enemiesSpawned} enemies");
            return;
        }

        // This ensures that it will spawn in only one enemy at a time
        if (EnemySpawnedAlr)
        {
            // Wait until the new enemy finish setup of its EnemyRhythmController script
            if (EnemyRhythmController.Instance != null)
            {
                EnemySpawnedAlr = false;
                Console.WriteLine("New enemy spawned!!!\n\n\n");
            }
            return;
        }

        if (EnemyRhythmController.Instance != null)
        {
            return;
        }

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
        StageClearElement.ResetStageClear();

        // WHY IS ALL THIS HARD CODED WHEN U HAVE A CONTAINER ALR??????
        Console.WriteLine("[EnemySpawner] Initialized");
        Console.WriteLine($"  Will spawn {EnemiesToSpawn} enemies");
        Console.WriteLine($"  Prefab: {EnemyPrefabName}");
        Console.WriteLine($"  Spawn position: ({SpawnX}, {SpawnY})");

        SetupEnemyQueue();

        // Reset loading state for new lvl
        LoadingComplete = false;
        PreloadPos = 0;
        PendingTextures.Clear();
        PendingSpawn.Clear();
        HiddenPreloads.Clear();

        // What assets need to be loaded before lvl start, just copy and paste this and change 2 bat/boss/or smth[cheese]
        if (EnemyContainer.Contains(EnemyIconType.Gorilla))
        {
            PendingTextures.AddRange(Gorilla.GetRequiredTextures());
            PendingSpawn.Enqueue(EnemyIconType.Gorilla);
        }

        // nth new 2 load in, idt we will need but just in case
        if (PendingTextures.Count == 0)
        {
            LoadingComplete = true; 
        }

        pendingSpawn = SpawnImmediatelyOnStart;
    }

    private void SetupEnemyQueue()
    {
        enemyQueue.Clear();

        //// Ensure Current + Next always exist for UI
        //int totalIcons = Math.Max(EnemiesToSpawn + 2, 2);

        ////HARD CODED AGAIN????????????????????????
        //for (int i = 0; i < totalIcons; i++)
        //    enemyQueue.Enqueue(EnemyIconType.Rat);



        // use my container for the enemyQueue UI instead,cos the queue is a fking enum of int when factory takes in string
        foreach (EnemyIconType type in EnemyContainer)
        {
            enemyQueue.Enqueue(type);
        }
        EnemiesToSpawn = EnemyContainer.Count;

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
        // all enemies are dead. Yippeee
        if (enemyQueue.Count == 0)
        {
            //Scene.LoadScene("GameWin");
            StageClearElement.StageClear(); // bring up winner overlay
            return;
        }

        // Look at container and spawn wtv next
        EnemyIconType type = enemyQueue.Peek();
        string FactoryString = EnumtoString(type);

        // if it returns ""
        if (string.IsNullOrEmpty(FactoryString))
        {
            return;
        }

        // If it was one of the enemy that was preloaded at the start of the lvl, just make in visible.[cheese]
        if (HiddenPreloads.TryGetValue(type, out int pooledID))
        {
            HiddenPreloads.Remove(type); // releases the pool guard in EnemyRhythmController
            EnemySpawnedAlr = true;
            enemiesSpawned++;
            return;
        }

        int enemyID = Factory.Instantiate(FactoryString);

        if (enemyID < 0)
        {
            Console.WriteLine($"[EnemySpawner] Failed to spawn enemy! Prefab '{FactoryString}' not found");
            return;
        }

        EnemySpawnedAlr = true;
        enemiesSpawned++;
        Console.WriteLine($"[EnemySpawner] Spawned enemy {enemiesSpawned}/{EnemiesToSpawn} (ID: {enemyID})");
    }

    //[cheese]
    public string EnumtoString(EnemyIconType type)
    {
        switch (type)
        {
            case EnemyIconType.Rat: return "Rat_Enemy";
            case EnemyIconType.Bat: return "Rat_Enemy";     // temp, to replace after new enemies spritesheet has been added in
            case EnemyIconType.Gorilla: return "Gorilla_Enemy"; 
            default: return "";
        }
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