using System;
using ScriptAPI;
// Call loading screen scene and the png will have this script where it just loads the png then loads level 1 after like 3 frames
// Then it will freeze on this scene instead of wtv scene it was at first.

public class LoadingScreenScript : Script
{
    public int CurrentFrame = 0;
    public int LoadingFrame = 1; // how many frame then load nxt level, i just put 3 cos idk tbh XD

    // for the next level to be loaded, it is global so before u load the scene, just make delcare this variable.
    public static string NextLevel = ""; 

    public override void Update()
    {
        ++CurrentFrame;
        
        // >= just incase it skips ig :P
        if(CurrentFrame >= LoadingFrame)
        {
            // scene is global :3
            Scene.LoadScene(NextLevel);
        }

    }
}

