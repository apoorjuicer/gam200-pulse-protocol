using ScriptAPI;

// ======================================================
// Shared dropdown state (engine-style)
// ======================================================
public static class DropdownState
{
    public static bool ScreenDropdownOpen = false;

    // Bounds of the OPEN dropdown (world space)
    public static float OpenX;
    public static float OpenY;
    public static float OpenHalfW;
    public static float OpenHalfH;

    // Bounds of the CLOSED header box (world space)
    public static float HeaderX;
    public static float HeaderY;
    public static float HeaderHalfW;
    public static float HeaderHalfH;

    // Debounce: block option selection for a couple frames after opening
    public static int SuppressOptionClickFrames = 0;

    public static bool IsInsideOpenDropdown(float x, float y)
    {
        return x >= OpenX - OpenHalfW && x <= OpenX + OpenHalfW &&
               y >= OpenY - OpenHalfH && y <= OpenY + OpenHalfH;
    }

    public static bool IsInsideHeader(float x, float y)
    {
        return x >= HeaderX - HeaderHalfW && x <= HeaderX + HeaderHalfW &&
               y >= HeaderY - HeaderHalfH && y <= HeaderY + HeaderHalfH;
    }
}

// ======================================================
// Controls click + open/close state
// Attach to: DropdownBox (Closed)
// ======================================================
public class DropdownToggle : Script
{
    private bool initialized = false;
    private float baseScaleX;
    private float baseScaleY;

    public override void Update()
    {
        InputComponent input = GetInput();
        TransformComponent t = GetTransform();

        if (!initialized)
        {
            baseScaleX = t.ScaleX;
            baseScaleY = t.ScaleY;
            initialized = true;
        }

        // Publish header bounds every frame
        DropdownState.HeaderX = t.X;
        DropdownState.HeaderY = t.Y;
        DropdownState.HeaderHalfW = baseScaleX;
        DropdownState.HeaderHalfH = baseScaleY;

        float mx = input.WorldMouseX;
        float my = input.WorldMouseY;

        bool isHovered =
            mx >= t.X - baseScaleX && mx <= t.X + baseScaleX &&
            my >= t.Y - baseScaleY && my <= t.Y + baseScaleY;

        // Click header toggles dropdown open/close
        if (isHovered && input.IsMouseButtonTriggered(0))
        {
            bool wasOpen = DropdownState.ScreenDropdownOpen;
            DropdownState.ScreenDropdownOpen = !wasOpen;

            // If we JUST opened, suppress option click for 2 frames
            // (2 makes it robust even if script update order is weird)
            if (!wasOpen && DropdownState.ScreenDropdownOpen)
                DropdownState.SuppressOptionClickFrames = 2;

            return;
        }

        // Click outside closes dropdown (but NOT if click is inside open dropdown)
        if (DropdownState.ScreenDropdownOpen &&
            input.IsMouseButtonTriggered(0) &&
            !DropdownState.IsInsideOpenDropdown(mx, my) &&
            !DropdownState.IsInsideHeader(mx, my))
        {
            DropdownState.ScreenDropdownOpen = false;
        }
    }
}

// ======================================================
// Shows / hides the open dropdown graphic
// Attach to: DropdownBox (Open)
// ======================================================
public class DropdownOpenFollower : Script
{
    private bool initialized = false;
    private float originalScaleX;
    private float originalScaleY;

    public override void Update()
    {
        TransformComponent t = GetTransform();

        if (!initialized)
        {
            originalScaleX = t.ScaleX;
            originalScaleY = t.ScaleY;
            initialized = true;
        }

        // Decrement debounce here (runs every frame)
        if (DropdownState.SuppressOptionClickFrames > 0)
            DropdownState.SuppressOptionClickFrames--;

        // Publish open bounds using original size (even if hidden)
        DropdownState.OpenX = t.X;
        DropdownState.OpenY = t.Y;
        DropdownState.OpenHalfW = originalScaleX;
        DropdownState.OpenHalfH = originalScaleY;

        if (DropdownState.ScreenDropdownOpen)
        {
            t.ScaleX = originalScaleX;
            t.ScaleY = originalScaleY;
        }
        else
        {
            t.ScaleX = 0.0f;
            t.ScaleY = 0.0f;
        }
    }
}

// ======================================================
// OPTION: Fullscreen
// Attach to: the "FULL SCREEN" row entity (text OR sprite)
// ======================================================
public class DropdownOptionFullscreen : Script
{
    // Make the clickable region feel like a real dropdown row:
    // - width comes from the open dropdown box
    // - height comes from this row's size (scaled up)
    public float RowHitboxScaleY = 2.8f; // bigger = easier to click
    public float RowPadY = 0.0f;

    private bool initialized = false;
    private float originalScaleX;
    private float originalScaleY;

    public override void Update()
    {
        // Show this option only when currently WINDOWED
        if (ToggleScreenAPI.IsFullscreen() || !DropdownState.ScreenDropdownOpen)
        {
            HideSelf();
            return;
        }

        ShowSelf();

        // Prevent the "open click" from instantly selecting
        if (DropdownState.SuppressOptionClickFrames > 0)
            return;

        InputComponent input = GetInput();
        TransformComponent t = GetTransform();

        if (!initialized)
        {
            originalScaleX = t.ScaleX;
            originalScaleY = t.ScaleY;
            initialized = true;
        }

        float mx = input.WorldMouseX;
        float my = input.WorldMouseY;

        // Big row hitbox (not just the letters)
        float cx = DropdownState.OpenX;       // center aligned with dropdown box
        float cy = t.Y;                       // row position (you placed it visually)
        float halfW = DropdownState.OpenHalfW;
        float halfH = (originalScaleY * RowHitboxScaleY) + RowPadY;

        bool hovered =
            mx >= cx - halfW && mx <= cx + halfW &&
            my >= cy - halfH && my <= cy + halfH;

        if (hovered && input.IsMouseButtonTriggered(0))
        {
            ToggleScreenAPI.ToggleFullscreen();
            DropdownState.ScreenDropdownOpen = false;
        }
    }

    private void HideSelf()
    {
        TransformComponent t = GetTransform();
        if (!initialized)
        {
            originalScaleX = t.ScaleX;
            originalScaleY = t.ScaleY;
            initialized = true;
        }
        t.ScaleX = 0.0f;
        t.ScaleY = 0.0f;
    }

    private void ShowSelf()
    {
        TransformComponent t = GetTransform();
        if (!initialized)
        {
            originalScaleX = t.ScaleX;
            originalScaleY = t.ScaleY;
            initialized = true;
        }
        t.ScaleX = originalScaleX;
        t.ScaleY = originalScaleY;
    }
}

// ======================================================
// OPTION: Windowed
// Attach to: the "WINDOWED" row entity (for fullscreen state)
// ======================================================
public class DropdownOptionWindowed : Script
{
    public float RowHitboxScaleY = 2.8f;
    public float RowPadY = 0.0f;

    private bool initialized = false;
    private float originalScaleX;
    private float originalScaleY;

    public override void Update()
    {
        // Show this option only when currently FULLSCREEN
        if (!ToggleScreenAPI.IsFullscreen() || !DropdownState.ScreenDropdownOpen)
        {
            HideSelf();
            return;
        }

        ShowSelf();

        if (DropdownState.SuppressOptionClickFrames > 0)
            return;

        InputComponent input = GetInput();
        TransformComponent t = GetTransform();

        if (!initialized)
        {
            originalScaleX = t.ScaleX;
            originalScaleY = t.ScaleY;
            initialized = true;
        }

        float mx = input.WorldMouseX;
        float my = input.WorldMouseY;

        float cx = DropdownState.OpenX;
        float cy = t.Y;
        float halfW = DropdownState.OpenHalfW;
        float halfH = (originalScaleY * RowHitboxScaleY) + RowPadY;

        bool hovered =
            mx >= cx - halfW && mx <= cx + halfW &&
            my >= cy - halfH && my <= cy + halfH;

        if (hovered && input.IsMouseButtonTriggered(0))
        {
            ToggleScreenAPI.ToggleFullscreen();
            DropdownState.ScreenDropdownOpen = false;
        }
    }

    private void HideSelf()
    {
        TransformComponent t = GetTransform();
        if (!initialized)
        {
            originalScaleX = t.ScaleX;
            originalScaleY = t.ScaleY;
            initialized = true;
        }
        t.ScaleX = 0.0f;
        t.ScaleY = 0.0f;
    }

    private void ShowSelf()
    {
        TransformComponent t = GetTransform();
        if (!initialized)
        {
            originalScaleX = t.ScaleX;
            originalScaleY = t.ScaleY;
            initialized = true;
        }
        t.ScaleX = originalScaleX;
        t.ScaleY = originalScaleY;
    }

// ======================================================
// CLOSED LABEL: WINDOWED
// Attach to: ScreenLabel_Windowed
// Shows only when NOT fullscreen
// ======================================================
public class DropdownClosedLabelWindowed : Script
{
    private bool initialized = false;
    private float originalScaleX;
    private float originalScaleY;

    public override void Update()
    {
        TransformComponent t = GetTransform();

        // Cache original visible scale (make sure label is NOT saved at 0 in editor)
        if (!initialized)
        {
            originalScaleX = t.ScaleX;
            originalScaleY = t.ScaleY;
            initialized = true;
        }

        bool isFs = ToggleScreenAPI.IsFullscreen();

        if (!isFs)
        {
            // show
            t.ScaleX = originalScaleX;
            t.ScaleY = originalScaleY;
        }
        else
        {
            // hide
            t.ScaleX = 0.0f;
            t.ScaleY = 0.0f;
        }
    }
}

// ======================================================
// CLOSED LABEL: FULL SCREEN
// Attach to: ScreenLabel_Fullscreen
// Shows only when fullscreen
// ======================================================
public class DropdownClosedLabelFullscreen : Script
{
    private bool initialized = false;
    private float originalScaleX;
    private float originalScaleY;

    public override void Update()
    {
        TransformComponent t = GetTransform();

        // Cache original visible scale (make sure label is NOT saved at 0 in editor)
        if (!initialized)
        {
            originalScaleX = t.ScaleX;
            originalScaleY = t.ScaleY;
            initialized = true;
        }

        bool isFs = ToggleScreenAPI.IsFullscreen();

        if (isFs)
        {
            // show
            t.ScaleX = originalScaleX;
            t.ScaleY = originalScaleY;
        }
        else
        {
            // hide
            t.ScaleX = 0.0f;
            t.ScaleY = 0.0f;
        }
    }
}



}
