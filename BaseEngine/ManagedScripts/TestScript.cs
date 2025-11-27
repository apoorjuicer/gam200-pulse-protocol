using System;
using ScriptAPI;
public class TestScript : Script
{
    public static void SayHello()
    {
        Console.WriteLine("[C#] Hello from ManagedScripts!");
    }
    public override void Update()
    {
        //Console.WriteLine("[Testing CSHARP]: Hello from ManagedScripts! \n ");

        TransformComponent TFC = GetTransform();

        TFC.ScaleX += 0.01f;
        TFC.ScaleY += 0.01f;

        TFC.RotX += 1.0f;
     
        TFC.X += 1.0f;
        TFC.Y += 1.0f;
    }
}

public class TestScript2 : Script
{
    public override void Update()
    {
        TransformComponent TFC = GetTransform();

        TFC.ScaleX += 5.0f;
        TFC.ScaleY += 5.0f;
    }
}
