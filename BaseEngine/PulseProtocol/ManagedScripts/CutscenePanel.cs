/******************************************************************************/
/**
* @file        CutscenePanel.cs
* @project     Pulse Protocol
* @author      Carrie Lam (85%)
*              Chloe Lau (15%)
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

public class CutscenePanel : Script
{
    // =========================
    // Timing Configuration
    // =========================
    public float slideInDuration = 0.8f;     // Time for one panel to slide into view
    public float staggerDelay = 0.3f;        // Delay before the second panel in a pair starts sliding in
    public float holdDuration = 2.0f;        // Time pair stays fully visible (after both are in)
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
    private static int totalPairs = 7;

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
    private float mySlideStartInGroup = 0f;  // when within the group this panel starts sliding in
    private float myHoldStart = 0f;          // when all panels in this group are fully in

    public override void Update()
    {
        if (!initialized)
        {
            Initialize();
            return;
        }

        if (sharedCutsceneFinished)
            return;

        if (isTimerOwner)
        {
            InputComponent input = GetInput();
            if (input.IsKeyTriggered(32))
            {
                Console.WriteLine("[Cutscene] Skipped!");
                sharedCutsceneFinished = true;
                NavigationButtons.LastEnteredLevel = "Level1";

                //(Im making it load loading screen scene then after about 3 frames go 2 lvl 1)
                // redefine this var to lvl 1 so in the LoadingScreenScript script, it will load this ltrrrrrr
                LoadingScreenScript.NextLevel = "Level1"; 
                Scene.LoadScene("LoadingScreenScene");
                //Scene.LoadScene("Level1");

                return;
            }

            sharedElapsedTime += Time.DeltaTime;

            float totalDuration = (totalPairs - 1) * (sPairTotalDuration - sOverlapDuration) + sPairTotalDuration;
            if (sharedElapsedTime >= totalDuration)
            {
                sharedCutsceneFinished = true;
                Console.WriteLine("[Cutscene] Complete! Loading Level1...");
                NavigationButtons.LastEnteredLevel = "Level1";

                // Same for here, look up^^^
                LoadingScreenScript.NextLevel = "Level1";
                Scene.LoadScene("LoadingScreenScene");
                //Scene.LoadScene("Level1");
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
        // reset the count back 2 0 if u finish the cutscene. Pressing space also make it true soooooo, yeah das all folks :>
        if (sharedCutsceneFinished)
        {
            sharedInitCount = 0;
            sharedCutsceneFinished = false;
        }

        //Console.WriteLine($"[DAS PRETTY GAY IF U ASK ME] sharedInitCount: {sharedInitCount}, sharedCutsceneFinished: {sharedCutsceneFinished}\n\n\n\n\n\n\n");

        myPanelIndex = sharedInitCount;
        sharedInitCount++;

        //Console.WriteLine($"[PEEPEEPOOPOO] Panel: {myPanelIndex} | Pair: {myPanelIndex / 2}\n\n\n\n\n\n");


        // Pair/group assignment
        // Panel 2 (index 2) joins group 0 as the third panel, staggered after panels 0 and 1.
        // Panel 3 (index 3) becomes the first (left) panel of group 1 since index 2 vacated it.
        // All other panels use the default index/2 formula.
        myPairIndex = myPanelIndex / 2;
        isLeftPanel = (myPanelIndex % 2) == 0;

        // Use instance fields directly here — sStaggerDelay/sSlideInDuration may not be set yet
        // (panel 0 sets the statics later in its init block, after this runs).
        float sd = staggerDelay;
        float si = slideInDuration;

        if (myPanelIndex <= 1)
        {
            // Group 0 normal panels: hold start delayed to wait for the third panel (index 2)
            mySlideStartInGroup = isLeftPanel ? 0f : sd;
            myHoldStart = sd * 2 + si;
        }
        else if (myPanelIndex == 2)
        {
            // Third panel of group 0: slides in at staggerDelay*2
            myPairIndex = 0;
            mySlideStartInGroup = sd * 2;
            myHoldStart = sd * 2 + si;
        }
        else if (myPanelIndex == 3)
        {
            // First panel of group 1 (was right panel of a pair — now leads solo)
            mySlideStartInGroup = 0f;
            myHoldStart = sd + si;
        }
        else
        {
            // All remaining panels: default stagger and hold
            mySlideStartInGroup = isLeftPanel ? 0f : sd;
            myHoldStart = sd + si;
        }

        // Determine this panel's X position
        if (targetX != 0f)
        {
            // Use the value set in the inspector
            myTargetX = targetX;
        }
        else
        {
            // Use defaults
            myTargetX = isLeftPanel ? defaultLeftX : defaultRightX;
        }

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

            // Pair total: for group 0 (triple), hold starts after staggerDelay*2 + slideIn.
            // Other groups (pairs) use staggerDelay + slideIn.
            // We use the triple timing as the canonical pair duration so group 0 gets full hold time.
            sPairTotalDuration = sStaggerDelay * 2 + sSlideInDuration + sHoldDuration + sSlideOutDuration;

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

        float mySlideStart = mySlideStartInGroup;
        float mySlideEnd = mySlideStart + sSlideInDuration;
        float holdStart = myHoldStart;
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

            tf.Y = Lerp(sOffScreenBelow, sCenterY, eased);
            SetAlpha(sprite, Lerp(0.2f, 1f, eased));
        }
        else if (localTime < holdEnd)
        {
            // === HOLDING (or waiting for other panel to finish sliding in) ===
            tf.Y = sCenterY;
            SetAlpha(sprite, 1f);
        }
        else
        {
            // === SLIDING OUT (both panels exit together) ===
            float t = (localTime - holdEnd) / sSlideOutDuration;
            float eased = EaseInCubic(t);

            tf.Y = Lerp(sCenterY, sOffScreenAbove, eased);
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
}