using ScriptAPI;

// ======================================================
// Shared dropdown state (engine-style)
// ======================================================
public static class DropdownState
{
    // You can add more later using keys
    public static bool ScreenDropdownOpen = false;
}

// ======================================================
// Controls click + open/close state
// Attach to: DropdownBox (Closed)
// ======================================================
public class DropdownToggle : Script
{
    public float HoverScaleBoost = 1.0f; // keep 1.0 if no hover animation

    private bool initialized = false;
    private float baseScaleX;
    private float baseScaleY;

    public override void Update()
    {
        InputComponent input = GetInput();
        TransformComponent t = GetTransform();

        // Cache base scale once (same as buttons)
        if (!initialized)
        {
            baseScaleX = t.ScaleX;
            baseScaleY = t.ScaleY;
            initialized = true;
        }

        float mx = input.WorldMouseX;
        float my = input.WorldMouseY;

        float halfW = baseScaleX;
        float halfH = baseScaleY;

        bool isHovered =
            mx >= t.X - halfW && mx <= t.X + halfW &&
            my >= t.Y - halfH && my <= t.Y + halfH;

        // Click to toggle dropdown
        if (isHovered && input.IsMouseButtonTriggered(0))
        {
            DropdownState.ScreenDropdownOpen =
                !DropdownState.ScreenDropdownOpen;
            return;
        }

        // Click outside closes dropdown
        if (input.IsMouseButtonTriggered(0) && !isHovered)
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

        // Cache original scale once
        if (!initialized)
        {
            originalScaleX = t.ScaleX;
            originalScaleY = t.ScaleY;
            initialized = true;
        }

        if (DropdownState.ScreenDropdownOpen)
        {
            // Show
            t.ScaleX = originalScaleX;
            t.ScaleY = originalScaleY;
        }
        else
        {
            // Hide
            t.ScaleX = 0.0f;
            t.ScaleY = 0.0f;
        }
    }
}
