using ScriptAPI;
using System;

public class LevelSelectOverlay : Script
{
    private const int GP_CIRCLE = 1;

    public override void Update()
    {
        TransformComponent t = GetTransform();
        bool visible = NavigationButtons.ShowLevelSelectOverlay &&
                       !NavigationButtons.ShowSettingsOverlay &&
                       !NavigationButtons.ShowQuitOverlay &&
                       !NavigationButtons.ShowHowToPlayOverlay;

        t.IsVisible = visible;
        if (!visible)
            return;

        InputComponent input = GetInput();
        if (input.IsKeyTriggered(256) || input.IsGamepadButtonTriggered(GP_CIRCLE))
        {
            NavigationButtons.ShowLevelSelectOverlay = false;
        }
    }
}

public class LevelSelectCard : Script
{
    private static readonly string[] s_sceneOrder = new[] { "Level1", "Level2", "Level3" };
    private static int s_selectedIndex = 0;
    private static bool s_initializedSelection = false;
    private static bool s_clickedThisFrame = false;
    private static int s_clickFrame = -1;
    private static bool s_centerSettled = false;
    private static int s_settleFrame = -1;

    private bool initialized = false;
    private int myIndex = 0;
    private string levelScene = "Level1";

    public override void Update()
    {
        if (s_clickFrame != Time.FrameCount)
        {
            s_clickFrame = Time.FrameCount;
            s_clickedThisFrame = false;
        }
        if (s_settleFrame != Time.FrameCount)
        {
            s_settleFrame = Time.FrameCount;
            s_centerSettled = false;
        }

        TransformComponent t = GetTransform();
        bool visible = NavigationButtons.ShowLevelSelectOverlay &&
                       !NavigationButtons.ShowSettingsOverlay &&
                       !NavigationButtons.ShowQuitOverlay &&
                       !NavigationButtons.ShowHowToPlayOverlay;

        t.IsVisible = visible;
        if (!visible)
        {
            // Reinitialize selection whenever overlay is opened again.
            s_initializedSelection = false;
            return;
        }

        if (!initialized)
        {
            initialized = true;

            string tex = (GetSprite().Texture ?? "").ToLower();
            if (tex.Contains("level2"))
                levelScene = "Level2";
            else if (tex.Contains("level3"))
                levelScene = "Level3";
            else
                levelScene = "Level1";

            myIndex = GetSceneIndex(levelScene);
        }

        if (!s_initializedSelection)
        {
            s_initializedSelection = true;
            s_selectedIndex = GetSceneIndex(NavigationButtons.SelectedLevelScene);
            if (!IsSceneUnlocked(s_sceneOrder[s_selectedIndex]))
                s_selectedIndex = 0;
            NavigationButtons.SelectedLevelScene = s_sceneOrder[s_selectedIndex];
        }

        InputComponent input = GetInput();

        float halfW = t.ScaleX * 0.5f;
        float halfH = t.ScaleY * 0.5f;
        bool isHoveredNow = input.InGameViewport &&
                         input.WorldMouseX >= t.X - halfW && input.WorldMouseX <= t.X + halfW &&
                         input.WorldMouseY >= t.Y - halfH && input.WorldMouseY <= t.Y + halfH;

        if (!s_clickedThisFrame && isHoveredNow && input.IsMouseButtonTriggered(0))
        {
            s_clickedThisFrame = true;
            if (IsSceneUnlocked(levelScene))
            {
                s_selectedIndex = myIndex;
                NavigationButtons.SelectedLevelScene = levelScene;
            }
        }

        SpriteComponent sprite = GetSprite();
        if (IsSceneUnlocked(levelScene))
            sprite.SetTint(1.0f, 1.0f, 1.0f, 1.0f);
        else
            sprite.SetTint(0.42f, 0.42f, 0.42f, 1.0f);

        int offset = myIndex - s_selectedIndex;
        if (offset > 1) offset = 1;
        if (offset < -1) offset = -1;

        float targetX = 0.0f;
        float targetY = 160.0f;
        float targetScaleX = 420.0f;
        float targetScaleY = 470.0f;

        if (offset < 0)
        {
            targetX = -920.0f;
            targetY = 140.0f;
            targetScaleX = 360.0f;
            targetScaleY = 400.0f;
        }
        else if (offset > 0)
        {
            targetX = 920.0f;
            targetY = 140.0f;
            targetScaleX = 360.0f;
            targetScaleY = 400.0f;
        }

        float lerp = Math.Min(1.0f, Time.DeltaTime * 10.0f);
        t.X += (targetX - t.X) * lerp;
        t.Y += (targetY - t.Y) * lerp;
        t.ScaleX += (targetScaleX - t.ScaleX) * lerp;
        t.ScaleY += (targetScaleY - t.ScaleY) * lerp;

        if (offset == 0)
        {
            bool atCenter =
                Math.Abs(t.X - targetX) < 3.0f &&
                Math.Abs(t.Y - targetY) < 3.0f &&
                Math.Abs(t.ScaleX - targetScaleX) < 3.0f &&
                Math.Abs(t.ScaleY - targetScaleY) < 3.0f;

            if (atCenter)
                s_centerSettled = true;
        }
    }

    public static float SelectedX => 0.0f;
    public static float SelectedY => 160.0f;
    public static float SelectedScaleX => 420.0f;
    public static float SelectedScaleY => 470.0f;
    public static bool IsCenterSettled => s_centerSettled;

    private static int GetSceneIndex(string scene)
    {
        for (int i = 0; i < s_sceneOrder.Length; ++i)
        {
            if (string.Equals(s_sceneOrder[i], scene, StringComparison.OrdinalIgnoreCase))
                return i;
        }
        return 0;
    }

    public static bool IsSceneUnlocked(string sceneName)
    {
        int targetLevel = ExtractLevelNumber(sceneName);
        if (targetLevel <= 1)
            return true; // Level1 always unlocked

        int lastClearedLevel = ExtractLevelNumber(NavigationButtons.LastClearedLevel);
        return lastClearedLevel >= (targetLevel - 1);
    }

    private static int ExtractLevelNumber(string sceneName)
    {
        if (string.IsNullOrEmpty(sceneName))
            return 0;

        int i = sceneName.Length - 1;
        while (i >= 0 && char.IsDigit(sceneName[i]))
            i--;

        if (i == sceneName.Length - 1)
            return 0;

        string numberPart = sceneName.Substring(i + 1);
        if (int.TryParse(numberPart, out int parsed))
            return parsed;

        return 0;
    }
}

public class LevelSelectSelector : Script
{
    public override void Update()
    {
        TransformComponent t = GetTransform();
        bool visible = NavigationButtons.ShowLevelSelectOverlay &&
                       !NavigationButtons.ShowSettingsOverlay &&
                       !NavigationButtons.ShowQuitOverlay &&
                       !NavigationButtons.ShowHowToPlayOverlay &&
                       LevelSelectCard.IsCenterSettled;

        t.IsVisible = visible;
        if (!visible)
            return;

        float lerp = Math.Min(1.0f, Time.DeltaTime * 12.0f);
        float targetX = LevelSelectCard.SelectedX;
        float targetY = LevelSelectCard.SelectedY;
        float targetScaleX = LevelSelectCard.SelectedScaleX * 1.20f;
        float targetScaleY = LevelSelectCard.SelectedScaleY * 1.20f;

        t.X += (targetX - t.X) * lerp;
        t.Y += (targetY - t.Y) * lerp;
        t.ScaleX += (targetScaleX - t.ScaleX) * lerp;
        t.ScaleY += (targetScaleY - t.ScaleY) * lerp;
    }
}

public class LevelSelectStartDisc : Script
{
    private bool initialized = false;
    private bool wasStartHovered = false;
    private float baseScaleX;
    private float baseScaleY;
    private const float SpinSpeed = 45.0f;
    private const float StartHitboxScale = 0.22f; // center-focused area around the START text
    private const float StartHoverScaleBoost = 1.03f;
    private const float DiscHoverRadiusScale = 2.25f; // very generous full-disc hover coverage

    public override void Update()
    {
        TransformComponent t = GetTransform();
        bool visible = NavigationButtons.ShowLevelSelectOverlay &&
                       !NavigationButtons.ShowSettingsOverlay &&
                       !NavigationButtons.ShowQuitOverlay &&
                       !NavigationButtons.ShowHowToPlayOverlay;

        t.IsVisible = visible;
        if (!visible)
            return;

        if (!initialized)
        {
            initialized = true;
            baseScaleX = t.ScaleX;
            baseScaleY = t.ScaleY;
        }

        InputComponent input = GetInput();
        float mx = input.WorldMouseX;
        float my = input.WorldMouseY;

        // Any hover on the disc surface should stop spinning.
        float discNx = (mx - t.X) / (t.ScaleX * 0.5f * DiscHoverRadiusScale);
        float discNy = (my - t.Y) / (t.ScaleY * 0.5f * DiscHoverRadiusScale);
        bool inDiscEllipse = (discNx * discNx + discNy * discNy <= 1.0f);

        float discHalfW = t.ScaleX * 0.95f;
        float discHalfH = t.ScaleY * 0.95f;
        bool inDiscBox =
            mx >= t.X - discHalfW && mx <= t.X + discHalfW &&
            my >= t.Y - discHalfH && my <= t.Y + discHalfH;

        bool isDiscHovered = input.InGameViewport && (inDiscEllipse || inDiscBox);

        // Only the center START area triggers highlight/click-to-start.
        float startNx = (mx - t.X) / (baseScaleX * StartHitboxScale);
        float startNy = (my - t.Y) / (baseScaleY * StartHitboxScale);
        bool isStartHovered = input.InGameViewport && (startNx * startNx + startNy * startNy <= 1.0f);

        if (isStartHovered && !wasStartHovered)
        {
            AudioComponent audio = GetAudio();
            audio.PlaySFX("SFX_Button_Hover");
        }
        wasStartHovered = isStartHovered;

        if (!isDiscHovered)
            t.RotX += SpinSpeed * Time.DeltaTime;

        float targetScaleX = isStartHovered ? baseScaleX * StartHoverScaleBoost : baseScaleX;
        float targetScaleY = isStartHovered ? baseScaleY * StartHoverScaleBoost : baseScaleY;
        float scaleLerp = Math.Min(1.0f, Time.DeltaTime * 12.0f);
        t.ScaleX += (targetScaleX - t.ScaleX) * scaleLerp;
        t.ScaleY += (targetScaleY - t.ScaleY) * scaleLerp;

        if (isStartHovered && input.IsMouseButtonTriggered(0))
        {
            AudioComponent audio = GetAudio();
            audio.PlaySFX("SFX_Button_Select");

            string sceneToLoad = NavigationButtons.SelectedLevelScene;
            if (!LevelSelectCard.IsSceneUnlocked(sceneToLoad))
                return;

            if (string.IsNullOrEmpty(sceneToLoad) || !Scene.SceneExists(sceneToLoad))
                sceneToLoad = "Level1";

            NavigationButtons.ShowLevelSelectOverlay = false;
            NavigationButtons.currentScene = sceneToLoad;
            NavigationButtons.LastEnteredLevel = sceneToLoad;
            LoadingScreenScript.NextLevel = sceneToLoad;
            Scene.LoadScene("LoadingScreenScene");
        }
    }
}

public class LevelSelectBackButton : Script
{
    private bool initialized = false;
    private string defaultTexture = "";
    private string hoverTexture = "";
    private bool isHovered = false;

    public override void Update()
    {
        TransformComponent t = GetTransform();
        bool visible = NavigationButtons.ShowLevelSelectOverlay &&
                       !NavigationButtons.ShowSettingsOverlay &&
                       !NavigationButtons.ShowQuitOverlay &&
                       !NavigationButtons.ShowHowToPlayOverlay;
        t.IsVisible = visible;

        if (!visible)
            return;

        SpriteComponent sprite = GetSprite();
        if (!initialized)
        {
            initialized = true;
            defaultTexture = sprite.Texture ?? "Assets/Textures/UI/back.png";

            int dot = defaultTexture.LastIndexOf('.');
            if (dot >= 0)
                hoverTexture = defaultTexture.Substring(0, dot) + "_hover" + defaultTexture.Substring(dot);
            else
                hoverTexture = defaultTexture + "_hover";
        }

        InputComponent input = GetInput();
        float halfW = t.ScaleX * 0.5f;
        float halfH = t.ScaleY * 0.5f;
        bool hoveredNow = input.InGameViewport &&
                          input.WorldMouseX >= t.X - halfW && input.WorldMouseX <= t.X + halfW &&
                          input.WorldMouseY >= t.Y - halfH && input.WorldMouseY <= t.Y + halfH;

        if (hoveredNow && !isHovered)
        {
            sprite.Texture = hoverTexture;
            isHovered = true;
            PlayHoverSound();
        }
        else if (!hoveredNow && isHovered)
        {
            sprite.Texture = defaultTexture;
            isHovered = false;
        }

        if (hoveredNow && input.IsMouseButtonTriggered(0))
        {
            PlayClickSound();
            NavigationButtons.ShowLevelSelectOverlay = false;
            sprite.Texture = defaultTexture;
            isHovered = false;
        }
    }

    private void PlayHoverSound()
    {
        AudioComponent audio = GetAudio();
        audio.PlaySFX("SFX_Button_Hover");
    }

    private void PlayClickSound()
    {
        AudioComponent audio = GetAudio();
        audio.PlaySFX("SFX_Button_Select");
    }
}
