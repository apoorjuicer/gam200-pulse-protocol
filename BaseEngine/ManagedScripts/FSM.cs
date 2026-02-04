using System;
using ScriptAPI;

public class TestFSM : ScriptAPI.Script
{
    int frame = 0;

    public override void Update()
    {
        var fsm = GetFSM();
        var input = GetInput();

        if ((frame++ % 30) == 0)
            Console.WriteLine($"[TestFSM] State: {fsm.GetState()}");

        if (input.IsKeyPressed(87)) // W
            fsm.RequestMove(true);
        else
            fsm.RequestMove(false);

        if (input.IsKeyTriggered(32)) // Space
            fsm.RequestAttack();
    }
}
