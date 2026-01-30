using System;
using ScriptAPI;

public enum EnemyState { Idle, Moving, Attacking, Dead }
public enum RhythmType { OneBeat, TwoBeat, FourBeat }

public class EnemyRhythmController : Script
{
    // =========================
    // Config
    // =========================
    public RhythmType rhythmType = RhythmType.OneBeat;
    public float stepDistance = 1.0f;
    public float attackDamage = 10.0f;
    public float health = 10.0f;

    // =========================
    // State
    // =========================
    private EnemyState currentState = EnemyState.Idle;
    private int beatCounter = 0;
    private bool canMove = true;
    private bool recentlyAttacked = false;

    private float attackCooldown = 0.0f;
    private const float ATTACK_COOLDOWN = 2.0f;

    // =========================
    // Update
    // =========================
    public override void Update()
    {
        float dt = Time.DeltaTime;

        if (currentState == EnemyState.Dead)
            return;

        if (recentlyAttacked)
        {
            attackCooldown -= dt;
            if (attackCooldown <= 0.0f)
                recentlyAttacked = false;
        }

        // TEMP: simple beat simulation
        beatCounter++;

        HandleBeat();
    }

    // =========================
    // Beat Logic
    // =========================
    private void HandleBeat()
    {
        if (!canMove)
            return;

        switch (rhythmType)
        {
            case RhythmType.OneBeat:
                Step();
                break;

            case RhythmType.TwoBeat:
                if (beatCounter % 2 == 0)
                    Step();
                break;

            case RhythmType.FourBeat:
                if (beatCounter % 4 == 0)
                    Step();
                break;
        }
    }

    // =========================
    // Movement
    // =========================
    private void Step()
    {
        SetState(EnemyState.Moving);

        TransformComponent tf = GetTransform();
        tf.X -= stepDistance;

        if (!recentlyAttacked)
            Attack();
    }

    // =========================
    // Combat
    // =========================
    private void Attack()
    {
        SetState(EnemyState.Attacking);

        HealthComponent hc = GetHealth();
        hc.hp -= (int)attackDamage;

        recentlyAttacked = true;
        attackCooldown = ATTACK_COOLDOWN;
    }

    public void TakeDamage(float dmg)
    {
        health -= dmg;
        if (health <= 0)
            Die();
    }

    private void Die()
    {
        SetState(EnemyState.Dead);
        Console.WriteLine("Enemy defeated!");
    }

    // =========================
    // State
    // =========================
    private void SetState(EnemyState newState)
    {
        currentState = newState;

        Animation anim = GetAnimation();
        if (newState == EnemyState.Moving)
            anim.SetAnimation("EnemyMove", 0, 4, 4, 1.0f);
        else if (newState == EnemyState.Attacking)
            anim.SetAnimation("EnemyAttack", 0, 3, 3, 1.0f);
        else if (newState == EnemyState.Dead)
            anim.SetAnimation("EnemyDie", 0, 2, 2, 1.0f);
    }
}

