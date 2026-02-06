using System;
using ScriptAPI;

public class DebugDisplayScript : Script
{
    private float updateTimer = 0f;
    private const float UPDATE_INTERVAL = 0.1f;
    private static int s_instanceCount = 0;
    private int myInstanceNumber;
    private bool firstFrame = true;
    
    public DebugDisplayScript()
    {
        s_instanceCount++;
        myInstanceNumber = s_instanceCount;
    }
    
    public override void Update()
    {
        var textComp = GetTextComponent();
        
        // On first frame, show build mode in the text itself
        if (firstFrame)
        {
            firstFrame = false;
            
            string buildInfo = "";
#if DEBUG
            buildInfo = "DEBUG BUILD";
#else
            buildInfo = "RELEASE BUILD";
#endif
            
            textComp.SetText($"Instance #{myInstanceNumber} | {buildInfo}");
            return;  // Show this for one frame
        }
        
        float dt = Time.DeltaTime;
        updateTimer += dt;
        
        if (updateTimer >= UPDATE_INTERVAL || updateTimer == dt)
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
            
            // Show build mode in the text
            string buildMode = "";
#if DEBUG
            buildMode = " [DEBUG]";
#else
            buildMode = " [RELEASE]";
#endif
            
            string combinedText = $"FPS: {fps:F1} | {topSystem}: {topPercentage:F2}%{buildMode}";
            textComp.SetText(combinedText);
            
            updateTimer = 0f;
        }
    }
}