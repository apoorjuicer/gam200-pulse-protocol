using System;
using ScriptAPI;

public class TestFSM : ScriptAPI.Script
{
    public override void Update()
    {
        var fsm = GetFSM();
        var input = GetInput();

        // Print every 30 frames so Output isn't flooded
        //if ((frame++ % 30) == 0)
        //{
           Console.WriteLine($"[TestFSM] State: {fsm.GetState()}");
        //}

       
        // Tap Space to attack
        if (input.IsKeyTriggered(32)) // Space
        {
            fsm.RequestAttack();
            Console.WriteLine("[TestFSM] RequestAttack()\n\n\n\n\n\n\n");
        }

        // Hold W to move
        if (input.IsKeyPressed(87)) // W
        {
            fsm.RequestMove(true);
            Console.WriteLine("[TestFSM] fsm.RequestMove(true);\n\n\n\n\n");
        }
        else
        {
            fsm.RequestMove(false);
            Console.WriteLine("[TestFSM] fsm.RequestMove(false);\n\n\n\n\n");
        }

    }
}
