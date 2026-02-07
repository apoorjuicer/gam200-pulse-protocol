/******************************************************************************/
/**
* @file        HoverableButton.cs
* @project     Pulse Protocol
* @author      Chloe Lau Rey En
* @brief       Script for hovering of buttons, and transition of scenes and showing overlay for quit button
*
* @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/

using System;
using System.Collections.Generic;
using ScriptAPI;

public class NavigationButtons : Script
{
    // === STATIC SCENE HISTORY (shared across all buttons) ===
    private static Stack<string> sceneHistory = new Stack<string>();
    private static string currentScene = "";
    
    // Track if we've set the initial scene THIS play session
    private static int lastFrameCount = -1;

    // === QUIT & PAUSE OVERLAY STATE ===
    public static bool ShowQuitOverlay = false;
    public static bool ShowPauseOverlay = false;

    private string pauseTexture = "";  
    private string pauseHoverTexture = "";
    private string defaultTexture = "";
    private string hoverTexture = "";
    private string exitTexture = "";
    private string exitHoverTexture = "";
    private bool isCurrentlyHovered = false;
    private bool buttonInitialized = false;
    private bool lastPauseState = false;
    private bool isPauseButton = false;
    private float hoverScaleBoost = 1.05f;

    private bool hasPlayedHoverSound = false;

    public override void Update()
    {
        // Reset history when a new Play session starts (frame count resets to 0)
        int currentFrame = Time.FrameCount;
        if (currentFrame < lastFrameCount || lastFrameCount < 0)
        {
            sceneHistory.Clear();
            currentScene = "";
            ShowQuitOverlay = false;
            ShowPauseOverlay = false;
        }

        lastFrameCount = currentFrame;

        InputComponent input = GetInput();
        TransformComponent t = GetTransform();

        if (!t.IsVisible)
        {
            return;
        }
        
        if (!buttonInitialized)
        {
            SpriteComponent sprite = GetSprite();
            defaultTexture = sprite.Texture;
            pauseTexture = defaultTexture;
            
            if (string.IsNullOrEmpty(currentScene))
            {
                currentScene = DetectCurrentScene();
            }
            
            int dotIndex = defaultTexture.LastIndexOf('.');
            if (dotIndex >= 0)
                hoverTexture = defaultTexture.Substring(0, dotIndex) + "_hover" + defaultTexture.Substring(dotIndex);
            else
                hoverTexture = defaultTexture + "_hover";
            
            pauseHoverTexture = hoverTexture;
            isPauseButton = defaultTexture.ToLower().Contains("pause");
        
            buttonInitialized = true;

            if (defaultTexture.ToLower().Contains("pause"))
            {
                exitTexture = defaultTexture.Replace("Pause", "Exit");
                exitHoverTexture = pauseHoverTexture.Replace("Pause", "Exit");
            }
        }

        if (isPauseButton)
        {
            bool isPauseShowing = ShowPauseOverlay;
            
            if (isPauseShowing != lastPauseState)
            {
                SpriteComponent sprite = GetSprite();
                if (isPauseShowing)
                {
                    Application.SetPaused(true);
                    
                    sprite.Texture = exitTexture;
                    defaultTexture = exitTexture;
                    hoverTexture = exitHoverTexture;
                }
                else
                {
                    Application.SetPaused(false);

                    sprite.Texture = pauseTexture;
                    defaultTexture = pauseTexture;
                    hoverTexture = pauseHoverTexture;
                }
                lastPauseState = isPauseShowing;
            }
        }

        string texLower = defaultTexture.ToLower();
        bool isQuitOverlayButton = texLower.Contains("quit yes") || texLower.Contains("quit no");
        bool isPauseOverlayButton = texLower.Contains("pause") || texLower.Contains("exit");
        bool isOverlayButton = isQuitOverlayButton || isPauseOverlayButton;
        
        if ((ShowQuitOverlay || ShowPauseOverlay) && !isOverlayButton)
        {
            if (isCurrentlyHovered)
            {
                SpriteComponent sprite = GetSprite();
                TransformComponent transform = GetTransform();
                sprite.Texture = defaultTexture;
                transform.ScaleX = t.ScaleX / hoverScaleBoost;
                transform.ScaleY = t.ScaleY / hoverScaleBoost;
                isCurrentlyHovered = false;
            }
            return;
        }

        if (ShowQuitOverlay && isPauseOverlayButton && !isQuitOverlayButton) return;
        if (ShowPauseOverlay && isQuitOverlayButton && !isPauseOverlayButton) return;
        
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

            PlayHoverSound();
            hasPlayedHoverSound = true;
        }
        else if (!isHovered && isCurrentlyHovered)
        {
            SpriteComponent sprite = GetSprite();
            TransformComponent transform = GetTransform();
            sprite.Texture = defaultTexture;
            transform.ScaleX = currentScaleX / hoverScaleBoost;
            transform.ScaleY = currentScaleY / hoverScaleBoost;
            isCurrentlyHovered = false;
            
            hasPlayedHoverSound = false;
        }
        
        if (isHovered && input.IsMouseButtonTriggered(0))
        {
            PlayClickSound();
            OnClick();
        }
    }

    private string DetectCurrentScene()
    {
        string tex = defaultTexture.ToLower();
        
        if (tex.Contains("new game") || tex.Contains("new_game") || tex.Contains("settings") || tex.Contains("continue") || tex.Contains("quit"))
            return "MainMenu";
        
        if (tex.Contains("back"))
            return "Settings";
        
        return "MainMenu";
    }

    private void NavigateTo(string newScene)
    {
        if (!string.IsNullOrEmpty(currentScene))
        {
            sceneHistory.Push(currentScene);
        }
        currentScene = newScene;
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
        ShowPauseOverlay = false;
        ShowQuitOverlay = false;
        
        // IMPORTANT: Unpause when going to MainMenu
        Application.SetPaused(false);
        
        Scene.LoadScene("MainMenu");
    }
    
    private void PlayHoverSound()
    {
        string texLower = defaultTexture.ToLower();
        bool isQuitOverlayButton = texLower.Contains("quit yes") || texLower.Contains("quit no");
        bool isPauseOverlayButton = texLower.Contains("pause") || texLower.Contains("exit") ||
                                    texLower.Contains("pausequit") || texLower.Contains("pauserestart") || 
                                    texLower.Contains("pausesettings");
        
        // Only block if there's an overlay showing AND this button doesn't belong to that overlay
        if (ShowQuitOverlay && !isQuitOverlayButton)
            return;
        
        if (ShowPauseOverlay && !isPauseOverlayButton)
            return;
        
        AudioComponent audio = GetAudio();
        audio.PlaySFX("SFX_Button_Hover");
    }
    
    private void PlayClickSound()
    {
        AudioComponent audio = GetAudio();
        audio.PlaySFX("SFX_Button_Select");
    }

    private void OnClick()
    {
        string tex = defaultTexture.ToLower();
        Console.WriteLine($"Button clicked: {tex}");

        if (tex.Contains("exit"))
        {
            Console.WriteLine("PAUSE EXIT - Hiding overlay");
            ShowPauseOverlay = false;
            return;
        }
        
        if (tex.Contains("pausequit"))
        {
            ShowPauseOverlay = false;
            Application.SetPaused(false);  // Unpause!
            GoToMainMenu();
            return;
        }
        
        if (tex.Contains("pauserestart"))
        {
            Scene.LoadScene(currentScene);
            ShowPauseOverlay = false;
            return;
        }

        if (tex.Contains("pausesettings"))
        {
            NavigateTo("Settings");
            ShowPauseOverlay = false;
            return;
        }
        
        if (tex.Contains("quit yes"))
        {
            Application.Quit();
            return;
        }
        
        if (tex.Contains("quit no"))
        {
            ShowQuitOverlay = false;
            return;
        }
        
        if (tex.Contains("new game") || tex.Contains("new_game") || tex.Contains("start"))
        {
            NavigateTo("Cutscene");
            return;
        }
        
        if (tex.Contains("continue"))
        {
            Console.WriteLine("NOT IMPLEMENTED YET!");
            return;
        }
        
        if (tex.Contains("settings") || tex.Contains("options"))
        {
            NavigateTo("Settings");
            return;
        }
        
        if (tex.Contains("back") || tex.Contains("return"))
        {
            GoBack();
            return;
        }
        
        if (tex.Contains("quit"))
        {
            ShowQuitOverlay = true;
            return;
        }

        if (tex.Contains("pause"))
        {
            ShowPauseOverlay = true;
            return;
        }
    }
}

public class QuitOverlay : Script
{
    public override void Update()
    {
        TransformComponent t = GetTransform();
        t.IsVisible = NavigationButtons.ShowQuitOverlay;
    }
}

public class PauseOverlay : Script
{
    public override void Update()
    {
        TransformComponent t = GetTransform();
        t.IsVisible = NavigationButtons.ShowPauseOverlay;
    }
}