using System;
using ScriptAPI;

public class DebugDisplayScript : Script
{
    private float updateTimer = 0f;
    private const float UPDATE_INTERVAL = 0.1f;
    private bool hasSetInitialVisibility = false;
    
    // Static variables persist across scene changes
    private static bool s_hasBeenToggled = false;
    private static bool s_userVisibilitySetting = false;
    
    // Track if this is first instance ever (application start)
    private static bool s_isFirstInstanceEver = true;
    
    public override void Update()
    {
        // On very first instance (app start), reset toggle state
        if (s_isFirstInstanceEver)
        {
            s_isFirstInstanceEver = false;
            s_hasBeenToggled = false;
            s_userVisibilitySetting = false;
        }
        
        // We can't directly access TextComponent.visible from C#
        // The C++ F8 toggle handles visibility via the C++ TextComponent
        // We just need to update the text content
        
        // Set initial visibility only once per entity using Transform
        if (!hasSetInitialVisibility)
        {
            hasSetInitialVisibility = true;
            
            // Note: We can't control TextComponent.visible from C#
            // But we can track if user has toggled via other means
            // For now, just set initial state and let C++ F8 handle it
        }
        
        // Always update text content
        var text = GetTextComponent();
        
        float dt = Time.DeltaTime;
        updateTimer += dt;
        
        if (updateTimer >= UPDATE_INTERVAL)
        {
            float fps = ProfilingAPI.GetFPS();
            var displayProfiles = ProfilingAPI.GetDisplayProfiles();
            
            string topSystem = "None";
            double topPercentage = 0.0;
            
            foreach (var profile in displayProfiles)
            {
                if (profile.Value.percentage > topPercentage)
                {
                    topPercentage = profile.Value.percentage;
                    topSystem = profile.Key;
                }
            }
            
            string combinedText = $"FPS: {fps:F1} | {topSystem}: {topPercentage:F2}%";
            text.SetText(combinedText);
            
            updateTimer = 0f;
        }
    }
}