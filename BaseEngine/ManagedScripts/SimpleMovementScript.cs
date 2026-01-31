using System;
using ScriptAPI;

public class SimpleMove : Script
{
    private float speed = 200f;
    
    // GLFW Key Codes
    private const int KEY_W = 87;
    private const int KEY_A = 65;
    private const int KEY_S = 83;
    private const int KEY_D = 68;
    private const int KEY_UP = 265;
    private const int KEY_DOWN = 264;
    private const int KEY_LEFT = 263;
    private const int KEY_RIGHT = 262;
    
    public override void Update()
    {
        var transform = GetTransform();
        var input = GetInput();
        
        float dt = Time.DeltaTime;
        
        // WASD
        if (input.IsKeyPressed(KEY_W))
        {
            transform.Y += speed * dt;
        }
        if (input.IsKeyPressed(KEY_S))
        {
            transform.Y -= speed * dt;
        }
        if (input.IsKeyPressed(KEY_A))
        {
            transform.X -= speed * dt;
        }
        if (input.IsKeyPressed(KEY_D))
        {
            transform.X += speed * dt;
        }
        
        // Arrow Keys
        if (input.IsKeyPressed(KEY_UP))
        {
            transform.Y += speed * dt;
        }
        if (input.IsKeyPressed(KEY_DOWN))
        {
            transform.Y -= speed * dt;
        }
        if (input.IsKeyPressed(KEY_LEFT))
        {
            transform.X -= speed * dt;
        }
        if (input.IsKeyPressed(KEY_RIGHT))
        {
            transform.X += speed * dt;
        }
    }
}