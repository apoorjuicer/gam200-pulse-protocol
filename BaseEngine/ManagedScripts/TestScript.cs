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

        TFC.RotX += 0.5f;
     
        TFC.X += 1.0f;
        TFC.Y += 1.0f;
    }
}

public class TestScript2 : Script
{
    public override void Update()
    {
        TransformComponent TFC = GetTransform();

        TFC.ScaleX += 2.5f;
        TFC.ScaleY += 2.5f;
    }
}
