using System;
using ScriptAPI;

public class MainMenuButton : Script
{
    private string defaultTexture = "";
    private string hoverTexture = "";
    private bool isCurrentlyHovered = false;
    private bool initialized = false;
    private float hoverScaleBoost = 1.05f;

    public override void Update()
    {
        InputComponent input = GetInput();
        TransformComponent t = GetTransform();
        
        if (!initialized)
        {
            SpriteComponent sprite = GetSprite();
            defaultTexture = sprite.Texture;
            
            int dotIndex = defaultTexture.LastIndexOf('.');
            if (dotIndex >= 0)
                hoverTexture = defaultTexture.Substring(0, dotIndex) + "_hover" + defaultTexture.Substring(dotIndex);
            else
                hoverTexture = defaultTexture + "_hover";
            
            initialized = true;
        }
        
        float currentScaleX = t.ScaleX;
        float currentScaleY = t.ScaleY;
        float baseScaleX = isCurrentlyHovered ? currentScaleX / hoverScaleBoost : currentScaleX;
        float baseScaleY = isCurrentlyHovered ? currentScaleY / hoverScaleBoost : currentScaleY;
        
        float halfW = baseScaleX / 2f;
        float halfH = baseScaleY / 2f;
        float mx = input.WorldMouseX;
        float my = input.WorldMouseY;
        
        bool isHovered = mx >= t.X - halfW && mx <= t.X + halfW &&
                         my >= t.Y - halfH && my <= t.Y + halfH;
        
        if (isHovered && !isCurrentlyHovered)
        {
            SpriteComponent sprite = GetSprite();
            TransformComponent transform = GetTransform();
            sprite.Texture = hoverTexture;
            transform.ScaleX = currentScaleX * hoverScaleBoost;
            transform.ScaleY = currentScaleY * hoverScaleBoost;
            isCurrentlyHovered = true;
        }
        else if (!isHovered && isCurrentlyHovered)
        {
            SpriteComponent sprite = GetSprite();
            TransformComponent transform = GetTransform();
            sprite.Texture = defaultTexture;
            transform.ScaleX = currentScaleX / hoverScaleBoost;
            transform.ScaleY = currentScaleY / hoverScaleBoost;
            isCurrentlyHovered = false;
        }
        
        if (isHovered && input.IsMouseButtonTriggered(0))
        {
            OnClick();
        }
    }
    
    private void OnClick()
    {
        string tex = defaultTexture.ToLower();
        
        if (tex.Contains("newgame") || tex.Contains("new_game") || tex.Contains("new game"))
            Scene.LoadScene("Level1");
        else if (tex.Contains("continue"))
            // Scene.LoadScene("SavedGame");
            Console.WriteLine("NOT IMPLEMENTED YET!");
        else if (tex.Contains("settings"))
            // Scene.LoadScene("Settings");
            Console.WriteLine("NOT IMPLEMENTED YET!");
        else if (tex.Contains("quit"))
            Console.WriteLine("NOT IMPLEMENTED YET!");
    }
}