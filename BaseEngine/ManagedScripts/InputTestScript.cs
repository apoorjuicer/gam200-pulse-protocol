using System;
using ScriptAPI;

public class InputTestScript : Script
{
    public override void Update()
    {
        InputComponent input = GetInput();
        
        // Mouse test
        if (input.IsMouseButtonTriggered(0))
            Console.WriteLine("Left mouse clicked!");
        
        if (input.IsMouseButtonTriggered(1))
            Console.WriteLine("Right mouse clicked!");
        
        // Keyboard test (W=87, A=65, S=83, D=68)
        if (input.IsKeyTriggered(87))
            Console.WriteLine("W pressed!");
            
        if (input.IsKeyTriggered(65))
            Console.WriteLine("A pressed!");
            
        if (input.IsKeyTriggered(83))
            Console.WriteLine("S pressed!");
            
        if (input.IsKeyTriggered(68))
            Console.WriteLine("D pressed!");
    }
}