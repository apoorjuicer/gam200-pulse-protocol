/******************************************************************************/
/**
* @file        HoverableButton.cs
* @project     Pulse Protocol
* @author      Chloe Lau Rey En (primary) - 70%
               Leu Jun Yong (secondary) - 30%
* @brief       Script for hovering of buttons, and transition of scenes and showing overlay for all 4 buttons
*
* @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/

using ScriptAPI;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;

public class NavigationButtons : Script
{
    // === STATIC SCENE HISTORY (shared across all buttons) ===
    private static Stack<string> sceneHistory = new Stack<string>();
    public static string currentScene = "";
    public static string LastGameplayScene = "";
    public static string LastEnteredLevel = ""; // set when entering any level
    public static string LastClearedLevel = ""; // set only when a level is beaten

    // Track if we've set the initial scene THIS play session
    private static int lastFrameCount = -1;

    // === OVERLAY STATE ===
    public static bool ShowQuitOverlay = false;
    public static bool ShowPauseOverlay = false;
    public static bool ShowHowToPlayOverlay = false;
    public static int HtpPage = 1; // 1, 2, or 3
    private static bool openedSettingsFromPause  = false;
    private static bool stageClearPaused = false;
    private static bool s_appliedGamePause = false;

    // === SETTINGS OVERLAY (inside MainMenu scene) ===
    public static bool ShowSettingsOverlay = false;
    public static bool ShowLevelSelectOverlay = false;
    public static string SelectedLevelScene = "Level1";

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

    private static bool wasEscPressed = false;

    // Gamepad button constants
    private const int GP_CROSS    = 0; // confirm / click
    private const int GP_CIRCLE   = 1; // cancel / back
    private const int GP_START    = 7; // Options button — pause toggle
    private const int GP_DPAD_UP    = 11;
    private const int GP_DPAD_RIGHT = 12;
    private const int GP_DPAD_DOWN  = 13;
    private const int GP_DPAD_LEFT  = 14;

    // D-pad navigation state (shared across all button instances)
    private static readonly List<NavigationButtons> s_navCandidates = new List<NavigationButtons>();
    private static readonly List<NavigationButtons> s_prevCandidates = new List<NavigationButtons>();
    private static NavigationButtons? s_focusedButton = null;
    private static int s_navClearFrame   = -1;
    private static int s_navHandledFrame = -1;

    private static bool waitingForSceneTransition = false;
    private static string pendingScene = "";

    public override void Update()
    {
        // Reset history when a new Play session starts (frame count resets to 0)
        int currentFrame = Time.FrameCount;
        if (currentFrame < lastFrameCount || lastFrameCount < 0)
        {
            sceneHistory.Clear();
            currentScene = "";
            LastGameplayScene = "";
            LastClearedLevel = LevelProgressStore.LoadLastClearedLevel();

            ShowQuitOverlay = false;
            ShowPauseOverlay = false;
            ShowHowToPlayOverlay = false;
            HtpPage = 1;
            ShowSettingsOverlay = false;
            ShowLevelSelectOverlay = false;
            SelectedLevelScene = "Level1";

            wasEscPressed = false;
            s_appliedGamePause = false;

            waitingForSceneTransition = false;
            pendingScene = "";

            s_navCandidates.Clear();
            s_prevCandidates.Clear();
            s_focusedButton    = null;
            s_navClearFrame    = -1;
            s_navHandledFrame  = -1;
        }

        if (currentFrame != lastFrameCount)
        {
            wasEscPressed = false;
        }

        lastFrameCount = currentFrame;

        InputComponent input = GetInput();
        TransformComponent t = GetTransform();

        string realScene = Scene.GetCurrentScene();

        if (waitingForSceneTransition)
        {
            AudioComponent audio = GetAudio();

            // Go next only after BGM has finished/stopped
            if (!audio.IsBGMPlaying())
            {
                waitingForSceneTransition = false;

                string targetScene = pendingScene;
                pendingScene = "";

                if (!string.IsNullOrEmpty(targetScene))
                    NavigateTo(targetScene);
            }

            return;
        }

        // Keep tracked scene in sync with the actual loaded scene (fixes Cutscene -> Level1 issue)
        if (!string.IsNullOrEmpty(realScene) && currentScene != realScene)
        {
            currentScene = realScene;
        }

        // ESC or gamepad Options button → pause toggle (only in gameplay scenes)
        bool pauseTriggered = (input.IsKeyTriggered(256) && !wasEscPressed)  // 256 = ESC
                           || input.IsGamepadButtonTriggered(GP_START);
        if (pauseTriggered)
        {
            wasEscPressed = true;

            if (!openedSettingsFromPause &&
                !string.IsNullOrEmpty(realScene) &&
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
            else if (ShowLevelSelectOverlay)
            {
                if (isBackBtn) shouldBeVisible = false;
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
            bool isPauseMenuOpen = ShowPauseOverlay;
            bool isPauseSettingsOpen = ShowSettingsOverlay && openedSettingsFromPause;
            bool shouldGameBePaused = isPauseMenuOpen || isPauseSettingsOpen || stageClearPaused;

            if (isPauseMenuOpen != lastPauseState)
            {
                SpriteComponent sprite = GetSprite();

                if (isPauseMenuOpen)
                {
                    sprite.Texture = exitTexture;
                    defaultTexture = exitTexture;
                    hoverTexture = exitHoverTexture;
                }
                else
                {
                    sprite.Texture = pauseTexture;
                    defaultTexture = pauseTexture;
                    hoverTexture = pauseHoverTexture;
                }

                lastPauseState = isPauseMenuOpen;
            }

            // Apply actual pause state once
            if (shouldGameBePaused != s_appliedGamePause)
            {
                Application.SetPaused(shouldGameBePaused);

                AudioComponent audio = GetAudio();

                if (shouldGameBePaused)
                {
                    audio.PauseBGM();
                }
                else
                {
                    audio.FadeInMusic(0.5f);
                }

                s_appliedGamePause = shouldGameBePaused;
            }
        }

        string texLower = defaultTexture.ToLower();
        bool isQuitOverlayButton = texLower.Contains("quit yes") || texLower.Contains("quit no");
        bool isPauseOverlayButton = texLower.Contains("pause") || texLower.Contains("exit");
        
        bool isHowToPlayOverlayButton = texLower.Contains("temp_close") || texLower.Contains("close_htp") ||
            texLower.Contains("exit") || texLower.Contains("nextstage");

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
        if (ShowHowToPlayOverlay && !isHowToPlayOverlayButton) return;

        float currentScaleX = t.ScaleX;
        float currentScaleY = t.ScaleY;
        float baseScaleX = isCurrentlyHovered ? currentScaleX / hoverScaleBoost : currentScaleX;
        float baseScaleY = isCurrentlyHovered ? currentScaleY / hoverScaleBoost : currentScaleY;

        float halfW = baseScaleX / 2f;
        float halfH = baseScaleY / 2f;
        float mx = input.WorldMouseX;
        float my = input.WorldMouseY;

        // --- Register as D-pad nav candidate (visible, passed all overlay filters) ---
        if (s_navClearFrame != currentFrame)
        {
            s_prevCandidates.Clear();
            s_prevCandidates.AddRange(s_navCandidates);
            s_navCandidates.Clear();
            s_navClearFrame = currentFrame;
        }
        s_navCandidates.Add(this);

        // --- Handle D-pad navigation once per frame, using previous frame's candidates ---
        if (s_navHandledFrame != currentFrame)
        {
            s_navHandledFrame = currentFrame;

            // D-pad navigation only active in menus / overlays — not during gameplay,
            // to prevent D-pad game inputs from accidentally focusing UI buttons.
            bool inMenuOrOverlay = realScene == "MainMenu" || realScene == "Settings" ||
                                   realScene == "HowToPlay" ||
                                   realScene == "LatencyCalibration" ||
                                   realScene == "VisualLatencyCalibration" ||
                                   ShowPauseOverlay || ShowQuitOverlay ||
                                   ShowSettingsOverlay || ShowHowToPlayOverlay;

            // Clear any lingering focus when returning to gameplay
            if (!inMenuOrOverlay && s_focusedButton != null)
                s_focusedButton = null;

            bool navFwd  = inMenuOrOverlay && (input.IsGamepadButtonTriggered(GP_DPAD_DOWN)  || input.IsGamepadButtonTriggered(GP_DPAD_RIGHT));
            bool navBack = inMenuOrOverlay && (input.IsGamepadButtonTriggered(GP_DPAD_UP)    || input.IsGamepadButtonTriggered(GP_DPAD_LEFT));
            if (navFwd || navBack)
            {
                int count = s_prevCandidates.Count;
                if (count > 0)
                {
                    int idx = s_focusedButton != null ? s_prevCandidates.IndexOf(s_focusedButton) : -1;
                    if (idx < 0) idx = navFwd ? -1 : 0;
                    idx = navFwd ? (idx + 1) % count : (idx - 1 + count) % count;
                    s_focusedButton = s_prevCandidates[idx];
                }
            }
        }

        // Hover: mouse bounds check OR gamepad D-pad focus
        bool isMouseHovered = input.InGameViewport && mx >= t.X - halfW && mx <= t.X + halfW && my >= t.Y - halfH && my <= t.Y + halfH;
        bool isGamepadFocused = (this == s_focusedButton);

        // Mouse movement releases gamepad focus so the two modes don't fight
        if (isMouseHovered && !isGamepadFocused && s_focusedButton != null)
            s_focusedButton = null;

        bool isHovered = isMouseHovered || isGamepadFocused;

        if (isHovered && !isCurrentlyHovered)
        {
            SpriteComponent sprite = GetSprite();
            TransformComponent transform = GetTransform();
            sprite.Texture = hoverTexture;
            transform.ScaleX = currentScaleX * hoverScaleBoost;
            transform.ScaleY = currentScaleY * hoverScaleBoost;
            isCurrentlyHovered = true;

            PlayHoverSound();
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

        // Click: left mouse button OR gamepad Cross button while hovering
        if (isHovered && (input.IsMouseButtonTriggered(0) || input.IsGamepadButtonTriggered(GP_CROSS)))
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
        {
            ShowSettingsOverlay = false;
            ShowLevelSelectOverlay = false;
        }

        if (!string.IsNullOrEmpty(currentScene))
        {
            sceneHistory.Push(currentScene);
        }  

        if (newScene.ToLower().Contains("level"))
            LastEnteredLevel = newScene;

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
        ShowLevelSelectOverlay = false;

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
        ShowLevelSelectOverlay = false;

        ShowSettingsOverlay = true;
    }

    private static void HideSettings()
    {
        ShowSettingsOverlay = false;
        ShowLevelSelectOverlay = false;

        if (openedSettingsFromPause)
        {
            ShowPauseOverlay = true;      // return to pause menu
            Application.SetPaused(true);  // keep game paused
            openedSettingsFromPause = false;
        }
    }

    public static void SetStageClearPaused(bool value)
    {
        stageClearPaused = value;
    }

    public static void ReportLevelCleared(string sceneName)
    {
        if (string.IsNullOrEmpty(sceneName))
            return;

        if (!sceneName.ToLower().Contains("level"))
            return;

        int newLevel = ExtractTrailingNumber(sceneName);
        int prevLevel = ExtractTrailingNumber(LastClearedLevel);

        // Keep the highest unlocked level progression.
        if (newLevel > prevLevel)
        {
            LastClearedLevel = sceneName;
            LevelProgressStore.SaveLastClearedLevel(sceneName);
        }
    }

    private static int ExtractTrailingNumber(string sceneName)
    {
        if (string.IsNullOrEmpty(sceneName))
            return 0;

        int i = sceneName.Length - 1;
        while (i >= 0 && char.IsDigit(sceneName[i]))
            i--;

        if (i == sceneName.Length - 1)
            return 0;

        string numberPart = sceneName.Substring(i + 1);
        if (int.TryParse(numberPart, out int levelNum))
            return levelNum;

        return 0;
    }

    private bool TryGetNextLevelSceneName(string sceneName, out string nextScene)
    {
        nextScene = "";

        if (string.IsNullOrEmpty(sceneName))
            return false;

        int i = sceneName.Length - 1;
        while (i >= 0 && char.IsDigit(sceneName[i]))
            i--;

        // no trailing number
        if (i == sceneName.Length - 1)
            return false;

        string prefix = sceneName.Substring(0, i + 1);
        string numberPart = sceneName.Substring(i + 1);

        int levelNumber;
        if (!int.TryParse(numberPart, out levelNumber))
            return false;

        nextScene = prefix + (levelNumber + 1).ToString();
        return true;
    }

    private void ContinueToNextLevel()
    {
    // Sync with actual loaded scene first (important for Cutscene -> Level1 flow)
    string realScene = Scene.GetCurrentScene();
    if (!string.IsNullOrEmpty(realScene))
        currentScene = realScene;

    if (string.IsNullOrEmpty(currentScene))
    {
        Application.SetPaused(false);
        GoToMainMenu();
        return;
    }

    string nextScene;
    if (!TryGetNextLevelSceneName(currentScene, out nextScene))
    {
        Application.SetPaused(false);
        GoToMainMenu();
        return;
    }

    // Check once only (avoid duplicate checks / duplicate debug prints)
    bool nextSceneExists = Scene.SceneExists(nextScene);

    Console.WriteLine("ContinueToNextLevel -> currentScene: " + currentScene);
    Console.WriteLine("ContinueToNextLevel -> realScene: " + realScene);
    Console.WriteLine("ContinueToNextLevel -> nextScene: " + nextScene);
    Console.WriteLine("ContinueToNextLevel -> exists?: " + nextSceneExists);

    if (!nextSceneExists)
    {
        Console.WriteLine("Next scene does not exist: " + nextScene + " -> going MainMenu");

        // Make sure game is not stuck paused when falling back
        ShowQuitOverlay = false;
        ShowPauseOverlay = false;
        ShowHowToPlayOverlay = false;
        ShowSettingsOverlay = false;
        openedSettingsFromPause = false;
        Application.SetPaused(false);

        GoToMainMenu();
        return;
    }

    // Clear UI/overlay states before changing scene
    ShowQuitOverlay = false;
    ShowPauseOverlay = false;
    ShowHowToPlayOverlay = false;
    ShowSettingsOverlay = false;
    openedSettingsFromPause = false;

    // Make sure stage clear / gameplay freeze is removed before next scene
    Application.SetPaused(false);

    currentScene = nextScene;

    // Load loading screen first then the nxt level
    LoadingScreenScript.NextLevel = currentScene;
    Scene.LoadScene("LoadingScreenScene");
    //Scene.LoadScene(nextScene);
}


    private void OnClick()
    {
        string tex = defaultTexture.ToLower();
        Console.WriteLine($"Button clicked: {tex}");

        // HTP overlay: handle Exit (close overlay) and NextStage (advance page) before generic handlers
        if (ShowHowToPlayOverlay)
        {
            if (tex.Contains("exit") || tex.Contains("temp_close") || tex.Contains("close_htp"))
            {
                ShowHowToPlayOverlay = false;
                HtpPage = 1;
                return;
            }
            if (tex.Contains("nextstage"))
            {
                if (HtpPage < 3) HtpPage++;
                return;
            }
        }

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
            ShowPauseOverlay = false;
            Application.SetPaused(false);
            if (Scene.GetCurrentScene() == "LatencyCalibration")
            {
                LatencyCalibrator.Instance?.Reset();
                return;
            }
            if (Scene.GetCurrentScene() == "VisualLatencyCalibration")
            {
                VisualLatencyCalibrator.Instance?.Reset();
                return;
            }
            //Scene.RestartScene();

            // make it load loading screen when we restart instead of it freezing at the pause menu
            LoadingScreenScript.NextLevel = Scene.GetCurrentScene();
            Scene.LoadScene("LoadingScreenScene");

            return;
        }

        if (tex.Contains("pausesettings"))
        {
            openedSettingsFromPause = true;
            ShowPauseOverlay = false;
            ShowSettingsOverlay = true;
            return;
        }

        if (tex.Contains("restartyes"))
        {
            Application.SetPaused(false);

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
            if (!waitingForSceneTransition)
            {
                ShowLevelSelectOverlay = false;
                GetAudio().FadeOutMusic(0.4f);
                waitingForSceneTransition = true;
                pendingScene = "Cutscene";
            }
            return;
        }

        if (tex.Contains("continue"))
        {
            if (Scene.GetCurrentScene() == "MainMenu")
            {
                ShowQuitOverlay = false;
                ShowPauseOverlay = false;
                ShowHowToPlayOverlay = false;
                ShowSettingsOverlay = false;
                ShowLevelSelectOverlay = true;
                SelectedLevelScene = "Level1";
                return;
            }

            string sceneToLoad = "";

            if (!string.IsNullOrEmpty(LastClearedLevel))
            {
                // Player has cleared at least one level — go to the next one
                string nextScene;
                if (TryGetNextLevelSceneName(LastClearedLevel, out nextScene) && Scene.SceneExists(nextScene))
                    sceneToLoad = nextScene;
                else
                    sceneToLoad = LastClearedLevel; // no next level, replay last cleared
            }
            else if (!string.IsNullOrEmpty(LastEnteredLevel))
            {
                // Player paused and quit — return to the level they were in
                sceneToLoad = LastEnteredLevel;
            }
            else
            {
                // Nothing played yet — same as New Game
                sceneToLoad = "Cutscene";
            }

            NavigateTo(sceneToLoad);
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
            if (Scene.GetCurrentScene() == "LatencyCalibration")
            {
                LatencyCalibrator.Instance?.Skip();
                return;
            }
            if (Scene.GetCurrentScene() == "VisualLatencyCalibration")
            {
                VisualLatencyCalibrator.Instance?.Skip();
                return;
            }
            StageClearElement.ResetStageClear();
            ReportLevelCleared(currentScene);
            ContinueToNextLevel();
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

public class HowToPlayPage1 : Script
{
    public override void Update()
    {
        TransformComponent t = GetTransform();
        t.IsVisible = NavigationButtons.ShowHowToPlayOverlay && NavigationButtons.HtpPage == 1;
    }
}

public class HowToPlayPage2 : Script
{
    public override void Update()
    {
        TransformComponent t = GetTransform();
        t.IsVisible = NavigationButtons.ShowHowToPlayOverlay && NavigationButtons.HtpPage == 2;
    }
}

public class HowToPlayPage3 : Script
{
    public override void Update()
    {
        TransformComponent t = GetTransform();
        t.IsVisible = NavigationButtons.ShowHowToPlayOverlay && NavigationButtons.HtpPage == 3;
    }
}

// Visible when HTP overlay is open AND there is a next page (pages 1 and 2)
public class HtpNextButton : Script
{
    public override void Update()
    {
        TransformComponent t = GetTransform();
        t.IsVisible = NavigationButtons.ShowHowToPlayOverlay && NavigationButtons.HtpPage < 3;
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

internal static class LevelProgressStore
{
    private const string SaveFileName = "level_progress.json";

    private static readonly string[] CandidatePaths = new[]
    {
        Path.Combine("PulseProtocol", "JSON", SaveFileName),
        Path.Combine("..", "..", "PulseProtocol", "JSON", SaveFileName),
        Path.Combine("..", "..", "..", "PulseProtocol", "JSON", SaveFileName),
        SaveFileName
    };

    private sealed class ProgressData
    {
        public string LastClearedLevel { get; set; } = "";
    }

    public static string LoadLastClearedLevel()
    {
        foreach (string path in CandidatePaths)
        {
            try
            {
                if (!File.Exists(path))
                    continue;

                string json = File.ReadAllText(path);
                ProgressData? data = JsonSerializer.Deserialize<ProgressData>(json);
                if (!string.IsNullOrWhiteSpace(data?.LastClearedLevel))
                    return data.LastClearedLevel!;
            }
            catch
            {
                // Try next candidate path.
            }
        }

        return "";
    }

    public static void SaveLastClearedLevel(string sceneName)
    {
        if (string.IsNullOrWhiteSpace(sceneName))
            return;

        string json = JsonSerializer.Serialize(
            new ProgressData { LastClearedLevel = sceneName },
            new JsonSerializerOptions { WriteIndented = true });

        foreach (string path in CandidatePaths)
        {
            try
            {
                string? dir = Path.GetDirectoryName(path);
                if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                    continue;

                File.WriteAllText(path, json);
                return;
            }
            catch
            {
                // Try next candidate path.
            }
        }
    }
}
