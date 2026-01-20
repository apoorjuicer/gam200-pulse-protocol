using ScriptAPI;

public class TestAudioScript : Script
{
    private bool played = false;

    public override void Update()
    {
        if (played) return;

        // plays whatever AudioSource is set on this entity in Inspector
        GetAudio().Play();
        played = true;
    }
}
