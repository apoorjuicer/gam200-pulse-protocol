using System;
using System.Collections.Generic;
using ScriptAPI;

public class NavigationButtons : Script
{
    // === STATIC SCENE HISTORY (shared across all buttons) ===
    private static Stack<string> sceneHistory = new Stack<string>();
    private static string currentScene = "MainMenu";
    private static bool initialized = false;

    private string defaultTexture = "";
    private string hoverTexture = "";
    private bool isCurrentlyHovered = false;
    private bool buttonInitialized = false;
    private float hoverScaleBoost = 1.05f;

    public override void Update()
    {
        // Initialize scene history once
        if (!initialized)
        {
            sceneHistory.Clear();
            currentScene = "MainMenu";  // Or detect from your engine if possible
            initialized = true;
        }

        InputComponent input = GetInput();
        TransformComponent t = GetTransform();
        
        if (!buttonInitialized)
        {
            SpriteComponent sprite = GetSprite();
            defaultTexture = sprite.Texture;
            
            int dotIndex = defaultTexture.LastIndexOf('.');
            if (dotIndex >= 0)
                hoverTexture = defaultTexture.Substring(0, dotIndex) + "_hover" + defaultTexture.Substring(dotIndex);
            else
                hoverTexture = defaultTexture + "_hover";
            
            buttonInitialized = true;
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

    // === NAVIGATION HELPERS ===
    private void NavigateTo(string newScene)
    {
        if (!string.IsNullOrEmpty(currentScene))
        {
            sceneHistory.Push(currentScene);
        }
        currentScene = newScene;
        Console.WriteLine($"Navigating to: {newScene} | History: {sceneHistory.Count} scenes");
        Scene.LoadScene(newScene);
    }

    private void GoBack()
    {
        if (sceneHistory.Count > 0)
        {
            currentScene = sceneHistory.Pop();
            Console.WriteLine($"Going back to: {currentScene}");
            Scene.LoadScene(currentScene);
        }
        else
        {
            Console.WriteLine("No history, going to MainMenu");
            currentScene = "MainMenu";
            Scene.LoadScene("MainMenu");
        }
    }

    private void GoToMainMenu()
    {
        sceneHistory.Clear();
        currentScene = "MainMenu";
        Scene.LoadScene("MainMenu");
    }
    
    private void OnClick()
    {
        string tex = defaultTexture.ToLower();
        
        if (tex.Contains("new game") || tex.Contains("new_game") || tex.Contains("start"))
            NavigateTo("Level1");
        else if (tex.Contains("continue"))
            Console.WriteLine("NOT IMPLEMENTED YET!");
        else if (tex.Contains("settings") || tex.Contains("options"))
            NavigateTo("Settings");
        else if (tex.Contains("quit") || tex.Contains("exit"))
            Console.WriteLine("NOT IMPLEMENTED YET!");
        else if (tex.Contains("back") || tex.Contains("return"))
            GoBack();
        // else if (tex.Contains("mainmenu") || tex.Contains("main_menu"))
        //     GoToMainMenu();
        // else if (tex.Contains("restart") || tex.Contains("retry"))
        //     Scene.LoadScene(currentScene);  // Reload current scene
    }
}