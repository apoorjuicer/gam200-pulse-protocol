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
    public float Width = 420f;
    public float Height = 64f;

    public float HitboxPadX = 10f;
    public float HitboxPadY = 6f;

    public override void Update()
    {
        InputComponent input = GetInput();
        TransformComponent t = GetTransform();

        float mx = input.WorldMouseX;
        float my = input.WorldMouseY;

        float cx = t.X;
        float cy = t.Y;

        bool click = input.IsMouseButtonTriggered(0);

        // Click on closed box → toggle
        if (click && PointInRect(mx, my, cx, cy, Width, Height))
        {
            DropdownState.ScreenDropdownOpen =
                !DropdownState.ScreenDropdownOpen;
            return;
        }

        // Click elsewhere → close
        if (click)
        {
            DropdownState.ScreenDropdownOpen = false;
        }
    }

    private bool PointInRect(
        float px, float py,
        float cx, float cy,
        float w, float h)
    {
        float halfW = (w * 0.5f) + HitboxPadX;
        float halfH = (h * 0.5f) + HitboxPadY;

        return px >= cx - halfW && px <= cx + halfW &&
               py >= cy - halfH && py <= cy + halfH;
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
