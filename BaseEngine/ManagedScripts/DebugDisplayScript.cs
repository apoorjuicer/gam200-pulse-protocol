using System;
using ScriptAPI;

public class DebugDisplayScript : Script
{
    private float updateTimer = 0f;
    private const float UPDATE_INTERVAL = 0.1f;  // Update every 0.1 seconds
    private bool initialized = false;
    
    public override void Update()
    {
        // On first frame, hide in Release mode
        if (!initialized)
        {
            initialized = true;
            
#if !DEBUG
            // Hide in Release mode
            TransformComponent t = GetTransform();
            t.IsVisible = false;
            Console.WriteLine("[DebugDisplay] Release mode detected - hiding by default");
#endif
        }

        var text = GetTextComponent();
        
        float dt = Time.DeltaTime;
        updateTimer += dt;
        
        if (updateTimer >= UPDATE_INTERVAL)
        {
            // Get FPS using ProfilingAPI
            float fps = ProfilingAPI.GetFPS();
            
            // Get profiler data using ProfilingAPI
            var displayProfiles = ProfilingAPI.GetDisplayProfiles();
            
            // Find the system with highest percentage
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
            
            // Combine both on one line: "FPS: 95.8 | SystemName: 51.93%"
            string combinedText = $"FPS: {fps:F1} | {topSystem}: {topPercentage:F2}%";
            text.SetText(combinedText);
            
            updateTimer = 0f;
        }
    }
}