/******************************************************************************/
/**
* @file        ComboDisplayManager.cs
* @project     Pulse Protocol
* @author      Carrie Lam (70%)
*              Chloe Lau (30%)
* @brief       Manages beat-synced reveal, staggered fade-out, and player input feedback
*              for combo key slot UI during enemy encounters. Updated to keep player
*              slots dimly visible during enemy turn instead of hiding them, with smooth fade
*              transitions between turn phases and per-slot question mark scale pulse support.
*
* @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/

using System;
using System.Collections.Generic;
using ScriptAPI;

public class ComboDisplayManager : Script
{
    public static ComboDisplayManager? Instance { get; private set; }

    public bool DebugLogging = false;
    public float FadeOutDuration = 0.1f; // making this faster to counter INPUT NOW showing on MEMORIZE enemy last key 
    public float FadeStaggerDelay = 0.05f;

    private bool initialized = false;
    private int lastInputProgress = 0;

    private static Dictionary<int, ComboKeySlot> enemySlots = new Dictionary<int, ComboKeySlot>();
    private static Dictionary<int, ComboKeySlot> playerSlots = new Dictionary<int, ComboKeySlot>();
    private static ComboJudgementDisplay? judgementScript = null;

    private bool isRevealing = false;
    private string revealCombo = "";
    private int revealIndex = 0;
    private float revealStartTime = 0f;
    private float[] revealSlotOffsets = new float[] { 0.0f, 0.5f, 1.0f, 1.5f };
    private int[] currentVisualSlots = new int[] { 0, 1, 2, 3 };

    // Maps key index → UI slot position (allows visual gaps for rhythm clarity)
    private int VisualSlot(int keyIndex)
    {
        if (keyIndex < currentVisualSlots.Length) return currentVisualSlots[keyIndex];
        return keyIndex;
    }

    private bool pendingCountdownStart = false;
    private int countdownCurrentSlot = 0;
    private bool isCountingDown = false;

    // time-based slot offset tracking
    private float countdownStartTime = 0f;
    private float[] currentSlotOffsets = new float[] { 0.0f, 0.5f, 1.0f, 1.5f };
    private const float ScaleAnimatedTime = 0.15f;

    private bool isFadingOut = false;
    private int fadeOutCount = 0;
    private float fadeStaggerTimer = 0f;

    private string currentWeakness = "";

    // =========================
    // Registration
    // =========================
    public static void RegisterKeySlot(int slotIndex, ComboKeySlot script)
    {
        if (script.GetIsEnemySlot())
        {
            enemySlots[slotIndex] = script;
            // Console.WriteLine($"[ComboDisplayManager] Registered EnemySlot {slotIndex}");
        }
        else
        {
            playerSlots[slotIndex] = script;
            // Console.WriteLine($"[ComboDisplayManager] Registered PlayerSlot {slotIndex}");
        }
    }

    public static void UnregisterKeySlot(int slotIndex, bool isEnemySlot)
    {
        if (isEnemySlot) { if (enemySlots.ContainsKey(slotIndex)) enemySlots.Remove(slotIndex); }
        else             { if (playerSlots.ContainsKey(slotIndex)) playerSlots.Remove(slotIndex); }
    }

    public static void RegisterJudgement(ComboJudgementDisplay script)
    {
        judgementScript = script;
        // Console.WriteLine("[ComboDisplayManager] Registered Judgement Display");
    }

    public static void UnregisterJudgement() { judgementScript = null; }

    // =========================
    // Update
    // =========================
    public override void Update()
    {
        if (!initialized) { Initialize(); return; }

        if (isRevealing) { HandleBeatReveal(); }

        if (isFadingOut) HandleStaggerFade();

        if (pendingCountdownStart)
        {
            pendingCountdownStart = false;
            isCountingDown = true;
            countdownStartTime = Conductor.Instance != null ? Conductor.Instance.GetAdjustedSongTime() : 0f;
            currentSlotOffsets = EnemyRhythmController.Instance?.SlotOffsets ?? new float[] { 0.0f, 0.5f, 1.0f, 1.5f };

            if (playerSlots.ContainsKey(0) && currentWeakness.Length > 0)
                playerSlots[0].StartCountdown(currentWeakness[0]);
            countdownCurrentSlot = 1;
        }

        if (isCountingDown && Conductor.Instance != null) {
            float elapsed = Conductor.Instance.GetAdjustedSongTime() - countdownStartTime;

            while (countdownCurrentSlot < currentWeakness.Length && countdownCurrentSlot < currentSlotOffsets.Length) {
                float targetTime = currentSlotOffsets[countdownCurrentSlot] - ScaleAnimatedTime;
                if (elapsed < targetTime) break;

                int alreadyPressed = ComboSystem.Instance != null ? ComboSystem.Instance.GetComboProgress() : 0;
                if (countdownCurrentSlot >= alreadyPressed && playerSlots.ContainsKey(countdownCurrentSlot))
                    playerSlots[countdownCurrentSlot].StartCountdown(currentWeakness[countdownCurrentSlot]);

                countdownCurrentSlot++;
            }

            if (countdownCurrentSlot >= currentWeakness.Length)
                isCountingDown = false;
        }

        // if (isCountingDown && Conductor.Instance != null)
        // {
        //     float songTime = Conductor.Instance.GetAdjustedSongTime();
        //     float beatDuration = Conductor.Instance.GetBeatDuration();
        //     float timeSinceBeat = (songTime % beatDuration + beatDuration) % beatDuration;
        //     float timeUntilNextBeat = beatDuration - timeSinceBeat;

        //     // Trigger next slot when we're ScaleAnimDuration away from the next beat
        //     if (timeUntilNextBeat <= 0.25f && countdownCurrentSlot < currentWeakness.Length)
        //     {
        //         int currentBeat = Conductor.Instance.GetCurrentBeat();
        //         if (currentBeat > lastCountdownBeat)
        //         {
        //             lastCountdownBeat = currentBeat;
        //             // show me blank placeholder
        //             // if player presses wrong flash them (with sprite)
             
        //             int alreadyPressed = ComboSystem.Instance != null ? ComboSystem.Instance.GetComboProgress() : 0;
        //             if (countdownCurrentSlot >= alreadyPressed && playerSlots.ContainsKey(countdownCurrentSlot))
        //                 playerSlots[countdownCurrentSlot].StartCountdown(currentWeakness[countdownCurrentSlot]);
        //             countdownCurrentSlot++;

        //             if (countdownCurrentSlot >= currentWeakness.Length)
        //                 isCountingDown = false;
        //         }
        //     }
        // }

        if (ComboSystem.Instance == null) return;

        int currentProgress = ComboSystem.Instance!.GetComboProgress();

        if (currentProgress > lastInputProgress)
        {
            string combo = ComboSystem.Instance!.GetCurrentComboString();
            if (combo.Length > 0)
            {
                int slotIndex = currentProgress - 1;
                int vs = VisualSlot(slotIndex);
                char pressedKey = combo[combo.Length - 1];

                bool isCorrect = slotIndex < currentWeakness.Length &&
                                 pressedKey == currentWeakness[slotIndex];

                if (playerSlots.ContainsKey(vs))
                {
                    if (isCorrect)
                    {
                        playerSlots[vs].ShowCorrect(pressedKey);
                    }
                    else
                    {
                        char expected = slotIndex < currentWeakness.Length ? currentWeakness[slotIndex] : pressedKey;
                        playerSlots[vs].ShowWrong(pressedKey, expected);
                        VFXAPI.ShakeCamera(0.01f, 5.0f);
                    }
                }
            }
        }

        lastInputProgress = currentProgress;
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        Instance = this;
        lastInputProgress = 0;
        isRevealing = false;
        isFadingOut = false;
        currentWeakness = "";
        revealStartTime = 0f;
        revealSlotOffsets = new float[] { 0.0f, 0.5f, 1.0f, 1.5f };
        currentVisualSlots = new int[] { 0, 1, 2, 3 };

        pendingCountdownStart = false;
        countdownCurrentSlot = 0;
        isCountingDown = false;

        enemySlots.Clear();
        playerSlots.Clear();
        judgementScript = null;

        Console.WriteLine("[ComboDisplayManager] Initialized");
        initialized = true;
    }

    // =========================
    // Beat Reveal (Enemy Row)
    // =========================
    public void StartComboReveal(string weakness)
    {
        revealCombo = weakness;
        currentWeakness = weakness;
        revealIndex = 0;
        isRevealing = true;
        isFadingOut = false;

        // Snap to the nearest beat boundary using visual time (same clock as VisualBeatSystem)
        // so all keys align with the border pulse, not just the first one.
        if (Conductor.Instance != null)
        {
            float visualTime = Conductor.Instance.GetVisualAdjustedSongTime();
            float spb = Conductor.Instance.SecondsPerBeat;
            revealStartTime = (float)Math.Floor(visualTime / spb) * spb;
        }
        else
        {
            revealStartTime = 0f;
        }
        revealSlotOffsets = EnemyRhythmController.Instance?.SlotOffsets ?? new float[] { 0.0f, 0.5f, 1.0f, 1.5f };
        currentVisualSlots = EnemyRhythmController.Instance?.VisualSlotIndices ?? new int[] { 0, 1, 2, 3 };

        ClearEnemySlotsInstant();
        float beatDur = Conductor.Instance != null ? Conductor.Instance.SecondsPerBeat : 0.5f;

        // Fade out all player slots; active combo slots will cancel the fade when ShowBlankFadeToDim runs
        for (int i = 0; i < 4; i++)
            if (playerSlots.ContainsKey(i)) playerSlots[i].FadeToHidden(beatDur);

        for (int i = 0; i < weakness.Length && i < 4; i++)
        {
            int vs = VisualSlot(i);
            if (playerSlots.ContainsKey(vs))
                playerSlots[vs].ShowBlankFadeToDim(weakness[i], beatDur);
        }

        Console.WriteLine($"[ComboDisplayManager] Starting beat reveal for: {weakness}");
    }

    public void StopComboReveal()
    {
        isRevealing = false;
        lastInputProgress = 0;

        if (revealIndex > 0)
        {
            isFadingOut = true;
            fadeOutCount = 0;
            fadeStaggerTimer = 0f;
        }

        // countdown is now started early via StartPlayerCountdownEarly()
        pendingCountdownStart = false;
        countdownCurrentSlot = 0;
        isCountingDown = false;
    }

    private void HandleBeatReveal()
    {
        if (Conductor.Instance == null) return;

        // Use visual time (same clock as VisualBeatSystem) so keys stay in sync with border pulse
        float elapsed = Conductor.Instance.GetVisualAdjustedSongTime() - revealStartTime;

        while (revealIndex < revealCombo.Length && revealIndex < revealSlotOffsets.Length)
        {
            if (elapsed < revealSlotOffsets[revealIndex]) break;

            char key = revealCombo[revealIndex];
            int vs = VisualSlot(revealIndex);
            if (enemySlots.ContainsKey(vs))
            {
                enemySlots[vs].ShowKeyPop(key);
                Console.WriteLine($"[ComboDisplayManager] Reveal at t={elapsed:F3}s: enemy slot {vs} (key {revealIndex}) = '{key}'");
            }
            revealIndex++;
        }

        if (revealIndex >= revealCombo.Length)
        {
            isRevealing = false;
            Console.WriteLine("[ComboDisplayManager] Beat reveal complete");
        }
    }

    // =========================
    // Stagger Fade (Enemy Row)
    // =========================
    private void HandleStaggerFade()
    {
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
            int vs = VisualSlot(fadeOutCount);
            if (enemySlots.ContainsKey(vs))
            {
                enemySlots[vs].FadeOutAndHide(FadeOutDuration);
                Console.WriteLine($"[ComboDisplayManager] Fading out enemy slot {vs}");
            }
            fadeOutCount++;
        }
    }

    // =========================
    // Display Control
    // =========================
    public void ShowJudgement(string judgement)
    {
        if (judgementScript != null)
            judgementScript.ShowJudgement(judgement);
        else
            Console.WriteLine("[ComboDisplayManager] ⚠️ Judgement display not registered!");
    }

    private void ClearPlayerSlots()
    {
        if (DebugLogging) Console.WriteLine("[ComboDisplayManager] Clearing player slots");
        for (int i = 0; i < 4; i++)
        {
            if (playerSlots.ContainsKey(i))
                playerSlots[i].HideInstant();
        }
    }

    private void ShowDimmedPlayerSlots(string weakness)
    {
        for (int i = 0; i < weakness.Length && i < 4; i++)
        {
            if (playerSlots.ContainsKey(i))
                playerSlots[i].ShowBlankDim(weakness[i]);
        }
    }

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
    public static void TriggerPlayerSlotPulse(int slotIndex, float duration)
    {
        if (playerSlots.ContainsKey(slotIndex))
            playerSlots[slotIndex].TriggerScalePulse(duration);
    }

    public static bool IsSlotRegistered(int slotIndex, ComboKeySlot slot)
    {
        if (slot.GetIsEnemySlot())
            return enemySlots.ContainsKey(slotIndex) && enemySlots[slotIndex] == slot;
        else
            return playerSlots.ContainsKey(slotIndex) && playerSlots[slotIndex] == slot;
    }

    public static bool IsJudgementRegistered(ComboJudgementDisplay script) => judgementScript == script;

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

    public void StartPlayerCountdownEarly()
    {
        if (currentWeakness.Length == 0) return;

        currentSlotOffsets = EnemyRhythmController.Instance?.SlotOffsets ?? new float[] { 0.0f, 0.5f, 1.0f, 1.5f };
        countdownStartTime = Conductor.Instance != null ? Conductor.Instance.GetAdjustedSongTime() : 0f;

        // Fade out unused slots; active combo slots will cancel the fade via ShowBlankDim
        float fadeOutDur = Conductor.Instance != null ? Conductor.Instance.SecondsPerBeat * 0.5f : 0.25f;
        for (int i = 0; i < 4; i++)
            if (playerSlots.ContainsKey(i)) playerSlots[i].FadeToHidden(fadeOutDur);

        for (int i = 0; i < currentWeakness.Length; i++)
        {
            int vs = VisualSlot(i);
            if (playerSlots.ContainsKey(vs))
                playerSlots[vs].ShowBlankDim(currentWeakness[i]);
        }

        countdownCurrentSlot = currentWeakness.Length;
        isCountingDown = false;
        pendingCountdownStart = false;
        lastInputProgress = 0;
        
        Console.WriteLine("[ComboDisplayManager] All player slots revealed simultaneously");
    }

    public void FadeOutEnemySlots()
    {
        lastInputProgress = 0;
        if (revealIndex > 0)
        {
            isFadingOut = true;
            fadeOutCount = 0;
            fadeStaggerTimer = 0f;
        }
    }
}