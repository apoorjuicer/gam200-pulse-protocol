using System;
using ScriptAPI;

public class BossRhythmController : EnemyRhythmController
{
    // =========================
    // Phase 1 fixed patterns
    // =========================
    private static readonly string[] Phase1Patterns = { "WASD", "WAWA", "DASD" };
    private int phase1PatternIndex = 0;

    // =========================
    // Phase 2 state
    // =========================
    private bool isPhase2 = false;
    private float phase2HPThreshold = 0.5f; // transition at ≤50% HP
    private float maxHealth = 0f;           // cached once HP is known

    // =========================
    // DING cue
    // =========================
    private int dingBeat = -1;
    private static readonly System.Random rng = new System.Random();

    // =========================
    // Sprite setup
    // =========================
    protected override void SetupSprites()
    {
        // Reuse Gorilla sheets until boss-specific assets are available.
        IdleSpriteSheet   = "../Enemies/Gorilla_Idle_spritesheet_1459x977";
        WalkSpriteSheet   = "../Enemies/Gorilla_Walk_spritesheet_1459x977";
        AttackSpriteSheet = "../Enemies/Gorilla_Attack_spritesheet_1459x977";
        HurtSpriteSheet   = "../Enemies/Gorilla_Hurt_spritesheet_1459x977";

        IdleFrames = 15;   IdleColumns = 4;   IdleRows = 4;
        WalkFrames = 30;   WalkColumns = 6;   WalkRows = 5;
        AttackFrames = 30; AttackColumns = 6; AttackRows = 5;
        HurtFrames = 15;   HurtColumns = 4;   HurtRows = 4;

        AnimationSpeed = 12f;
        SizeMultiplier = 5.0f;

        // Boss is stationary — zero out movement so base HandleBeat steps are no-ops.
        StepDistance = 0f;
    }

    // =========================
    // Beat handler override — boss never moves, only fires DING cue
    // =========================
    protected override void HandleBeat(int beat)
    {
        if (TurnManager.Instance == null) return;

        TurnPhase phase = TurnManager.Instance.GetCurrentPhase();

        if (phase == TurnPhase.Countdown)
            return;

        // Cache max health on first non-countdown beat
        if (maxHealth <= 0f)
        {
            HealthComponent hc = GetHealth();
            if (hc.hp > 0) maxHealth = hc.hp;
        }

        CheckPhaseTransition();

        // Fire DING regardless of which phase TurnManager has already transitioned to —
        // on the DING beat TurnManager switches to PlayerInput before HandleBeat runs,
        // so checking EnemyDisplay here would always miss it.
        if (beat == dingBeat)
        {
            Console.WriteLine("[Boss] DING! Sequence complete — player input starting.");
            GetAudio().PlaySFX("SFX_Enemy_Ping");
        }
    }

    // =========================
    // Failure punishment — attack only, no movement
    // =========================
    public override void ExecuteFailurePunishment()
    {
        Console.WriteLine("[Boss] ExecuteFailurePunishment() — attacking player (no movement)");
        AttackPlayer();
    }

    // =========================
    // New-turn hook — build combo and configure TurnManager/ComboSystem
    // =========================
    public override void ResetMovementForNewTurn()
    {
        // Boss doesn't move, but call base to handle isDead guard, idle state,
        // and weakness-retry state reset.
        base.ResetMovementForNewTurn();

        BuildAndApplyWeakness();
    }

    // =========================
    // Combo building
    // =========================
    private void BuildAndApplyWeakness()
    {
        string combo = isPhase2 ? BuildPhase2Weakness() : BuildPhase1Weakness();

        // Configure variable-length systems before injecting the combo.
        ComboSystem.Instance?.SetComboLength(combo.Length);
        TurnManager.Instance?.SetEnemyDisplayBeats(combo.Length + 1); // +1 so all keys reveal before transition

        // Set the public field (used by TurnManager display log) and inject into ComboSystem.
        WeaknessCombo = combo;
        ComboSystem.Instance?.SetEnemyWeakness(combo);

        // DING fires on the same beat as the last key reveal (beat combo.Length of display).
        // PlayerInput opens on the next beat — player hears DING and starts on beat 1 of input.
        // Total turn = (combo.Length + 1) display beats + PlayerInputBeats input beats = 8 beats (2 bars).
        if (Conductor.Instance != null)
            dingBeat = Conductor.Instance.GetCurrentBeat() + combo.Length - 1;

        Console.WriteLine($"[Boss] Phase {(isPhase2 ? 2 : 1)} | Combo: {combo} | DING beat: {dingBeat}");
    }

    private string BuildPhase1Weakness()
    {
        string combo = Phase1Patterns[phase1PatternIndex % Phase1Patterns.Length];
        phase1PatternIndex++;
        return combo;
    }

    private string BuildPhase2Weakness()
    {
        int len = rng.Next(3, 7); // 3–6 inclusive (Next upper bound is exclusive)
        char[] keys = { 'W', 'A', 'S', 'D' };
        var sb = new System.Text.StringBuilder();
        for (int i = 0; i < len; i++)
            sb.Append(keys[rng.Next(0, 4)]);
        return sb.ToString();
    }

    // =========================
    // Phase transition check
    // =========================
    private void CheckPhaseTransition()
    {
        if (isPhase2 || maxHealth <= 0f) return;

        float ratio = GetHealth().hp / maxHealth;
        if (ratio <= phase2HPThreshold)
        {
            isPhase2 = true;
            Console.WriteLine("[Boss] === PHASE 2 ACTIVATED (≤50% HP)! ===");
        }
    }
}
