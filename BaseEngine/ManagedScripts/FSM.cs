using System;
using ScriptAPI;

public class TestFSM : ScriptAPI.Script
{
    private int frame = 0;

    public override void Update()
    {
        var fsm = GetFSM();
        var input = GetInput();

        // Print every 30 frames so Output isn't flooded
        if ((frame++ % 30) == 0)
        {
            Console.WriteLine($"[TestFSM] State: {fsm.GetState()}");
        }

        // Hold W to move
        if (input.IsKeyPressed(87)) // W
        {
            fsm.RequestMove(true);
        }
        else
        {
            fsm.RequestMove(false);
        }

        // Tap Space to attack
        if (input.IsKeyTriggered(32)) // Space
        {
            fsm.RequestAttack();
            Console.WriteLine("[TestFSM] RequestAttack()");
        }
    }
}
