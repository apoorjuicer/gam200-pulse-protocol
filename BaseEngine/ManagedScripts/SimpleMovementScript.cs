using System;
using ScriptAPI;

public class SimpleMovementScript : Script
{
    private int frameCounter = 0;
    private float direction = 1.0f;

    public override void Update()
    {
        TransformComponent TFC = GetTransform();
        
        frameCounter++;
        
        if (frameCounter > 60)
        {
            direction *= -1;
            frameCounter = 0;
        }
        
        TFC.X += direction * 2.0f;
        Console.WriteLine($"SimpleMovementScript moving: {direction}");
    }
}