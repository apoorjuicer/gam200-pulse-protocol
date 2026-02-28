using System;
using ScriptAPI;

public enum PlayerAnimState { /*Idle1,*/ Idle2, Attack, Hurt }

public class PlayerAnimationController : Script
{
    // =========================
    // Singleton
    // =========================
    public static PlayerAnimationController? Instance { get; private set; }

    // =========================
    // Sprite Sheet Paths
    // =========================
    // public string Idle1SpriteSheet = "../Characters/Dorian_Idle1_579x615";
    public string Idle2SpriteSheet = "../Characters/Dorian_Idle2_579x615";
    public string AttackSpriteSheet = "../Characters/Dorian_Sword_579x615";
    public string HurtSpriteSheet = "../Characters/Dorian_Hurt_579x615";

    // =========================
    // Sprite Sheet Dimensions
    // =========================
    // public int Idle1Rows = 4;
    // public int Idle1Columns = 5;
    // public int Idle1Frames = 18;

    public int Idle2Rows = 4;
    public int Idle2Columns = 4;
    public int Idle2Frames = 15;

    public int AttackRows = 4;
    public int AttackColumns = 4;
    public int AttackFrames = 15;

    public int HurtRows = 4;
    public int HurtColumns = 4;
    public int HurtFrames = 15;

    public float AnimationSpeed = 30f;

    // =========================
    // Internal State
    // =========================
    private bool initialized = false;
    private PlayerAnimState currentState = PlayerAnimState.Idle2;

    // Determines which idle to return to after Attack/Hurt finishes
    // false = Idle2 (default, unarmed stance)
    // true  = Idle1 (armed stance, after attacking)
    // private bool hasAttacked = false;

    // Timer for non-idle states to auto-return to idle
    private float stateTimer = 0f;
    private float stateDuration = 0f;
    private bool waitingForStateEnd = false;
    private bool animPaused = false;

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            initialized = true;
            return;
        }

        if (Application.IsPaused())
            return;

        // Freeze animations during countdown; resume when GO! fires
        if (!GameStart.Ready)
        {
            if (!animPaused)
            {
                animPaused = true;
                Animation anim = GetAnimation();
                // if (currentState == PlayerAnimState.Idle1)
                //     anim.SetAnimation(Idle1SpriteSheet, Idle1Rows, Idle1Columns, Idle1Frames, 0f);
                // else
                    anim.SetAnimation(Idle2SpriteSheet, Idle2Rows, Idle2Columns, Idle2Frames, 0f);
            }
            return;
        }
        if (animPaused)
        {
            animPaused = false;
            currentState = PlayerAnimState.Attack; // bypass same-state guard in SetAnimState
            GoToIdle();
        }

        // Handle timed states (Attack, Hurt) returning to Idle
        if (waitingForStateEnd)
        {
            stateTimer += Time.DeltaTime;
            if (stateTimer >= stateDuration)
            {
                waitingForStateEnd = false;
                Console.WriteLine($"[PlayerAnim] {currentState} finished -> returning to idle");
                GoToIdle();
            }
        }
    }

    private void Initialize()
    {

        Instance = this;

        Console.WriteLine("[PlayerAnim] Initializing...");
        Console.WriteLine($"[PlayerAnim] Entity ID: {entityID}");

        // Start in Idle2 (default unarmed stance)
        // hasAttacked = false;
        animPaused = false;
        SetAnimState(PlayerAnimState.Idle2);

        Console.WriteLine("[PlayerAnim] Initialized - Idle2 (unarmed)");
    }

    // =========================
    // Idle Selection Logic
    // =========================
    // Idle1 = armed stance (sword drawn after first attack)
    // Idle2 = default / unarmed stance
    // Taking damage reverts to Idle2 (dropped guard)
    private void GoToIdle()
    {
        // if (hasAttacked)
        // {
        //     SetAnimState(PlayerAnimState.Idle1);
        //     Console.WriteLine("[PlayerAnim] -> Idle1 (armed stance)");
        // }
        // else
        // {
            SetAnimState(PlayerAnimState.Idle2);
            Console.WriteLine("[PlayerAnim] -> Idle2 (unarmed stance)");
        // }
    }

    // =========================
    // Public API
    // =========================

    public void PlayAttack()
    {
        // Don't interrupt an ongoing attack
        if (currentState == PlayerAnimState.Attack && waitingForStateEnd)
        {
            Console.WriteLine("[PlayerAnim] Already attacking - ignoring");
            return;
        }

        Console.WriteLine("[PlayerAnim] Playing Attack");
        // hasAttacked = true; // From now on, idle = Idle2
        SetAnimState(PlayerAnimState.Attack);

        waitingForStateEnd = true;
        stateTimer = 0f;
        stateDuration = AttackFrames / GetBeatSyncedSpeed(AttackFrames);
    }

    public void PlayHurt()
    {
        Console.WriteLine("[PlayerAnim] Playing Hurt");
        // hasAttacked = false; // Revert to Idle1 after hurt
        SetAnimState(PlayerAnimState.Hurt);

        waitingForStateEnd = true;
        stateTimer = 0f;
        stateDuration = HurtFrames / GetBeatSyncedSpeed(HurtFrames);
    }


    public void ForceIdle()
    {
        waitingForStateEnd = false;
        GoToIdle();
    }

    public PlayerAnimState GetCurrentState()
    {
        return currentState;
    }


    private float GetBeatSyncedSpeed(int frameCount)
    {
        if (Conductor.Instance == null || Conductor.Instance.SecondsPerBeat <= 0f)
            return AnimationSpeed;
        return frameCount / Conductor.Instance.SecondsPerBeat * 1.1065f;
    }

    // =========================
    // Internal: Set Animation
    // =========================
    private void SetAnimState(PlayerAnimState newState)
    {
        if (currentState == newState && !waitingForStateEnd)
            return;

        Console.WriteLine($"[PlayerAnim] State: {currentState} -> {newState}");
        currentState = newState;

        Animation anim = GetAnimation();

        switch (newState)
        {
            // case PlayerAnimState.Idle1:
            //     anim.SetAnimation(Idle1SpriteSheet, Idle1Rows, Idle1Columns, Idle1Frames, AnimationSpeed);
            //     break;

            case PlayerAnimState.Idle2:
                anim.SetAnimation(Idle2SpriteSheet, Idle2Rows, Idle2Columns, Idle2Frames, GetBeatSyncedSpeed(Idle2Frames));
                break;

            case PlayerAnimState.Attack:
                anim.SetAnimation(AttackSpriteSheet, AttackRows, AttackColumns, AttackFrames, GetBeatSyncedSpeed(AttackFrames));
                break;

            case PlayerAnimState.Hurt:
                anim.SetAnimation(HurtSpriteSheet, HurtRows, HurtColumns, HurtFrames, GetBeatSyncedSpeed(HurtFrames));
                break;
        }
    }
}
