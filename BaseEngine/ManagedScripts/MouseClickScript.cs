using System;
using ScriptAPI;

public class SimpleClickDetectScript : Script
{
    public override void Update()
    {
        InputComponent input = GetInput();
        
        if (input.IsButtonTriggered(0))
        {
            Console.WriteLine("Button clicked! Loading scene...");
            // sceneManager.CurrentScene = "Level1";
        }
    }
}