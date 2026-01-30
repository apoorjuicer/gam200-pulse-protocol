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
    float spawnCooldown = 1.0f;
    float deathtimer = 2.0f;
    bool startdeathtimer = false;
    bool deadalready = false;
    float sceneCoolDown = 10.0f;
    bool hasSpawned = false;
    bool said = false;
    public override void Update()
    {
        Console.Write("gay 1\n");
        //Console.WriteLine("[Testing CSHARP]: Hello from ManagedScripts! \n ");

        TransformComponent TFC = GetTransform();
        CollisionComponent CC = GetCollision();
        HealthComponent HC = GetHealth();
        Animation AM = GetAnimation();
        // You can just call factory and scene function since its not tight to any entity

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
        
        Console.WriteLine($"Collided?: {CC.CheckAABB(9)}");
        Console.WriteLine($"HP: {HC.hp}, Damage: {HC.damage}, Alive: {HC.isAlive}");
        //Console.WriteLine($"Script being used: {LC.ScriptName}");

        // Test modifying health
        //Console.WriteLine($"HP: {HC.hp}, Damage: {HC.damage}, Alive: {HC.isAlive}");
        HC.hp -= 1;  // Decrease health each frame
        HC.damage += 1;  // Increase damage 
        AM.SetAnimationSpeed(HC.damage);

        // Check if dead
        if (HC.hp <= 0 && !said)
        {
            said = true;
            HC.isAlive = false; Console.WriteLine("ENTITY IS DEAD!");
            AM.SetAnimation("hero_spritesheet", 1,3,3,2.0f);
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
            Console.WriteLine($"Current Animation: {AM.GetCurrentAnimation()}");
            deadalready = true;
        }

        if (sceneCoolDown  <=0 )
        {
            //Scene.LoadScene("GameLose");
            Scene.RestartScene();
            Console.WriteLine("New Scene Loaded!\n\n\n\n\n\n\n");
            return;
        }
       
        TFC.RotX += 50.0f * dt;
     
        TFC.X -= 20.0f * dt ;
        TFC.Y -= 20.0f *dt;

        Console.WriteLine($"Frams: {Time.FrameCount}, Time: {Time.GameTime}\n\n\n\n\n\n\n");
        Console.WriteLine($"Current Animation: {AM.GetCurrentAnimation()}");
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
