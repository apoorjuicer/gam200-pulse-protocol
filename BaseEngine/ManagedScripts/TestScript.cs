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
using System.Runtime.CompilerServices;
using ScriptAPI;
public class TestScript : Script
{
    int enemy;
    float spawnCooldown = 5.0f;
    float deathtimer = 10.0f;
    bool startdeathtimer = false;
    bool deadalready = false;
    float sceneCoolDown = 20.0f;
    bool hasSpawned = false;
    public override void Update()
    {
        Console.Write("Script 1\n");
        //Console.WriteLine("[Testing CSHARP]: Hello from ManagedScripts! \n ");

        TransformComponent TFC = GetTransform();
        HealthComponent HC = GetHealth();
        //LogicComponent LC = GetScriptName();

        //Timer Example ig :3
        float dt = Time.DeltaTime;
        if (spawnCooldown > 0)
        {
            spawnCooldown -= dt; // Subtract tiny slice of time
            Console.WriteLine(spawnCooldown);
        }

        if (sceneCoolDown > 0)
        {
            sceneCoolDown -= dt; // Subtract tiny slice of time
        }

        // Test modifying health
        Console.WriteLine($"HP: {HC.hp}, Damage: {HC.damage}, Alive: {HC.isAlive}");
        HC.hp -= 1;  // Decrease health each frame
        HC.damage += 1;  // Increase damage 

        // Check if dead
        if (HC.hp <= 0)
        {
            HC.isAlive = false; Console.WriteLine("ENTITY IS DEAD!");
        }


        // dumb way i tested if it works :>
        if (spawnCooldown <= 0 && !hasSpawned)
        {
            enemy = Factory.Instantiate("TestPrefab");
            hasSpawned = true; // Set the flag so it only happens once
            Console.WriteLine("Timer Finished: Monster Spawned!");
            startdeathtimer = true;
        }

        if (startdeathtimer)
        {
            if (deathtimer > 0)
            {
                deathtimer -= dt; 
            }
        }

        if (deathtimer <= 0 && startdeathtimer && !deadalready)
        {
            Factory.Destroy(enemy);
            enemy = -1;
            Console.WriteLine("ENTITY IS DELETED!\n\n\n\n\n\n\n");
            deadalready = true;
        }

        if (sceneCoolDown  <=0 )
        {
            Scene.LoadScene("GameLose");
            Console.WriteLine("New Scene Loaded!\n\n\n\n\n\n\n");
            return;
        }

        TFC.RotX += 50.0f * dt;
     
        TFC.X += 20.0f * dt ;
        TFC.Y += 20.0f *dt;

        Console.WriteLine($"Frams: {Time.FrameCount}, Time: {Time.GameTime}\n\n\n\n\n\n\n");
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
