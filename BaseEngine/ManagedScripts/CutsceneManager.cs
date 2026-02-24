using System;
using ScriptAPI;

public class CutsceneManager : Script
{
    // =========================
    // Singleton
    // =========================
    public static CutsceneManager Instance { get; private set; }

    // =========================
    // Panel Configuration
    // =========================
    public float panelDuration = 2.0f; // Duration for each panel to slide in AND out
    public float slideDistance = 1200.0f; // Distance to slide from off-screen

    public string panel1Name = "Panel1";
    public string panel2Name = "Panel2";
    public string panel3Name = "Panel3";
    public string panel4Name = "Panel4";
    public string panel5Name = "Panel5";
    public string panel6Name = "Panel6";
    public string panel7Name = "Panel7";
    public string panel8Name = "Panel8";
    public string panel9Name = "Panel9";
    public string panel10Name = "Panel10";
    public string panel11Name = "Panel11";
    public string panel12Name = "Panel12";
    public string panel13Name = "Panel13";
    public string panel14Name = "Panel14";

    // =========================
    // Computed Timeline
    // =========================
    private string[] _panelNames;
    private float[] _panelStartTimes;
    private int _totalPanels = 14;

    // =========================
    // Runtime State
    // =========================
    private float elapsedTime = 0.0f;
    private int currentPanelIndex = 0; // Which panel is currently animating
    private bool cutsceneFinished = false;
    private bool initialized = false;

    // =========================
    // Initialization
    // =========================
    public void OnCreate()
    {
        // Set singleton
        if (Instance == null)
            Instance = this;

        // Build the ordered list once
        _panelNames = new string[]
        {
            panel1Name, panel2Name, panel3Name, panel4Name, panel5Name, panel6Name, panel7Name,
            panel8Name, panel9Name, panel10Name, panel11Name, panel12Name, panel13Name, panel14Name
        };

        _panelStartTimes = new float[_panelNames.Length];

        // Start times: 0, 2, 4, ... up to Panel14
        for (int i = 0; i < _panelNames.Length; i++)
        {
            _panelStartTimes[i] = i * panelDuration;
        }

        Console.WriteLine("[CutsceneManager] Initialized with 14 panels");
        Console.WriteLine($"[CutsceneManager] Panel duration: {panelDuration}s each");
        Console.WriteLine($"[CutsceneManager] Total cutscene duration: {_totalPanels * panelDuration}s");

        initialized = true;
    }

    // =========================
    // Update Loop
    // =========================
    public override void Update()
    {
        if (!initialized)
        {
            OnCreate();
            return;
        }

        if (cutsceneFinished)
            return;

        InputComponent input = GetInput();
        if (input.IsKeyTriggered(32)) // SPACE=32
        {
            Console.WriteLine("[Cutscene] Skipped!");
            cutsceneFinished = true;
            OnCutsceneComplete();
            return;
        }

        float dt = Time.DeltaTime;
        elapsedTime += dt;

        // Update current panel index based on elapsed time
        int newPanelIndex = (int)(elapsedTime / panelDuration);

        if (newPanelIndex != currentPanelIndex && newPanelIndex < _totalPanels)
        {
            currentPanelIndex = newPanelIndex;
            Console.WriteLine($"[CutsceneManager] Now showing: {_panelNames[currentPanelIndex]} (Panel {currentPanelIndex + 1})");
        }

        // Check if cutscene is complete
        if (elapsedTime >= _totalPanels * panelDuration)
        {
            cutsceneFinished = true;
            Console.WriteLine("[CutsceneManager] Cutscene complete!");
            OnCutsceneComplete();
        }
    }

    // =========================
    // Public API for Panel Scripts
    // =========================

    /// Get the animation progress for a specific panel (0-13 for panels 1-14)
    /// Returns 0.0 before panel starts, 1.0 when complete
    public float GetPanelProgress(int panelIndex)
    {
        if (panelIndex < 0 || panelIndex >= _totalPanels)
            return 0.0f;

        float startTime = _panelStartTimes[panelIndex];
        float endTime = startTime + panelDuration;

        // Before panel starts
        if (elapsedTime < startTime)
            return 0.0f;

        // After panel ends
        if (elapsedTime >= endTime)
            return 1.0f;

        // During animation
        float localTime = elapsedTime - startTime;
        float progress = localTime / panelDuration;

        return EaseInOutQuad(progress);
    }

    /// Get the slide-in progress (first half of animation)
    /// Returns 0.0 to 1.0 during first half, then stays at 1.0

    public float GetPanelSlideInProgress(int panelIndex)
    {
        if (panelIndex < 0 || panelIndex >= _totalPanels)
            return 0.0f;

        float startTime = _panelStartTimes[panelIndex];
        float slideInEndTime = startTime + (panelDuration * 0.5f);

        // Before panel starts
        if (elapsedTime < startTime)
            return 0.0f;

        // After slide-in complete
        if (elapsedTime >= slideInEndTime)
            return 1.0f;

        // During slide-in
        float localTime = elapsedTime - startTime;
        float progress = localTime / (panelDuration * 0.5f);

        return EaseOutQuad(progress);
    }

    /// <summary>
    /// Get the slide-out progress (second half of animation)
    /// Returns 0.0 during first half, then 0.0 to 1.0 during second half
    /// </summary>
    public float GetPanelSlideOutProgress(int panelIndex)
    {
        if (panelIndex < 0 || panelIndex >= _totalPanels)
            return 0.0f;

        float startTime = _panelStartTimes[panelIndex];
        float slideOutStartTime = startTime + (panelDuration * 0.5f);
        float endTime = startTime + panelDuration;

        // Before slide-out starts
        if (elapsedTime < slideOutStartTime)
            return 0.0f;

        // After slide-out complete
        if (elapsedTime >= endTime)
            return 1.0f;

        // During slide-out
        float localTime = elapsedTime - slideOutStartTime;
        float progress = localTime / (panelDuration * 0.5f);

        return EaseInQuad(progress);
    }

    /// <summary>
    /// Check if a panel should be visible
    /// </summary>
    public bool IsPanelActive(int panelIndex)
    {
        if (panelIndex < 0 || panelIndex >= _totalPanels)
            return false;

        float startTime = _panelStartTimes[panelIndex];
        float endTime = startTime + panelDuration;

        return elapsedTime >= startTime && elapsedTime < endTime;
    }

    /// <summary>
    /// Determine slide direction based on panel number
    /// Odd panels (1,3,5...) = Left, Even panels (2,4,6...) = Right
    /// </summary>
    public bool IsPanelSlidingFromLeft(int panelIndex)
    {
        int panelNumber = panelIndex + 1; // Convert 0-based to 1-based
        return (panelNumber % 2) == 1; // Odd numbers slide from left
    }

    // =========================
    // Easing Functions
    // =========================
    private float EaseOutQuad(float t)
    {
        return 1.0f - (1.0f - t) * (1.0f - t);
    }

    private float EaseInQuad(float t)
    {
        return t * t;
    }

    private float EaseInOutQuad(float t)
    {
        if (t < 0.5f)
            return 2.0f * t * t;
        else
            return 1.0f - ((-2.0f * t + 2.0f) * (-2.0f * t + 2.0f)) / 2.0f;
    }

    // =========================
    // Helper Functions
    // =========================
    public float Lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    // =========================
    // Completion Handler
    // =========================
    private void OnCutsceneComplete()
    {
        Console.WriteLine("[CutsceneManager] All panels shown. Ready to transition.");
        NavigationButtons.LoadGameplayScene("Level1");
        Scene.LoadScene("Level1");
    }

    // =========================
    // Debug Info
    // =========================
    public string GetDebugInfo()
    {
        return $"Time: {elapsedTime:F1}s | Panel: {currentPanelIndex + 1}/14 | {_panelNames[currentPanelIndex]}";
    }
}


// =========================
// UNIVERSAL PANEL SLIDER SCRIPT
// Attach this to ALL panel entities (Panel1 through Panel14)
// =========================
public class PanelSlider : Script
{
    // =========================
    // Configuration (Set in Inspector or via Prefab)
    // =========================
    public int panelIndex = 0; // 0 for Panel1, 1 for Panel2, etc.

    // Target position (where panel should be when fully visible)
    public float targetX = 0.0f;
    public float targetY = 0.0f;

    // =========================
    // Internal State
    // =========================
    private float slideDistance = 1200.0f;
    private float startX;
    private float endX;
    private bool isInitialized = false;

    // =========================
    // Initialization
    // =========================
    private void Initialize()
    {
        if (CutsceneManager.Instance == null)
            return;

        slideDistance = CutsceneManager.Instance.slideDistance;

        // Get the target position from current transform if not set
        TransformComponent transform = GetTransform();
        if (targetX == 0.0f && targetY == 0.0f)
        {
            targetX = transform.X;
            targetY = transform.Y;
        }

        // Determine slide direction
        bool slidesFromLeft = CutsceneManager.Instance.IsPanelSlidingFromLeft(panelIndex);

        if (slidesFromLeft)
        {
            // Odd panels: Start off-screen left, slide in, then exit to left
            startX = targetX - slideDistance;
            endX = targetX - slideDistance;
        }
        else
        {
            // Even panels: Start off-screen right, slide in, then exit to right
            startX = targetX + slideDistance;
            endX = targetX + slideDistance;
        }

        // Set initial position (off-screen)
        transform.X = startX;
        transform.Y = targetY;
        transform.IsVisible = false; // Start invisible

        isInitialized = true;

        string direction = slidesFromLeft ? "LEFT" : "RIGHT";
        Console.WriteLine($"[PanelSlider] Panel {panelIndex + 1} initialized - slides from {direction}");
    }

    // =========================
    // Update Loop
    // =========================
    public override void Update()
    {
        if (CutsceneManager.Instance == null)
            return;

        if (!isInitialized)
        {
            Initialize();
            return;
        }

        TransformComponent transform = GetTransform();

        // Check if this panel is currently active
        bool isActive = CutsceneManager.Instance.IsPanelActive(panelIndex);
        transform.IsVisible = isActive;

        if (!isActive)
        {
            // If panel hasn't started yet, stay at start position
            // If panel is finished, stay at end position
            float progress = CutsceneManager.Instance.GetPanelProgress(panelIndex);
            if (progress == 0.0f)
            {
                transform.X = startX;
            }
            else if (progress >= 1.0f)
            {
                transform.X = endX;
            }
            return;
        }

        // Panel is active - animate it
        float slideInProgress = CutsceneManager.Instance.GetPanelSlideInProgress(panelIndex);
        float slideOutProgress = CutsceneManager.Instance.GetPanelSlideOutProgress(panelIndex);

        float currentX;

        if (slideOutProgress > 0.0f)
        {
            // Sliding out
            currentX = CutsceneManager.Instance.Lerp(targetX, endX, slideOutProgress);
        }
        else
        {
            // Sliding in
            currentX = CutsceneManager.Instance.Lerp(startX, targetX, slideInProgress);
        }

        transform.X = currentX;
        transform.Y = targetY;
    }
}


// =========================
// INDIVIDUAL PANEL SCRIPTS (For Easy Setup)
// These just set the panelIndex automatically
// =========================

public class Panel1Slider : PanelSlider
{
    public void OnCreate()
    {
        panelIndex = 0;
        targetX = -679.232f;
        targetY = -40.305f;
    }
}

public class Panel2Slider : PanelSlider
{
    public void OnCreate()
    {
        panelIndex = 1;
        targetX = 674.928f;
        targetY = -50.000f;
    }
}

public class Panel3Slider : PanelSlider
{
    public void OnCreate()
    {
        panelIndex = 2;
        targetX = -679.232f;
        targetY = -40.305f;
    }
}

public class Panel4Slider : PanelSlider
{
    public void OnCreate()
    {
        panelIndex = 3;
        targetX = 674.928f;
        targetY = -50.000f;
    }
}

public class Panel5Slider : PanelSlider
{
    public void OnCreate()
    {
        panelIndex = 4;
        targetX = -679.232f;
        targetY = -40.305f;
    }
}

public class Panel6Slider : PanelSlider
{
    public void OnCreate()
    {
        panelIndex = 5;
        targetX = 674.928f;
        targetY = -50.000f;
    }
}

public class Panel7Slider : PanelSlider
{
    public void OnCreate()
    {
        panelIndex = 6;
        targetX = -679.232f;
        targetY = -40.305f;
    }
}

public class Panel8Slider : PanelSlider
{
    public void OnCreate()
    {
        panelIndex = 7;
        targetX = 674.928f;
        targetY = -50.000f;
    }
}

public class Panel9Slider : PanelSlider
{
    public void OnCreate()
    {
        panelIndex = 8;
        targetX = -679.232f;
        targetY = -40.305f;
    }
}

public class Panel10Slider : PanelSlider
{
    public void OnCreate()
    {
        panelIndex = 9;
        targetX = 674.928f;
        targetY = -50.000f;
    }
}

public class Panel11Slider : PanelSlider
{
    public void OnCreate()
    {
        panelIndex = 10;
        targetX = -679.232f;
        targetY = -40.305f;
    }
}

public class Panel12Slider : PanelSlider
{
    public void OnCreate()
    {
        panelIndex = 11;
        targetX = 674.928f;
        targetY = -50.000f;
    }
}

public class Panel13Slider : PanelSlider
{
    public void OnCreate()
    {
        panelIndex = 12;
        targetX = -679.232f;
        targetY = -40.305f;
    }
}

public class Panel14Slider : PanelSlider
{
    public void OnCreate()
    {
        panelIndex = 13;
        targetX = 674.928f;
        targetY = -50.000f;
    }
}