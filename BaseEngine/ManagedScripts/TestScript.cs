using System;

public class TestScript : ScriptAPI.Script
{
    public static void SayHello()
    {
        Console.WriteLine("[C#] Hello from ManagedScripts!");
    }
    public override void Update()
    {
        Console.WriteLine("Testing! PLEASE FUCKING WORK!\n\n\n\n\n\n\n\n");
    }
}
