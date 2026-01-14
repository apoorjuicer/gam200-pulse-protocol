/******************************************************************************/
/**
* @file        TestScript.cs
* @project     Pulse Protocol
* @author      Chia Wei Xuan Rachael - 100%
* @brief       Prove that C# scripting is integrated with the engine by manipulate entity transforms.
*
* @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/
using System;
using ScriptAPI;
public class TestScript : Script
{
    public override void Update()
    {
        Console.Write("Script 1\n");
        //Console.WriteLine("[Testing CSHARP]: Hello from ManagedScripts! \n ");
        
        TransformComponent TFC = GetTransform();
        HealthComponent HC = GetHealth();
        //LogicComponent LC = GetScriptName();

        Console.WriteLine($"HP: {HC.hp}, Damage: {HC.damage}, Alive: {HC.isAlive}");
        //Console.WriteLine($"Script being used: {LC.ScriptName}");

        // Test modifying health
        HC.hp -= 1;  // Decrease health each frame
        HC.damage += 1;  // Increase damage 

        // Check if dead
        if (HC.hp <= 0)
        {
            HC.isAlive = false;
            Console.WriteLine("ENTITY IS DEAD!");
        }

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

        TFC.ScaleX -= 2.5f;
        TFC.ScaleY -= 2.5f;
    }
}
