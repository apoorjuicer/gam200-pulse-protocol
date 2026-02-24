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

    // =========================
    // Internal State
    // =========================
    private bool initialized = false;
    private int lastComboProgress = 0;

    private static Dictionary<int, ComboKeySlot> keySlotScripts = new Dictionary<int, ComboKeySlot>();
    private static ComboJudgementDisplay judgementScript = null;

    // =========================
    // Registration
    // =========================
    public static void RegisterKeySlot(int slotIndex, ComboKeySlot script)
    {
        keySlotScripts[slotIndex] = script;
        Console.WriteLine($"[ComboDisplayManager] Registered KeySlot {slotIndex}");
    }

    public static void UnregisterKeySlot(int slotIndex)
    {
        if (keySlotScripts.ContainsKey(slotIndex))
        {
            keySlotScripts.Remove(slotIndex);
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
                ShowKey(lastKey, slotIndex);
            }
        }
        else if (currentProgress < lastComboProgress)
        {
            ClearAllKeys();
        }

        lastComboProgress = currentProgress;
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        if (Instance != null && Instance != this)
        {
            Console.WriteLine("[ComboDisplayManager] Replacing stale singleton, clearing registrations");
            keySlotScripts.Clear();
            judgementScript = null;
        }

        Instance = this;
        lastComboProgress = 0;

        Console.WriteLine("[ComboDisplayManager] ========================================");
        Console.WriteLine("[ComboDisplayManager] Manager Initialized");
        Console.WriteLine("[ComboDisplayManager] Waiting for child scripts to register...");
        Console.WriteLine("[ComboDisplayManager] ========================================");

        initialized = true;
    }

    // =========================
    // Display Control
    // =========================
    public void ShowKey(char key, int slotIndex)
    {
        if (slotIndex < 0 || slotIndex > 3)
        {
            Console.WriteLine($"[ComboDisplayManager] ⚠️ Invalid slot index: {slotIndex}");
            return;
        }

        if (keySlotScripts.ContainsKey(slotIndex))
        {
            keySlotScripts[slotIndex].ShowKey(key);

            if (DebugLogging)
                Console.WriteLine($"[ComboDisplayManager] Showed '{key}' in slot {slotIndex}");
        }
        else
        {
            Console.WriteLine($"[ComboDisplayManager] ⚠️ KeySlot {slotIndex} not registered!");
        }
    }

    public void ShowJudgement(string judgement)
    {
        if (judgementScript != null)
        {
            judgementScript.ShowJudgement(judgement);
        }
        else
        {
            Console.WriteLine("[ComboDisplayManager] ⚠️ Judgement display not registered!");
        }
    }

    private void ClearAllKeys()
    {
        if (DebugLogging)
            Console.WriteLine("[ComboDisplayManager] Clearing all keys");

        for (int i = 0; i < 4; i++)
        {
            if (keySlotScripts.ContainsKey(i))
            {
                keySlotScripts[i].Hide();
            }
        }
    }

    public void OnDestroy()
    {
        if (Instance == this)
        {
            Instance = null;
            keySlotScripts.Clear();
            judgementScript = null;
        }
    }
}