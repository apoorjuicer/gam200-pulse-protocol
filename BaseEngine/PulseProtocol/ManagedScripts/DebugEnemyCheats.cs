using System;
using ScriptAPI;

public class DebugEnemyCheats : Script
{
    public int KillKey = 75; 

    public override void Update()
    {
        try
        {
            InputComponent inp = GetInput();

            if (inp.IsKeyTriggered(KillKey))
            {
                if (EnemyRhythmController.Instance == null)
                {
                    Console.WriteLine("[Cheat] No active enemy to kill.");
                    return;
                }

                Console.WriteLine("[Cheat] Forcing enemy death...");
                EnemyRhythmController.Instance!.TakeDamage(999999f);
            }
        }
        catch (Exception ex)
        {
            
            Console.WriteLine($"[Cheat] Exception: {ex.Message}");
        }
    }
}