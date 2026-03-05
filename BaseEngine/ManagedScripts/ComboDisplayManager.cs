    using System;
using System.Collections.Generic;
using ScriptAPI;

public class ComboDisplayManager : Script
{
    public static ComboDisplayManager? Instance { get; private set; }

    public bool DebugLogging = false;
    public float FadeOutDuration = 0.3f;
    public float FadeStaggerDelay = 0.1f;

    private bool initialized = false;
    private int lastInputProgress = 0;

    private static Dictionary<int, ComboKeySlot> enemySlots = new Dictionary<int, ComboKeySlot>();
    private static Dictionary<int, ComboKeySlot> playerSlots = new Dictionary<int, ComboKeySlot>();
    private static Dictionary<int, GlowOutlineScript> outlineSlots = new Dictionary<int, GlowOutlineScript>();
    private static ComboJudgementDisplay? judgementScript = null;

    private bool isRevealing = false;
    private string revealCombo = "";
    private int revealIndex = 0;
    private int lastRevealBeat = -1;

    private bool pendingCountdownStart = false;
    private int countdownCurrentSlot = 0;
    private int lastCountdownBeat = -1;
    private bool isCountingDown = false;

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

    public static void RegisterOutlineSlot(int slotIndex, GlowOutlineScript script)
    {
        outlineSlots[slotIndex] = script;
    }

    public static void TriggerOutlineGlow(int slotIndex)
    {
        if (outlineSlots.ContainsKey(slotIndex))
            outlineSlots[slotIndex].TriggerGlow();
    }

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
            lastCountdownBeat = Conductor.Instance != null ? Conductor.Instance.GetCurrentBeat() : -1;
            if (playerSlots.ContainsKey(0) && currentWeakness.Length > 0)
            {
                playerSlots[0].StartCountdown(currentWeakness[0]);
                TriggerOutlineGlow(0);
            }
            countdownCurrentSlot = 1;
        }

        if (isCountingDown && Conductor.Instance != null)
        {
            float songTime = Conductor.Instance.GetSongTime();
            float beatDuration = Conductor.Instance.GetBeatDuration();
            float timeSinceBeat = songTime % beatDuration;
            float timeUntilNextBeat = beatDuration - timeSinceBeat;

            // Trigger next slot when we're ScaleAnimDuration away from the next beat
            if (timeUntilNextBeat <= 0.25f && countdownCurrentSlot < currentWeakness.Length)
            {
                int currentBeat = Conductor.Instance.GetCurrentBeat();
                if (currentBeat > lastCountdownBeat)
                {
                    lastCountdownBeat = currentBeat;
                    if (playerSlots.ContainsKey(countdownCurrentSlot))
                    {
                        playerSlots[countdownCurrentSlot].StartCountdown(currentWeakness[countdownCurrentSlot]);
                        TriggerOutlineGlow(countdownCurrentSlot);
                    }
                    countdownCurrentSlot++;

                    if (countdownCurrentSlot >= currentWeakness.Length)
                        isCountingDown = false;
                }
            }
        }

        if (ComboSystem.Instance == null) return;

        int currentProgress = ComboSystem.Instance!.GetComboProgress();

        if (currentProgress > lastInputProgress)
        {
            string combo = ComboSystem.Instance!.GetCurrentComboString();
            if (combo.Length > 0)
            {
                int slotIndex = currentProgress - 1;
                char pressedKey = combo[combo.Length - 1];

                bool isCorrect = slotIndex < currentWeakness.Length &&
                                 pressedKey == currentWeakness[slotIndex];

                if (playerSlots.ContainsKey(slotIndex))
                {
                    if (isCorrect)
                    {
                        playerSlots[slotIndex].ShowCorrect(pressedKey);
                    }
                    else
                    {
                        char expected = slotIndex < currentWeakness.Length ? currentWeakness[slotIndex] : pressedKey;
                        playerSlots[slotIndex].ShowWrong(pressedKey, expected);
                        VFXAPI.ShakeCamera(0.01f, 5.0f);
                    }
                }
            }
        }
        else if (currentProgress < lastInputProgress)
        {
            ClearPlayerSlots();
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

        pendingCountdownStart = false;
        countdownCurrentSlot = 0;
        lastCountdownBeat = -1; 
        isCountingDown = false;

        enemySlots.Clear();
        playerSlots.Clear();    
        outlineSlots.Clear();
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
        lastRevealBeat = Conductor.Instance != null ? Conductor.Instance.GetCurrentBeat() : -1;

        ClearEnemySlotsInstant();
        ClearPlayerSlots();

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


        int currentBeat = Conductor.Instance!.GetCurrentBeat();

        Console.WriteLine($"[HandleBeatReveal] revealIndex={revealIndex}, beat={currentBeat}");
        if (revealIndex == 0)
        {
            lastRevealBeat = currentBeat;
            char key = revealCombo[0];
            if (enemySlots.ContainsKey(0))
            {
                enemySlots[0].ShowKeyPop(key);
                Console.WriteLine($"[ComboDisplayManager] Reveal immediate: enemy slot 0 = '{key}'");
            }
            revealIndex = 1;
            return;
        }

        if (currentBeat <= lastRevealBeat) return;
        lastRevealBeat = currentBeat;

        if (revealIndex < revealCombo.Length)
        {
            char key = revealCombo[revealIndex];
            if (enemySlots.ContainsKey(revealIndex))
            {
                enemySlots[revealIndex].ShowKeyPop(key);
                Console.WriteLine($"[ComboDisplayManager] Reveal beat {currentBeat}: enemy slot {revealIndex} = '{key}'");
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

            // Reset outline glow
            if (outlineSlots.ContainsKey(i))
                outlineSlots[i].ResetGlow();
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

        // isRevealing = false;

        if (playerSlots.ContainsKey(0))
        {
            playerSlots[0].StartCountdown(currentWeakness[0]);
            TriggerOutlineGlow(0);
        }

        countdownCurrentSlot = 1;
        isCountingDown = true;
        lastCountdownBeat = Conductor.Instance != null ? Conductor.Instance.GetCurrentBeat() : -1;
        pendingCountdownStart = false;
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