using System;
using ScriptAPI;

public class KeyVisual : Script
{
    // === CONFIG (auto-detects from sprite filename) ===
    public string KeyName = "";  // leave empty for auto-detect
    public float DisplayDuration = 0.2f;
    
    // === STATE ===
    private int keyCode = -1;
    private string soundID = "";
    private float timer = 0.0f;
    private bool isShowing = false;
    private bool initialized = false;

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            initialized = true;
        }

        if (keyCode < 0)
        {
            Console.WriteLine("[KeyVisual] ERROR: Invalid key code, cannot update");
            return;
        }

        InputComponent input = GetInput();
        TransformComponent t = GetTransform();
        
        // Check for key press
        if (input.IsKeyTriggered(keyCode))
        {
            Console.WriteLine($"[KeyVisual] {KeyName} pressed!");
            
            // Show visual
            t.IsVisible = true;
            isShowing = true;
            timer = DisplayDuration;
            
            // Play sound
            AudioComponent audio = GetAudio();
            audio.PlaySFX(soundID);
            Console.WriteLine($"[KeyVisual] Playing {soundID}");
        }
        
        // Update timer
        if (isShowing)
        {
            timer -= Time.DeltaTime;
            
            if (timer <= 0.0f)
            {
                // Hide visual
                t.IsVisible = false;
                isShowing = false;
            }
        }
    }

    private void Initialize()
    {
        Console.WriteLine("[KeyVisual] Initializing...");
        
        // auto-detect key from sprite filename if KeyName not set
        if (string.IsNullOrEmpty(KeyName))
        {
            SpriteComponent sprite = GetSprite();
            string texturePath = sprite.Texture.ToLower();
            
            Console.WriteLine($"[KeyVisual] Texture path: {texturePath}");
            
            if (texturePath.Contains("w.png"))
                KeyName = "W";
            else if (texturePath.Contains("a.png"))
                KeyName = "A";
            else if (texturePath.Contains("s.png"))
                KeyName = "S";
            else if (texturePath.Contains("d.png"))
                KeyName = "D";
            else
            {
                Console.WriteLine($"[KeyVisual] WARNING: Could not auto-detect key from texture: {texturePath}");
                Console.WriteLine("[KeyVisual] Please set KeyName manually in Inspector");
            }
        }

        // set key code and sound ID based on KeyName
        switch (KeyName.ToUpper())
        {
            case "W":
                keyCode = 87;
                soundID = "SFX_W_Key";
                break;
            case "A":
                keyCode = 65;
                soundID = "SFX_A_Key";
                break;
            case "S":
                keyCode = 83;
                soundID = "SFX_S_Key";
                break;
            case "D":
                keyCode = 68;
                soundID = "SFX_D_Key";
                break;
            default:
                Console.WriteLine($"[KeyVisual] ERROR: Unknown key name: '{KeyName}'");
                Console.WriteLine("[KeyVisual] Valid values: W, A, S, D");
                keyCode = -1;
                return;
        }

        Console.WriteLine($"[KeyVisual] Initialized successfully!");
        Console.WriteLine($"  Key: {KeyName}");
        Console.WriteLine($"  Key Code: {keyCode}");
        Console.WriteLine($"  Sound ID: {soundID}");
        Console.WriteLine($"  Display Duration: {DisplayDuration}s");
    }
}