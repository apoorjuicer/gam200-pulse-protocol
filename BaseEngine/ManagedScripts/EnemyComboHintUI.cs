using System;
using ScriptAPI;

/// <summary>
/// UI that displays the key combo required to defeat the enemy
/// Follows above the enemy's head
/// Shows keys one by one during Enemy Display phase
/// 
/// SETUP:
/// - Attach this to a UI entity (invisible sprite or text)
/// - It will automatically position itself above the enemy
/// - Create 4 child entities for each key display (W, A, S, D indicators)
/// </summary>
public class EnemyComboHintUI : Script
{
    // =========================
    // Global Access
    // =========================
    public static EnemyComboHintUI Instance { get; private set; }

    // =========================
    // Config
    // =========================
    public float OffsetAboveEnemy = 150f;  // How far above enemy's head
    public float KeySpacing = 50f;  // Space between key indicators
    public float KeySize = 40f;  // Size of each key indicator

    // Animation
    public bool AnimateReveal = true;
    public float RevealScale = 1.5f;  // Scale effect when key is revealed
    public float RevealDuration = 0.3f;  // How long reveal animation lasts

    // =========================
    // State
    // =========================
    private bool initialized = false;
    private bool isVisible = false;
    private string currentCombo = "";
    private int keysRevealed = 0;
    private int lastBeat = -1;
    
    // Animation timers for each key (up to 4 keys)
    private float[] keyRevealTimers = new float[4];
    private bool[] keyIsRevealed = new bool[4];

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

        // Update position to follow enemy
        UpdatePosition();

        // Check current phase
        if (TurnManager.Instance == null || Conductor.Instance == null)
            return;

        TurnPhase currentPhase = TurnManager.Instance.GetCurrentPhase();
        int currentBeat = Conductor.Instance.GetCurrentBeat();

        // Show during Enemy Display phase
        if (currentPhase == TurnPhase.EnemyDisplay)
        {
            if (!isVisible)
            {
                ShowComboHint();
            }

            // Reveal keys on beat
            if (currentBeat != lastBeat)
            {
                lastBeat = currentBeat;
                RevealNextKey();
            }
        }
        else if (currentPhase == TurnPhase.PlayerInput)
        {
            // Keep showing during player input
            if (!isVisible)
            {
                ShowComboHint();
            }
        }
        else
        {
            if (isVisible)
            {
                HideComboHint();
            }
        }

        // Update animations
        if (AnimateReveal)
        {
            UpdateAnimations();
        }
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        if (Instance == null)
            Instance = this;

        Console.WriteLine("[EnemyComboHintUI] Initialized");
        Console.WriteLine($"[EnemyComboHintUI] Offset above enemy: {OffsetAboveEnemy}");
        Console.WriteLine($"[EnemyComboHintUI] Key spacing: {KeySpacing}");

        // Start hidden
        HideComboHint();

        initialized = true;
    }

    // =========================
    // Position Update
    // =========================
    private void UpdatePosition()
    {
        // Follow enemy position
        if (EnemyRhythmController.Instance == null)
            return;

        TransformComponent enemyTf = EnemyRhythmController.Instance.GetTransform();
        TransformComponent myTf = GetTransform();

        // Position above enemy's head
        myTf.X = enemyTf.X;
        myTf.Y = enemyTf.Y + OffsetAboveEnemy;
    }

    // =========================
    // Display Logic
    // =========================
    private void ShowComboHint()
    {
        isVisible = true;
        TransformComponent tf = GetTransform();
        tf.IsVisible = true;

        // Get current combo from enemy
        if (EnemyRhythmController.Instance != null)
        {
            currentCombo = EnemyRhythmController.Instance.WeaknessCombo;
            Console.WriteLine($"[EnemyComboHintUI] 👁 Showing combo hint: {currentCombo}");
        }

        // Reset reveal state
        keysRevealed = 0;
        for (int i = 0; i < 4; i++)
        {
            keyIsRevealed[i] = false;
            keyRevealTimers[i] = 0f;
        }
    }

    private void HideComboHint()
    {
        if (!isVisible) return;

        isVisible = false;
        TransformComponent tf = GetTransform();
        tf.IsVisible = false;

        Console.WriteLine("[EnemyComboHintUI] 🙈 Hiding combo hint");
    }

    private void RevealNextKey()
    {
        if (keysRevealed >= currentCombo.Length)
            return;

        int beatsInPhase = TurnManager.Instance.GetBeatsInCurrentPhase();

        if (beatsInPhase < currentCombo.Length && beatsInPhase >= keysRevealed)
        {
            char key = currentCombo[beatsInPhase];
            keyIsRevealed[beatsInPhase] = true;
            keyRevealTimers[beatsInPhase] = RevealDuration;
            keysRevealed++;

            Console.WriteLine($"[EnemyComboHintUI] ✨ Revealed key {beatsInPhase + 1}/{currentCombo.Length}: {key}");

            // TODO: Update visual representation
            // This is where you'd update the actual UI sprites/text
            // For now, we just log it
        }
    }

    private void UpdateAnimations()
    {
        float dt = Time.DeltaTime;

        for (int i = 0; i < 4; i++)
        {
            if (keyRevealTimers[i] > 0f)
            {
                keyRevealTimers[i] -= dt;

                // Calculate scale based on timer (eased)
                float progress = 1.0f - (keyRevealTimers[i] / RevealDuration);
                float scale = Lerp(RevealScale, 1.0f, EaseOutBack(progress));

                // TODO: Apply scale to key sprite/text
                // GetKeySprite(i).SetScale(KeySize * scale);
            }
        }
    }

    // =========================
    // Helper Functions
    // =========================
    private float Lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    private float EaseOutBack(float t)
    {
        float c1 = 1.70158f;
        float c3 = c1 + 1f;
        return 1f + c3 * (float)Math.Pow(t - 1f, 3f) + c1 * (float)Math.Pow(t - 1f, 2f);
    }

    // =========================
    // Visual Rendering (Console Version)
    // =========================
    private void RenderComboHintToConsole()
    {
        // This prints a visual representation to console
        // In a real game, this would render UI sprites

        if (!isVisible || string.IsNullOrEmpty(currentCombo))
            return;

        string display = "";
        for (int i = 0; i < currentCombo.Length; i++)
        {
            if (keyIsRevealed[i])
            {
                display += $"[{currentCombo[i]}] ";
            }
            else
            {
                display += "[?] ";
            }
        }

        // Only print if something changed
        Console.WriteLine($"[EnemyComboHintUI] Combo: {display.Trim()}");
    }

    // =========================
    // Public API
    // =========================
    public bool IsCurrentlyVisible()
    {
        return isVisible;
    }

    public int GetKeysRevealed()
    {
        return keysRevealed;
    }

    public string GetCurrentCombo()
    {
        return currentCombo;
    }

    // For debugging - forces display of all keys
    public void RevealAllKeys()
    {
        for (int i = 0; i < currentCombo.Length; i++)
        {
            keyIsRevealed[i] = true;
        }
        keysRevealed = currentCombo.Length;
        Console.WriteLine("[EnemyComboHintUI] 🔓 All keys revealed (debug)");
    }
}
