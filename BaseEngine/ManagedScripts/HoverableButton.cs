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

using ScriptAPI;
using System;
using System.Collections.Generic;

public class NavigationButtons : Script
{
    // === STATIC SCENE HISTORY (shared across all buttons) ===
    private static Stack<string> sceneHistory = new Stack<string>();
    public static string currentScene = "";
    public static string LastGameplayScene = "";

    // Track if we've set the initial scene THIS play session
    private static int lastFrameCount = -1;

    // === OVERLAY STATE ===
    public static bool ShowQuitOverlay = false;
    public static bool ShowPauseOverlay = false;
    public static bool ShowHowToPlayOverlay = false;

    // === SETTINGS OVERLAY (inside MainMenu scene) ===
    public static bool ShowSettingsOverlay = false;

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

    private static bool wasEscPressed = false;

    public override void Update()
    {
        // Reset history when a new Play session starts (frame count resets to 0)
        int currentFrame = Time.FrameCount;
        if (currentFrame < lastFrameCount || lastFrameCount < 0)
        {
            sceneHistory.Clear();
            currentScene = "";
            LastGameplayScene = "";

            ShowQuitOverlay = false;
            ShowPauseOverlay = false;
            ShowHowToPlayOverlay = false;
            ShowSettingsOverlay = false;

            wasEscPressed = false;
        }

        if (currentFrame != lastFrameCount)
        {
            wasEscPressed = false;
        }

        lastFrameCount = currentFrame;

        InputComponent input = GetInput();
        TransformComponent t = GetTransform();

        string realScene = Scene.GetCurrentScene();

        // ESC pause toggle (only in gameplay scenes)
        if (input.IsKeyTriggered(256) && !wasEscPressed) // 256 = ESC (GLFW)
        {
            wasEscPressed = true;

            if (!string.IsNullOrEmpty(realScene) &&
                !realScene.Equals("MainMenu") &&
                !realScene.Equals("Settings") &&
                !realScene.Equals("Cutscene") &&
                !realScene.Equals("HowToPlay"))
            {
                ShowPauseOverlay = !ShowPauseOverlay;
            }
        }

        // --- Always initialize first (even if currently invisible),
        // so buttons can be hidden then shown again safely.
        if (!buttonInitialized)
        {
            SpriteComponent sprite = GetSprite();
            defaultTexture = sprite.Texture ?? "";
            pauseTexture = defaultTexture;

            if (string.IsNullOrEmpty(currentScene))
            {
                // Prefer engine scene name; fall back to old heuristic if empty
                string sc = Scene.GetCurrentScene();
                currentScene = string.IsNullOrEmpty(sc) ? DetectCurrentScene() : sc;
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

        // =========================================================
        // SETTINGS OVERLAY VISIBILITY GATING (MainMenu buttons)
        // =========================================================
        // This is what actually hides "other stuff" in MainMenu.
        // It runs BEFORE the early return on invisibility.
        if (realScene == "MainMenu")
        {
            SpriteComponent sp = GetSprite();
            string texNow = (sp.Texture ?? "").ToLower();

            bool isBackBtn = texNow.Contains("back");

            bool isMainMenuBtn =
                texNow.Contains("new game") || texNow.Contains("new_game") ||
                texNow.Contains("continue") ||
                // settings main menu button (avoid pause settings)
                (texNow.Contains("settings") && !texNow.Contains("pausesettings")) ||
                // quit main menu button (avoid quit yes/no and pausequit)
                (texNow.Contains("quit") && !texNow.Contains("quit yes") && !texNow.Contains("quit no") && !texNow.Contains("pausequit")) ||
                texNow.Contains("how to play") || texNow.Contains("how_to_play");

            bool shouldBeVisible = t.IsVisible;

            if (ShowSettingsOverlay)
            {
                if (isBackBtn) shouldBeVisible = true;
                else if (isMainMenuBtn) shouldBeVisible = false;
            }
            else
            {
                if (isBackBtn) shouldBeVisible = false;
                else if (isMainMenuBtn) shouldBeVisible = true;
            }

            if (t.IsVisible != shouldBeVisible)
            {
                // If we are hiding while hovered, reset sprite/scale cleanly
                if (!shouldBeVisible && isCurrentlyHovered)
                {
                    SpriteComponent sprite = GetSprite();
                    sprite.Texture = defaultTexture;

                    t.ScaleX = t.ScaleX / hoverScaleBoost;
                    t.ScaleY = t.ScaleY / hoverScaleBoost;

                    isCurrentlyHovered = false;
                    hasPlayedHoverSound = false;
                }

                t.IsVisible = shouldBeVisible;
            }
        }

        // If invisible after gating, do nothing further
        if (!t.IsVisible)
            return;

        // =========================================================
        // Pause button swaps to Exit while pause overlay is shown
        // =========================================================
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

        // Settings is now an overlay inside MainMenu, so back still means MainMenu
        if (tex.Contains("back"))
            return "MainMenu";

        return "MainMenu";
    }

    private void NavigateTo(string newScene)
    {
        // Leaving MainMenu? don't carry Settings overlay across scenes
        if (newScene != "MainMenu")
            ShowSettingsOverlay = false;

        if (!string.IsNullOrEmpty(currentScene))
        {
            sceneHistory.Push(currentScene);
        }
        currentScene = newScene;
        Scene.LoadScene(newScene);
    }

    private void GoBack()
    {
        // If settings overlay is open, back should just close it
        if (ShowSettingsOverlay)
        {
            HideSettings();
            return;
        }

        if (sceneHistory.Count > 0)
        {
            currentScene = sceneHistory.Pop();
            Scene.LoadScene(currentScene);
        }
        else
        {
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
        ShowHowToPlayOverlay = false;
        ShowSettingsOverlay = false;

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

    public static void LoadGameplayScene(string sceneName)
    {
        ShowSettingsOverlay = false;

        string previous = string.IsNullOrEmpty(currentScene) ? "MainMenu" : currentScene;
        LastGameplayScene = previous;
        currentScene = sceneName;

        Scene.LoadScene(sceneName);
    }

    public static void GoToGameLose()
    {
        ShowSettingsOverlay = false;

        if (!string.IsNullOrEmpty(currentScene))
            LastGameplayScene = currentScene;
        else
            LastGameplayScene = "MainMenu";

        currentScene = "GameLose";
        Scene.LoadScene("GameLose");
    }

    // === The 2 functions you asked for (now used properly) ===
    private static void ShowSettings()
    {
        // Close other overlays just in case
        ShowQuitOverlay = false;
        ShowPauseOverlay = false;
        ShowHowToPlayOverlay = false;

        ShowSettingsOverlay = true;
    }

    private static void HideSettings()
    {
        ShowSettingsOverlay = false;
    }

    private void OnClick()
    {
        string tex = defaultTexture.ToLower();
        Console.WriteLine($"Button clicked: {tex}");

        if (tex.Contains("exit"))
        {
            ShowPauseOverlay = false;
            return;
        }

        if (tex.Contains("pausequit"))
        {
            ShowPauseOverlay = false;
            Application.SetPaused(false);
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
            // Open settings overlay from pause by jumping back to MainMenu
            ShowPauseOverlay = false;
            Application.SetPaused(false);
            ShowSettingsOverlay = true;
            Scene.LoadScene("MainMenu");
            return;
        }

        if (tex.Contains("restartyes"))
        {
            if (!string.IsNullOrEmpty(LastGameplayScene))
            {
                currentScene = LastGameplayScene;
                Scene.LoadScene(LastGameplayScene);
            }
            else
            {
                currentScene = "MainMenu";
                Scene.LoadScene("MainMenu");
            }
            return;
        }

        if (tex.Contains("restartno"))
        {
            GoToMainMenu();
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
            ShowSettings();
            return;
        }

        if (tex.Contains("back") || tex.Contains("return"))
        {
            HideSettings();
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

        if (tex.Contains("home"))
        {
            StageClearElement.ResetStageClear();
            GoToMainMenu();
            return;
        }

        if (tex.Contains("nextstage"))
        {
            Console.WriteLine("NOT IMPLEMENTED YET!");
            return;
        }

        if (tex.Contains("how to play"))
        {
            ShowHowToPlayOverlay = true;
            return;
        }

        if (tex.Contains("temp_close"))
        {
            ShowHowToPlayOverlay = false;
            return;
        }

        if (tex.Contains("windowed"))
        {
            return;
        }

        if (tex.Contains("full screen"))
        {
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

public class HowToPlayOverlay : Script
{
    public override void Update()
    {
        TransformComponent t = GetTransform();
        t.IsVisible = NavigationButtons.ShowHowToPlayOverlay;
    }
}

// Attach this script name to your Settings root entity (BG_PlaceholderImg)
// so the actual settings panel appears/disappears with ShowSettingsOverlay.
public class SettingsOverlay : Script
{
    public override void Update()
    {
        TransformComponent t = GetTransform();
        t.IsVisible = NavigationButtons.ShowSettingsOverlay;
    }
}