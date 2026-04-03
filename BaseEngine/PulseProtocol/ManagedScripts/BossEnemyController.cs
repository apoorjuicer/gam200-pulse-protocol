using System;
using ScriptAPI;

public class BossEnemyController : EnemyRhythmController
{
    // =========================
    // Boss Config
    // =========================
    public int Phase1MaxHP = 30;
    public int Phase2MaxHP = 40;

    // =========================
    // Internal Boss State
    // =========================
    private int _bossPhase = 1;   // 1 or 2
    public static int BossPhase { get; private set; } = 0; // 0 = inactive, 1 = phase 1, 2 = phase 2

    public static void ResetPhase() { BossPhase = 0; }
    private static readonly Random _bossRng = new Random();

    // Phase 1 combo pool mirrors exact values from existing enemies
    private struct ComboPattern
    {
        public string Combo;
        public float[] SlotOffsets;
        public int[]   VisualSlotIndices;
    }

    private static readonly ComboPattern[] Phase1Pool = new ComboPattern[]
    {
        new ComboPattern { Combo = "WASD",  SlotOffsets = new float[]{ 0.0f, 0.5f, 1.0f, 1.5f }, VisualSlotIndices = new int[]{ 0,1,2,3 } }, // Rat
        new ComboPattern { Combo = "WAS",   SlotOffsets = new float[]{ 0.0f, 1.0f, 1.5f },        VisualSlotIndices = new int[]{ 0,2,3 }   }, // Gorilla
        new ComboPattern { Combo = "DWS",   SlotOffsets = new float[]{ 0.0f, 1.0f, 1.5f },        VisualSlotIndices = new int[]{ 0,2,3 }   }, // BlueGorilla
        new ComboPattern { Combo = "SAWD",  SlotOffsets = new float[]{ 0.0f, 0.25f, 1.0f, 1.25f}, VisualSlotIndices = new int[]{ 0,1,2,3 } }, // Bat
    };

    private static readonly int[] Phase2Lengths = new int[] { 2, 3, 4, 6, 8 };
    private static readonly char[] WasdKeys = new char[] { 'W', 'A', 'S', 'D' };

    // =========================
    // Overrides
    // =========================
    protected override void SetupSprites()
    {
        // =====================================================================
        // TODO (Art): Replace gorilla placeholders with boss sprite sheets below.
        //
        // IdleSpriteSheet   = "boss_idle";      // rows=?, cols=?, frames=?
        // WalkSpriteSheet   = "boss_move1";     // or boss_move2 / boss_move3 for phase variety
        // AttackSpriteSheet = "boss_attack1";   // or boss_attack2 / boss_attack3
        // HurtSpriteSheet   = "boss_hurt";
        
        // PLACEHOLDER
        IdleSpriteSheet   = "gorilla_idle";
        WalkSpriteSheet   = "gorilla_walk";
        AttackSpriteSheet = "gorilla_attack";
        HurtSpriteSheet   = "gorilla_hurt";

        IdleFrames = 15; IdleColumns = 4; IdleRows = 4;
        WalkFrames = 30; WalkColumns = 6; WalkRows = 5;
        AttackFrames = 30; AttackColumns = 6; AttackRows = 5;
        HurtFrames = 15; HurtColumns = 4; HurtRows = 4;

        AnimationSpeed = 12f;
        SizeMultiplier = 5.0f;

        HealthBarOffsetX = 0f;
        HealthBarScaleOffsetXFactor = -0.24f;
        HealthBarOffsetY = -110f;
        HealthBarScaleOffsetYFactor = 0.76f;

        // Boss starts with Rat pattern but will be overwritten on first ResetMovementForNewTurn
        WeaknessCombo     = "WASD";
        SlotOffsets       = new float[] { 0.0f, 0.5f, 1.0f, 1.5f };
        VisualSlotIndices = new int[]   { 0, 1, 2, 3 };

        RandomizeWeakness    = false;
        RandomizeVisualSlots = false;

        _bossPhase = 1;
        BossPhase  = 1;
    }

    public override void TakeDamage(float dmg)
    {
        if (_bossPhase == 1)
        {
            if (isDead) return;

            // Let base apply damage to the health component
            var h = GetHealth();
            int prev = h.hp;
            h.hp -= (int)dmg;
            if (h.hp < 0) h.hp = 0;
            Console.WriteLine($"[Boss] Phase 1 took {dmg} damage! HP: {h.hp}/{prev}");

            // Capture VFX position from this entity's context
            var tf = GetTransform();
            HitVFX.SpawnX = tf.X;
            HitVFX.SpawnY = tf.Y;

            // Spawn damage indicator
            EnemyDamageIndicator.SpawnX = tf.X;
            EnemyDamageIndicator.SpawnY = tf.Y;
            EnemyDamageIndicator.DamageText = "-" + ((int)dmg).ToString();
            Factory.Instantiate("EnemyDamageIndicator");

            if (h.hp <= 0)
            {
                // Phase transition instead of death
                TransitionToPhase2();
            }
        }
        else
        {
            // Phase 2: normal death path via base (includes pendingDeath deferral)
            base.TakeDamage(dmg);
        }
    }

    private void TransitionToPhase2()
    {
        Console.WriteLine("[Boss] === PHASE TRANSITION: Phase 1 -> Phase 2 ===");
        _bossPhase = 2;
        BossPhase  = 2;

        // Restore HP for phase 2
        var h = GetHealth();
        h.hp    = Phase2MaxHP;
        MaxHP   = Phase2MaxHP;
        h.isAlive = true;

        // Visual feedback
        VFXAPI.ShakeCamera(0.12f, 15f);
        VFXAPI.BeatPulse();

        Console.WriteLine($"[Boss] Phase 2 started! HP restored to {Phase2MaxHP}");
    }

    public override void ResetMovementForNewTurn()
    {
        // Let base reset movement steps and state
        base.ResetMovementForNewTurn();

        if (_bossPhase == 1)
        {
            ApplyPhase1Pattern();
        }
        else
        {
            ApplyPhase2Pattern();
        }

        // Re-sync weakness with ComboSystem after pattern change
        if (ComboSystem.Instance != null)
            ComboSystem.Instance.SetEnemyWeakness(WeaknessCombo);
    }

    // =========================
    // Phase 1: random pool entry
    // =========================
    private void ApplyPhase1Pattern()
    {
        ComboPattern pick = Phase1Pool[_bossRng.Next(Phase1Pool.Length)];
        WeaknessCombo     = pick.Combo;
        SlotOffsets       = pick.SlotOffsets;
        VisualSlotIndices = pick.VisualSlotIndices;

        // Phase 1 uses normal beat counts clear any overrides from last turn
        TurnManager.EnemyDisplayBeatsOverride = null;
        TurnManager.PlayerInputBeatsOverride  = null;

        Console.WriteLine($"[Boss] Phase 1 pattern: {WeaknessCombo} slots=[{string.Join(",", VisualSlotIndices)}]");
    }

    // =========================
    // Phase 2: random combo generation
    // =========================
    private void ApplyPhase2Pattern()
    {
        float spb = Conductor.Instance != null ? Conductor.Instance.SecondsPerBeat : 0.5f;
        int comboLength = Phase2Lengths[_bossRng.Next(Phase2Lengths.Length)];

        // 1. Build SlotOffsets from predefined mixed-rhythm patterns.
        //    Short combos (2-4): uniform crotchet or quaver is fine.
        //    Long combos (6,8): must mix crotchets and quavers. NO all-quaver allowed.
        SlotOffsets = BuildOffsets(comboLength, spb);

        // 2. Build VisualSlotIndices: wrap at 4
        VisualSlotIndices = new int[comboLength];
        for (int i = 0; i < comboLength; i++)
            VisualSlotIndices[i] = i % 4;

        // 3. Random WASD keys
        char[] keys = new char[comboLength];
        for (int i = 0; i < comboLength; i++)
            keys[i] = WasdKeys[_bossRng.Next(WasdKeys.Length)];
        WeaknessCombo = new string(keys);

        // 4. Set TurnManager beat overrides so phases are long enough
        float lastOffset = SlotOffsets[comboLength - 1];
        // +1 matches normal enemies' 1-beat gap between last reveal key and slot-0 pulse.
        // +2 caused a 2-beat gap for 8-key combos because the reveal finishes 1 beat
        // before the display phase ends, then playerInputStartTime adds another beat.
        int displayBeats = Math.Max(4, (int)Math.Ceiling(lastOffset / spb) + 1);
        int inputBeats   = Math.Max(5, (int)Math.Ceiling(lastOffset / spb) + 3);
        TurnManager.EnemyDisplayBeatsOverride = displayBeats;
        TurnManager.PlayerInputBeatsOverride  = inputBeats;

        Console.WriteLine($"[Boss] Phase 2 pattern: {WeaknessCombo} length={comboLength} lastOffset={lastOffset:F2} displayBeats={displayBeats} inputBeats={inputBeats}");
    }

    // Returns the slot-offset array for a given combo length.
    // C = crotchet (one beat), Q = quaver (half beat).
    // Patterns for 6 and 8 keys are hand-crafted so they always mix both values.
    private float[] BuildOffsets(int length, float spb)
    {
        float C = spb;        // crotchet: one key per beat  (same pace as Rat)
        float Q = spb * 0.5f; // quaver: two keys per beat (same pace as Bat pairs)

        switch (length)
        {
            case 2:
            case 3:
            case 4:
            {
                // Short combos: uniform crotchet or quaver is fine either way
                float gap = (_bossRng.Next(2) == 0) ? C : Q;
                float[] o = new float[length];
                for (int i = 0; i < length; i++) o[i] = i * gap;
                return o;
            }

            case 6:
                // Four patterns — at most 2 quavers total
                switch (_bossRng.Next(4))
                {
                    case 0: // C C C C C C  (no quavers)
                        return new float[] { 0, C, 2*C, 3*C, 4*C, 5*C };
                    case 1: // Q Q C C C C  (2 quavers at start)
                        return new float[] { 0, Q, 2*Q, 2*Q+C, 2*Q+2*C, 2*Q+3*C };
                    case 2: // C C Q Q C C  (2 quavers in middle)
                        return new float[] { 0, C, 2*C, 2*C+Q, 2*C+2*Q, 3*C+2*Q };
                    default: // C C C C Q Q  (2 quavers at end)
                        return new float[] { 0, C, 2*C, 3*C, 4*C, 4*C+Q };
                }

            default: // 8 — always all crotchets (long enough already)
                return new float[] { 0, C, 2*C, 3*C, 4*C, 5*C, 6*C, 7*C };
        }
    }
}
