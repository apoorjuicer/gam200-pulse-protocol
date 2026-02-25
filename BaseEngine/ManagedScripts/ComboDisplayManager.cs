using System;
using System.Collections.Generic;
using ScriptAPI;

public class ComboDisplayManager : Script
{
    // =========================
    // Global Access (Singleton)
    // =========================
    public static ComboDisplayManager Instance { get; private set; }

    // =========================
    // Configuration
    // =========================
    public bool DebugLogging = false;

    // can play with these values to get the right feel
    public float FadeOutDuration = 0.3f;    // How long each key takes to fade out
    public float FadeStaggerDelay = 0.1f;   // Delay between each key starting its fade

    // =========================
    // Internal State
    // =========================
    private bool initialized = false;
    private int lastComboProgress = 0;

    // =========================
    // Slot Registries
    // =========================
    private static Dictionary<int, ComboKeySlot> enemySlots = new Dictionary<int, ComboKeySlot>();
    private static Dictionary<int, ComboKeySlot> playerSlots = new Dictionary<int, ComboKeySlot>();
    private static ComboJudgementDisplay judgementScript = null;

    // =========================
    // Beat Reveal State
    // =========================
    private bool isRevealing = false;
    private string revealCombo = "";
    private int revealIndex = 0;
    private int lastRevealBeat = -1;

    // =========================
    // Stagger Fade State
    // =========================
    private bool isFadingOut = false;
    private int fadeOutCount = 0;
    private float fadeStaggerTimer = 0f;

    // =========================
    // Registration
    // =========================
    public static void RegisterKeySlot(int slotIndex, ComboKeySlot script)
    {
        if (script.GetIsEnemySlot())
        {
            enemySlots[slotIndex] = script;
            Console.WriteLine($"[ComboDisplayManager] Registered EnemySlot {slotIndex}");
        }
        else
        {
            playerSlots[slotIndex] = script;
            Console.WriteLine($"[ComboDisplayManager] Registered PlayerSlot {slotIndex}");
        }
    }

    public static void UnregisterKeySlot(int slotIndex, bool isEnemySlot)
    {
        if (isEnemySlot)
        {
            if (enemySlots.ContainsKey(slotIndex))
                enemySlots.Remove(slotIndex);
        }
        else
        {
            if (playerSlots.ContainsKey(slotIndex))
                playerSlots.Remove(slotIndex);
        }
    }

    public static void RegisterJudgement(ComboJudgementDisplay script)
    {
        judgementScript = script;
        Console.WriteLine("[ComboDisplayManager] Registered Judgement Display");
    }

    public static void UnregisterJudgement()
    {
        judgementScript = null;
    }

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

        // Beat reveal during EnemyDisplay phase
        if (isRevealing)
        {
            HandleBeatReveal();
            return;
        }

        // Stagger fade out after reveal completes, before player input
        if (isFadingOut)
        {
            HandleStaggerFade();
            return;
        }

        // Player input phase: show keys on bottom row as they're pressed
        if (ComboSystem.Instance == null)
            return;

        int currentProgress = ComboSystem.Instance.GetComboProgress();

        if (currentProgress > lastComboProgress)
        {
            string combo = ComboSystem.Instance.GetCurrentComboString();

            if (combo.Length > 0)
            {
                char lastKey = combo[combo.Length - 1];
                int slotIndex = currentProgress - 1;
                ShowPlayerKey(lastKey, slotIndex);
            }
        }
        else if (currentProgress < lastComboProgress)
        {
            ClearPlayerSlots();
        }

        lastComboProgress = currentProgress;
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        Instance = this;
        lastComboProgress = 0;
        isRevealing = false;
        isFadingOut = false;

        // Always clear uif not stale static refs from previous scene load will break
        // registration since all IDs get refreshed on reload
        enemySlots.Clear();
        playerSlots.Clear();
        judgementScript = null;

        Console.WriteLine("[ComboDisplayManager] ========================================");
        Console.WriteLine("[ComboDisplayManager] Manager Initialized");
        Console.WriteLine("[ComboDisplayManager] Waiting for child scripts to register...");
        Console.WriteLine("[ComboDisplayManager] ========================================");

        initialized = true;
    }

    // =========================
    // Beat Reveal
    // =========================
    public void StartComboReveal(string weakness)
    {
        revealCombo = weakness;
        revealIndex = 0;
        isRevealing = true;
        isFadingOut = false;
        lastRevealBeat = Conductor.Instance != null ? Conductor.Instance.GetCurrentBeat() : -1;

        ClearEnemySlotsInstant();

        Console.WriteLine($"[ComboDisplayManager] Starting beat reveal for: {weakness}");
    }

    public void StopComboReveal()
    {
        isRevealing = false;

        // Kick off stagger fade on enemy slots if any were revealed
        if (revealIndex > 0)
        {
            isFadingOut = true;
            fadeOutCount = 0;
            fadeStaggerTimer = 0f;
            Console.WriteLine("[ComboDisplayManager] Starting stagger fade out");
        }
    }

    private void HandleBeatReveal()
    {
        if (Conductor.Instance == null) return;

        int currentBeat = Conductor.Instance.GetCurrentBeat();
        if (currentBeat <= lastRevealBeat) return;

        lastRevealBeat = currentBeat;

        if (revealIndex < revealCombo.Length)
        {
            char key = revealCombo[revealIndex];

            if (enemySlots.ContainsKey(revealIndex))
            {
                enemySlots[revealIndex].ShowKeyPersistent(key);
                Console.WriteLine($"[ComboDisplayManager] Reveal beat {currentBeat}: enemy slot {revealIndex} = '{key}'");
            }

            revealIndex++;
        }

        // All keys revealed; wait for TurnManager to call StopComboReveal()
        if (revealIndex >= revealCombo.Length)
        {
            isRevealing = false;
            Console.WriteLine("[ComboDisplayManager] Beat reveal complete");
        }
    }

    // =========================
    // Stagger Fade
    // =========================
    private void HandleStaggerFade()
    {
        // All enemy slots have been kicked off, fade sequence done
        if (fadeOutCount >= revealCombo.Length)
        {
            isFadingOut = false;
            Console.WriteLine("[ComboDisplayManager] Stagger fade complete");
            return;
        }

        fadeStaggerTimer += Time.DeltaTime;

        if (fadeStaggerTimer >= FadeStaggerDelay)
        {
            fadeStaggerTimer = 0f;

            if (enemySlots.ContainsKey(fadeOutCount))
            {
                enemySlots[fadeOutCount].FadeOutAndHide(FadeOutDuration);
                Console.WriteLine($"[ComboDisplayManager] Fading out enemy slot {fadeOutCount}");
            }

            fadeOutCount++;
        }
    }

    // =========================
    // Display Control
    // =========================

    // Shows key on player bottom row, used during player input phase
    private void ShowPlayerKey(char key, int slotIndex)
    {
        if (slotIndex < 0 || slotIndex > 3)
        {
            Console.WriteLine($"[ComboDisplayManager] ⚠️ Invalid slot index: {slotIndex}");
            return;
        }

        if (playerSlots.ContainsKey(slotIndex))
        {
            playerSlots[slotIndex].ShowKey(key);
            if (DebugLogging)
                Console.WriteLine($"[ComboDisplayManager] Player slot {slotIndex} showing '{key}'");
        }
        else
        {
            Console.WriteLine($"[ComboDisplayManager] ⚠️ PlayerSlot {slotIndex} not registered!");
        }
    }

    public void ShowJudgement(string judgement)
    {
        if (judgementScript != null)
            judgementScript.ShowJudgement(judgement);
        else
            Console.WriteLine("[ComboDisplayManager] ⚠️ Judgement display not registered!");
    }

    // Clears player bottom row
    private void ClearPlayerSlots()
    {
        if (DebugLogging)
            Console.WriteLine("[ComboDisplayManager] Clearing player slots");

        for (int i = 0; i < 4; i++)
        {
            if (playerSlots.ContainsKey(i))
                playerSlots[i].Hide();
        }
    }

    // Instantly hides all enemy top row slots, used at start of reveal
    private void ClearEnemySlotsInstant()
    {
        for (int i = 0; i < 4; i++)
        {
            if (enemySlots.ContainsKey(i))
                enemySlots[i].HideInstant();
        }
    }

    // =========================
    // Helper Funcs
    // =========================
    public static bool IsSlotRegistered(int slotIndex, ComboKeySlot slot)
    {
        if (slot.GetIsEnemySlot())
            return enemySlots.ContainsKey(slotIndex) && enemySlots[slotIndex] == slot;
        else
            return playerSlots.ContainsKey(slotIndex) && playerSlots[slotIndex] == slot;
    }

    public static bool IsJudgementRegistered(ComboJudgementDisplay script)
    {
        return judgementScript == script;
    }

    // =========================
    // Cleanup
    // =========================
    public void OnDestroy()
    {
        if (Instance == this)
        {
            Instance = null;
            enemySlots.Clear();
            playerSlots.Clear();
            judgementScript = null;
        }
    }
}