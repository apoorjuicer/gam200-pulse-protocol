/******************************************************************************/
/**
* @file        CutscenePanel.cs
* @project     Pulse Protocol
* @author      Carrie Lam (83%)
* @author      Chloe Lau (13%)
* @author      Leu Jun Yong (2%)
* @author      Goh Pin Kai (2%)
* @brief       Controls staggered slide-in and simultaneous slide-out animations
*              for paired cutscene panels, with shared timing state across all
*              panel instances and space-to-skip support.
*
* @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/

using System;
using ScriptAPI;

/// <summary>
/// Cutscene Panel Controller (Paired + Staggered)
/// =================================================
/// Attach this script to EVERY panel entity (Panel1 through Panel8).
///
/// Pairs: (1+2), (3+4), (5+6), (7+8)
/// 
/// Entry: Left panel slides in first, then right panel follows after a short stagger.
/// Exit:  Both panels slide upward together simultaneously.
/// 
/// POSITIONING:
///   Set 'targetX' on each panel entity in the inspector to control where it sits.
///   - Left panels (1,3,5...): use negative values like -700
///   - Right panels (2,4,6...): use positive values like 700
///   - Adjust per panel to account for different panel sizes
///   If targetX is left at 0, it will use the defaults (-700 / 700).
///
/// Press SPACE to skip. Loads Level1 when done.
/// </summary>
public class CutscenePanel : Script
{
    // =========================
    // Timing Configuration
    // =========================
    public float slideInDuration = 0.8f;     // Time for one panel to slide into view
    public float staggerDelay = 1.5f;        // Delay before the second panel in a pair starts sliding in
    public float holdDuration = 1.8f;        // Time pair stays fully visible (after both are in)
    public float slideOutDuration = 0.8f;    // Time for both to slide out together
    public float overlapDuration = 0.6f;     // Overlap between current pair exiting and next pair entering

    // =========================
    // Position Configuration (per-panel)
    // =========================
    // Set this in the inspector for each panel entity.
    // Left panels should be negative (e.g. -700), right panels positive (e.g. 700).
    // If left at 0, defaults will be used.
    public float targetX = 0f;

    // Defaults used when targetX is not set (left at 0)
    public float defaultLeftX = -700f;
    public float defaultRightX = 700f;

    public float centerY = 0f;
    public float offScreenBelow = -900f;
    public float offScreenAbove = 900f;

    // =========================
    // Static Shared State
    // =========================
    private static float sharedElapsedTime = 0f;
    private static bool sharedCutsceneFinished = false;
    private static int sharedInitCount = 0;
    private static int totalPairs = 4;
    private static bool s_waitingForSkipTransition = false;
    private static float s_skipTransitionTimer = 0f;
    private const float SkipTransitionDuration = 0.5f;
    private static bool s_sceneChangeIssued = false;
    private static string s_sceneSessionTag = "";
    private static bool s_waitingForEntryFade = true;

    public static bool IsCutsceneFinished => sharedCutsceneFinished;

    // Cached timing values (set once during init)
    private static float sPairTotalDuration = 0f;
    private static float sStaggerDelay = 0f;
    private static float sSlideInDuration = 0f;
    private static float sHoldDuration = 0f;
    private static float sSlideOutDuration = 0f;
    private static float sOverlapDuration = 0f;
    private static float sOffScreenBelow = 0f;
    private static float sOffScreenAbove = 0f;
    private static float sCenterY = 0f;

    // =========================
    // Per-Instance State
    // =========================
    private int myPanelIndex = -1;
    private int myPairIndex = -1;
    private bool isLeftPanel = true;
    private bool initialized = false;
    private bool isTimerOwner = false;
    private float myTargetX = 0f;

    public override void Update()
    {
        NavigationButtons.TryProcessPendingPostLoadFadeIn();

        if (!initialized)
        {
            Initialize();
            return;
        }

        if (WaitForEntryFadeComplete())
        {
            HoldPanelHidden();
            return;
        }

        if (isTimerOwner && s_waitingForSkipTransition)
        {
            s_skipTransitionTimer -= Time.DeltaTime;
            bool timeDone = s_skipTransitionTimer <= 0f;
            bool screenFadeDone = FadeAPI.IsFadeDone();

            if (timeDone)
                FadeAPI.SetAlpha(1.0f);

            if (timeDone && screenFadeDone)
            {
                s_waitingForSkipTransition = false;
                s_skipTransitionTimer = 0f;
                GoToLevel1AfterCutscene();
            }
            return;
        }

        if (sharedCutsceneFinished)
            return;

        if (isTimerOwner)
        {
            InputComponent input = GetInput();
            if (input.IsKeyTriggered(32) || input.IsGamepadButtonTriggered(0))
            {
                Console.WriteLine("[Cutscene] Skipped!");
                BeginSkipTransition();
                return;
            }

            sharedElapsedTime += Time.DeltaTime;

            float totalDuration = (totalPairs - 1) * (sPairTotalDuration - sOverlapDuration) + sPairTotalDuration;
            if (sharedElapsedTime >= totalDuration)
            {
                Console.WriteLine("[Cutscene] Complete! Loading Level1...");
                GoToLevel1AfterCutscene();
                return;
            }
        }

        AnimatePanel();
    }

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        // Reset shared state when scene changes OR when this is the very first panel
        // initializing (sharedInitCount == 0), which means it's a fresh scene load
        // even if the session tag wasn't cleared (e.g. OnDestroy not called on scene change).
        string currentScene = Scene.GetCurrentScene();
        bool sceneChanged = !string.Equals(s_sceneSessionTag, currentScene, StringComparison.OrdinalIgnoreCase);
        bool freshStart = sharedInitCount == 0 || sharedInitCount >= totalPairs * 2;
        if (sceneChanged || freshStart)
        {
            s_sceneSessionTag = currentScene;
            sharedInitCount = 0;
            sharedElapsedTime = 0f;
            sharedCutsceneFinished = false;
            s_waitingForSkipTransition = false;
            s_skipTransitionTimer = 0f;
            s_sceneChangeIssued = false;
            s_waitingForEntryFade = true;
        }

        //Console.WriteLine($"[DAS PRETTY GAY IF U ASK ME] sharedInitCount: {sharedInitCount}, sharedCutsceneFinished: {sharedCutsceneFinished}\n\n\n\n\n\n\n");

        myPanelIndex = sharedInitCount;
        sharedInitCount++;

        //Console.WriteLine($"[PEEPEEPOOPOO] Panel: {myPanelIndex} | Pair: {myPanelIndex / 2}\n\n\n\n\n\n");


        // Pair info
        myPairIndex = myPanelIndex / 2;
        isLeftPanel = (myPanelIndex % 2) == 0;

        // Read X and Y directly from the saved transform position
        TransformComponent savedTf = GetTransform();
        myTargetX = savedTf.X;
        centerY = savedTf.Y;

        // First panel sets up shared state
        if (myPanelIndex == 0)
        {
            sharedElapsedTime = 0f;
            sharedCutsceneFinished = false;
            isTimerOwner = true;

            // Cache timing values from this panel's config (all panels should have the same timing)
            sSlideInDuration = slideInDuration;
            sStaggerDelay = staggerDelay;
            sHoldDuration = holdDuration;
            sSlideOutDuration = slideOutDuration;
            sOverlapDuration = overlapDuration;
            sOffScreenBelow = offScreenBelow;
            sOffScreenAbove = offScreenAbove;
            sCenterY = centerY;

            // Pair total = stagger + slideIn + hold + slideOut
            // The pair "starts" when the LEFT panel begins sliding in.
            // RIGHT panel starts sliding in after staggerDelay.
            // Hold begins after BOTH panels are in (i.e. after stagger + slideIn).
            // Both exit together after hold.
            sPairTotalDuration = sStaggerDelay + sSlideInDuration + sHoldDuration + sSlideOutDuration;

            float totalDuration = (totalPairs - 1) * (sPairTotalDuration - sOverlapDuration) + sPairTotalDuration;
            Console.WriteLine($"[Cutscene] Timer owner: Panel 1");
            Console.WriteLine($"[Cutscene] Pair duration: {sPairTotalDuration}s (stagger:{sStaggerDelay} + in:{sSlideInDuration} + hold:{sHoldDuration} + out:{sSlideOutDuration})");
            Console.WriteLine($"[Cutscene] Overlap: {sOverlapDuration}s | Pairs: {totalPairs}");
            Console.WriteLine($"[Cutscene] Total: {totalDuration:F1}s");
        }

        // Start off-screen and invisible
        TransformComponent tf = GetTransform();
        tf.X = myTargetX;
        tf.Y = offScreenBelow;
        tf.IsVisible = false;

        string side = isLeftPanel ? "LEFT" : "RIGHT";
        Console.WriteLine($"[CutscenePanel] Panel {myPanelIndex + 1} -> Pair {myPairIndex + 1} ({side}) X={myTargetX}");

        initialized = true;
    }

    private bool WaitForEntryFadeComplete()
    {
        if (!s_waitingForEntryFade)
            return false;

        string currentScene = Scene.GetCurrentScene();
        if (NavigationButtons.IsPostLoadFadePendingForScene(currentScene))
            return true;

        if (!FadeAPI.IsFadeDone())
            return true;

        s_waitingForEntryFade = false;
        return false;
    }

    private void HoldPanelHidden()
    {
        TransformComponent tf = GetTransform();
        SpriteComponent sprite = GetSprite();
        tf.IsVisible = false;
        tf.X = myTargetX;
        tf.Y = sOffScreenBelow != 0f ? sOffScreenBelow : offScreenBelow;
        SetAlpha(sprite, 0f);
    }

    private void GoToLevel1AfterCutscene()
    {
        if (s_sceneChangeIssued)
            return;

        s_sceneChangeIssued = true;
        sharedCutsceneFinished = true;
        NavigationButtons.ShowLevelSelectOverlay = false;
        s_sceneSessionTag = "";

        const string targetScene = "Level1";
        NavigationButtons.currentScene = targetScene;
        NavigationButtons.LastEnteredLevel = targetScene;

        if (NavigationButtons.ShouldUseLoadingScreenForScene(targetScene))
        {
            LoadingScreenScript.NextLevel = targetScene;
            Scene.LoadScene("LoadingScreenScene");
            return;
        }

        NavigationButtons.QueuePostLoadFadeIn(targetScene, 0.5f);
        Scene.LoadScene(targetScene);
    }

    private void BeginSkipTransition()
    {
        if (s_waitingForSkipTransition)
            return;

        sharedCutsceneFinished = true;
        s_waitingForSkipTransition = true;
        s_skipTransitionTimer = SkipTransitionDuration;
        FadeAPI.Stop();
        FadeAPI.SetAlpha(0.0f);
        FadeAPI.FadeOut(SkipTransitionDuration);
    }

    // =========================
    // Animation
    // =========================
    private void AnimatePanel()
    {
        if (myPanelIndex < 0) return;

        TransformComponent tf = GetTransform();
        SpriteComponent sprite = GetSprite();

        // When does this pair start?
        float pairStartTime = myPairIndex * (sPairTotalDuration - sOverlapDuration);
        float localTime = sharedElapsedTime - pairStartTime;

        // Timeline for a pair:
        //
        //  0.0                        stagger   stagger+slideIn   stagger+slideIn+hold   pairTotal
        //  |--- LEFT slides in ---------|-----------|                    |                    |
        //                               |--- RIGHT slides in ---|       |                    |
        //                                                               |--- BOTH slide out -|
        //
        // LEFT  starts sliding at localTime = 0
        // RIGHT starts sliding at localTime = staggerDelay
        // Hold starts at localTime = staggerDelay + slideInDuration (both are in)
        // Both exit at localTime = staggerDelay + slideInDuration + holdDuration

        float mySlideStart;
        if (isLeftPanel)
            mySlideStart = 0f;                 // Left panel enters immediately
        else
            mySlideStart = sStaggerDelay;       // Right panel enters after stagger

        float mySlideEnd = mySlideStart + sSlideInDuration;
        float holdStart = sStaggerDelay + sSlideInDuration;  // Both are in after this
        float holdEnd = holdStart + sHoldDuration;
        float exitEnd = sPairTotalDuration;

        // Before this pair starts
        if (localTime < 0f)
        {
            tf.IsVisible = false;
            tf.X = myTargetX;
            tf.Y = sOffScreenBelow;
            SetAlpha(sprite, 0f);
            return;
        }

        // After this pair is done
        if (localTime >= exitEnd)
        {
            tf.IsVisible = false;
            tf.X = myTargetX;
            tf.Y = sOffScreenAbove;
            SetAlpha(sprite, 0f);
            return;
        }

        // === BEFORE MY SLIDE STARTS (only applies to right panel during stagger) ===
        if (localTime < mySlideStart)
        {
            tf.IsVisible = false;
            tf.X = myTargetX;
            tf.Y = sOffScreenBelow;
            SetAlpha(sprite, 0f);
            return;
        }

        // === Panel is active from here ===
        tf.IsVisible = true;
        tf.X = myTargetX;

        if (localTime < mySlideEnd)
        {
            // === SLIDING IN ===
            float t = (localTime - mySlideStart) / sSlideInDuration;
            float eased = EaseOutCubic(t);

            tf.Y = Lerp(sOffScreenBelow, centerY, eased);
            SetAlpha(sprite, Lerp(0.2f, 1f, eased));
        }
        else if (localTime < holdEnd)
        {
            // === HOLDING (or waiting for other panel to finish sliding in) ===
            tf.Y = centerY;
            SetAlpha(sprite, 1f);
        }
        else
        {
            // === SLIDING OUT (both panels exit together) ===
            float t = (localTime - holdEnd) / sSlideOutDuration;
            float eased = EaseInCubic(t);

            tf.Y = Lerp(centerY, sOffScreenAbove, eased);
            SetAlpha(sprite, Lerp(1f, 0f, eased));
        }
    }

    // =========================
    // Helpers
    // =========================
    private void SetAlpha(SpriteComponent sprite, float alpha)
    {
        if (sprite.HasSprite())
            sprite.TintA = alpha;
    }

    private float Lerp(float a, float b, float t)
    {
        if (t < 0f) t = 0f;
        if (t > 1f) t = 1f;
        return a + (b - a) * t;
    }

    private float EaseOutCubic(float t)
    {
        float f = 1f - t;
        return 1f - f * f * f;
    }

    private float EaseInCubic(float t)
    {
        return t * t * t;
    }

    public void OnDestroy()
    {
        // Reset static state so the next scene load always triggers a full reinitialize.
        s_sceneSessionTag = "";
        sharedInitCount = 0;
        sharedCutsceneFinished = false;
        s_waitingForSkipTransition = false;
        s_sceneChangeIssued = false;
        s_waitingForEntryFade = true;
    }
}
