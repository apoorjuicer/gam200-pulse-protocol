using System;
using ScriptAPI;

/// <summary>
/// DIAGNOSTIC SCRIPT - Attach to any entity to see what singletons exist
/// </summary>
public class DiagnosticCheck : Script
{
    private bool hasRun = false;

    public override void Update()
    {
        if (hasRun) return;
        hasRun = true;

        Console.WriteLine("========================================");
        Console.WriteLine("DIAGNOSTIC CHECK - Singleton Status");
        Console.WriteLine("========================================");

        Console.WriteLine($"Conductor.Instance: {(Conductor.Instance != null ? "EXISTS" : "NULL")}");
        Console.WriteLine($"TurnManager.Instance: {(TurnManager.Instance != null ? "EXISTS" : "NULL")}");
        Console.WriteLine($"ComboSystem.Instance: {(ComboSystem.Instance != null ? "EXISTS" : "NULL")}");
        Console.WriteLine($"EnemyRhythmController.Instance: {(EnemyRhythmController.Instance != null ? "EXISTS" : "NULL")}");
        Console.WriteLine($"OrderSlipUIController.Instance: {(OrderSlipUIController.Instance != null ? "EXISTS" : "NULL")}");
        Console.WriteLine($"EnemySpawner.Instance: {(EnemySpawner.Instance != null ? "EXISTS" : "NULL")}");

        Console.WriteLine("========================================");

        if (EnemySpawner.Instance == null)
        {
            Console.WriteLine("ERROR: EnemySpawner.Instance is NULL!");
            Console.WriteLine("  This means:");
            Console.WriteLine("  1. EnemySpawner script is not attached to any entity, OR");
            Console.WriteLine("  2. You attached the OLD version without the singleton, OR");
            Console.WriteLine("  3. The EnemySpawner entity isn't active");
        }

        if (ComboSystem.Instance != null && InputBridge.Instance == null)
        {
            Console.WriteLine("WARNING: ComboSystem exists but InputBridge doesn't!");
            Console.WriteLine("  Make sure InputBridge script is attached to an entity");
        }

        Console.WriteLine("========================================");
    }
}
