using System;
using System.Runtime.CompilerServices;
using ScriptAPI;

//How to make a script
public class ScriptName : Script
{
    // An update function & if you want, helper functions 
    public override void Update()
    {
        //Firstly, to get get a component:
        FSMComponentAPI fsm = GetFSM();
        Animation anim = GetAnimation();
        SpriteComponent s = GetSprite();
        InputComponent input = GetInput();
        AudioComponent audio = GetAudio();
        HealthComponent health = GetHealth();
        CollisionComponent c = GetCollision();
        TransformComponent t = GetTransform();


        // 1. TransformComponent:
        t.X = 0f;                   // world position X
        t.Y = 0f;                   // world position Y
        t.RotX = 0f;                // rotation X
        t.RotY = 0f;                // rotation Y
        t.ScaleX = 0f;              // width
        t.ScaleY = 0f;              // height
        t.IsVisible = true;         // show/hide entity


        health.hp = 5;              // returns int, get/set current HP
        health.damage += 1;         // returns int, get/set damage value
        health.isAlive = false;     // returns bool, get/set alive state

        // 2. Scene API
        Scene.LoadScene("SceneName");                       // load scene,name must match JSON!!!
        string SceneName = Scene.GetCurrentScene();         // string of current scene name

        // 3. Factory API
        int id = Factory.Instantiate("PrefabName");         // spawn from prefab JSON, returns entity ID
        Factory.Destroy(id);                                // destroy an entity, KILL OTHER ID, not urself. That one call health API

        // 4. Collision API
        // Note: currently there is no check all entity with collision component,but since 
        // we only have 1 enemy, should be quite easy to just check with 1 enemy.
        int otherEntityID = Factory.Instantiate("PrefabName");
        c.CheckAABB(otherEntityID);                                              // returns bool, check CURRENT entity with another entity
        float leeway =5.0f;
        c.CheckEdgeContact(otherEntityID, leeway);                               // return bool, edge contact with some leeway
        float CurrentEntityRadius = 5, OtherEntityRadius = 5;
        c.CheckDistance(CurrentEntityRadius, otherEntityID, OtherEntityRadius);  // bool - circle vs circle

        // 5. Delta Time
        float dt = Time.DeltaTime;     // seconds since last frame (use for movement OR timer!!!!!!!)
        t.X += 5.0f * Time.DeltaTime;  // Movement
        float spawnCooldown = 5.0f;    // Timer example :p
        if (spawnCooldown > 0)
        {
            spawnCooldown -= dt;
        }
        //Time.FrameCount;             // total frames since start
        //Time.GameTime;               // total time elapsed in seconds, Used for game Time Counter


        // 6. Game State API
        Application.Quit();               // close the game
        Application.SetPaused(false);     // paused means set DeltaTime to = 0, so it no move if u apply dt 2 movement
        Application.IsPlaying();          
        Application.IsPaused();

        // 7. Sprite texture
        s.Texture = "";                     // returns string, get/set texture path, not sure who did it 

        // 8. Audio API
        audio.Play();                       // play this entity's own audiosource component
        audio.PlaySFX("SFX_Button_Hover");  // play sound by audio bank ID
        audio.PlayBGM("BGM_GENERAL");       // play BGM by ID
        audio.StopBGM();
        audio.PauseBGM();
        audio.ResumeBGM();

        //Setter for audio
        audio.SetBGMVolume(0.5f);           // 0.0 - 1.0
        audio.SetMasterVolume(0.5f);
        audio.SetMusicVolume(0.5f);
        audio.SetSfxVolume(0.5f);
        
        //Getter for audio
        audio.GetMasterVolume();            // returns float
        audio.GetMusicVolume();
        audio.GetSfxVolume();


        // 9. Vector2 for audio I think? TLDR, vec2 is now able to use in C#
        float x = 5, y = 5;
        Vector2 v = new Vector2(x,y);


        // 10. State Machine
        fsm.GetState();          // Get current state (Idle/Moving/Attack/Damage/Death)
        fsm.RequestMove(true);   // request movement on/off
        fsm.RequestAttack();     // trigger attack
        fsm.NotifyDamage();      // notify taken damage
        fsm.ClearRequests();     // clear all pending requests

        // 11. Mouse/Keyboard API 
        float PosX, PosY;
        PosX = input.MouseX;                    // screen space X
        PosY = input.MouseY;                    // screen space Y
        PosX = input.WorldMouseX;               // world space X (use for button hit detection?)
        PosY = input.WorldMouseY;               // world space Y
        bool OnScreen = input.InGameViewport;   // check if mouse is inside game window

        // Mouse buttons — 0=left, 1=right, 2=middle
        input.IsMouseButtonPressed(0);    // held down
        input.IsMouseButtonTriggered(0);  // just clicked this frame
        input.IsMouseButtonReleased(0);  // just released this frame

        // Keyboard — GLFW key codes (NOT ASCII)
        input.IsKeyPressed(32);    // held down
        input.IsKeyTriggered(32);  // just pressed this frame
        input.IsKeyReleased(32);   // just released this frame


        // 12. Animation API
        int row = 5, col = 5, totalFrames = 5; 
        float speed = 5.0f;
        anim.SetAnimation("SpriteName", row, col, totalFrames, speed);  // Set current entity animation 2 smth else
        string current = anim.GetCurrentAnimation();                    // Get current sprite name (returns full path string)
        anim.SetAnimationSpeed(speed);                                  // Change speed of animation
    }
}