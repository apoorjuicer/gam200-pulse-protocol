using System;
using System.Collections.Generic;
using ScriptAPI;

/// <summary>
/// NEW COMBO DISPLAY MANAGER
/// Attach this to the parent ComboDisplay entity
/// 
/// SETUP:
/// 1. Attach ComboKeySlot script to each KeySlot0-3 entity
/// 2. Attach ComboJudgementDisplay script to JudgementDisplay entity
/// 3. Attach THIS script to the parent ComboDisplay entity
/// 4. Set the entity IDs in the editor for this script
/// </summary>
public class ComboDisplayManager : Script
{
    // =========================
    // Global Access (Singleton)
    // =========================
    public static ComboDisplayManager Instance { get; private set; }

    // =========================
    // Configuration - Set in editor
    // =========================
    // Note: These need to be set manually in your scene editor
    public int KeySlot0_EntityID = 0;
    public int KeySlot1_EntityID = 0;
    public int KeySlot2_EntityID = 0;
    public int KeySlot3_EntityID = 0;
    public int JudgementDisplay_EntityID = 0;

    // =========================
    // Internal State
    // =========================
    private bool initialized = false;
    private int lastComboProgress = 0;

    // Cache references to child scripts
    // NOTE: We can't actually get these via entity ID with current API
    // So we'll use a workaround with static dictionaries
    private static Dictionary<int, ComboKeySlot> keySlotScripts = new Dictionary<int, ComboKeySlot>();
    private static ComboJudgementDisplay judgementScript = null;

    // =========================
    // Registration (Called by child scripts)
    // =========================
    public static void RegisterKeySlot(int slotIndex, ComboKeySlot script)
    {
        keySlotScripts[slotIndex] = script;
        Console.WriteLine($"[ComboDisplayManager] Registered KeySlot {slotIndex}");
    }

    public static void RegisterJudgement(ComboJudgementDisplay script)
    {
        judgementScript = script;
        Console.WriteLine("[ComboDisplayManager] Registered Judgement Display");
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
                    ShowKey(lastKey, currentProgress - 1);
                }
            }
            // Combo reset?
            else if (currentProgress < lastComboProgress)
            {
                ClearAllKeys();
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
            Console.WriteLine("[ComboDisplayManager] WARNING: Multiple instances!");
            return;
        }

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
        }
        else
        {
            Console.WriteLine($"[ComboDisplayManager] ⚠️ KeySlot {slotIndex} not registered yet!");
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
            Console.WriteLine("[ComboDisplayManager] ⚠️ Judgement display not registered yet!");
        }
    }

    private void ClearAllKeys()
    {
        Console.WriteLine("[ComboDisplayManager] 🧹 Clearing all keys");
        
        for (int i = 0; i < 4; i++)
        {
            if (keySlotScripts.ContainsKey(i))
            {
                keySlotScripts[i].Hide();
            }
        }
    }
}
