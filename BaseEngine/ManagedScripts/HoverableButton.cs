using System;
using ScriptAPI;

public class HoverableButton : Script
{
    // Set these in editor or initialize in Start()
    private string defaultTexture;
    private string hoverTexture;
    
    private float originalScaleX;
    private float originalScaleY;
    private bool wasHovered = false;
    private bool initialized = false;
    
    // Hover scale multiplier (1.0 = no change, 1.1 = 10% bigger)
    private float hoverScaleMultiplier = 1.05f;

    public override void Start()
    {
        // Grab current texture as default
        defaultTexture = GetSprite().Texture;
        
        // Build hover texture path by inserting "_hover" before extension
        // e.g., "path/to/button.png" -> "path/to/button_hover.png"
        hoverTexture = BuildHoverTexturePath(defaultTexture);
        
        // Store original scale
        originalScaleX = GetTransform().ScaleX;
        originalScaleY = GetTransform().ScaleY;
        
        initialized = true;
        
        Console.WriteLine($"[HoverableButton] Default: {defaultTexture}");
        Console.WriteLine($"[HoverableButton] Hover:   {hoverTexture}");
    }

    public override void Update()
    {
        if (!initialized)
            return;

        InputComponent input = GetInput();
        
        if (!input.InGameViewport)
            return;

        bool isHovered = IsMouseOverEntity(input);

        // Hover enter
        if (isHovered && !wasHovered)
        {
            OnHoverEnter();
        }
        // Hover exit
        else if (!isHovered && wasHovered)
        {
            OnHoverExit();
        }

        // Click while hovering
        if (isHovered && input.IsMouseButtonTriggered(0))
        {
            OnClick();
        }

        wasHovered = isHovered;
    }

    private bool IsMouseOverEntity(InputComponent input)
    {
        TransformComponent transform = GetTransform();
        
        // Use ORIGINAL scale for hit detection (not current scaled-up size)
        float halfW = originalScaleX / 2f;
        float halfH = originalScaleY / 2f;

        float mx = input.WorldMouseX;
        float my = input.WorldMouseY;

        return mx >= transform.X - halfW && mx <= transform.X + halfW &&
               my >= transform.Y - halfH && my <= transform.Y + halfH;
    }

    private void OnHoverEnter()
    {
        // Change texture
        GetSprite().Texture = hoverTexture;
        
        // Scale up slightly
        GetTransform().ScaleX = originalScaleX * hoverScaleMultiplier;
        GetTransform().ScaleY = originalScaleY * hoverScaleMultiplier;
        
        Console.WriteLine("[HOVER ENTER]");
    }

    private void OnHoverExit()
    {
        // Restore texture
        GetSprite().Texture = defaultTexture;
        
        // Restore scale
        GetTransform().ScaleX = originalScaleX;
        GetTransform().ScaleY = originalScaleY;
        
        Console.WriteLine("[HOVER EXIT]");
    }

    private void OnClick()
    {
        Console.WriteLine("[CLICKED]");
        // Add your button action here
    }

    private string BuildHoverTexturePath(string originalPath)
    {
        // Find last dot for extension
        int dotIndex = originalPath.LastIndexOf('.');
        
        if (dotIndex == -1)
        {
            // No extension, just append _hover
            return originalPath + "_hover";
        }
        
        // Insert _hover before extension
        // "folder/button.png" -> "folder/button_hover.png"
        string basePath = originalPath.Substring(0, dotIndex);
        string extension = originalPath.Substring(dotIndex);
        
        return basePath + "_hover" + extension;
    }
}