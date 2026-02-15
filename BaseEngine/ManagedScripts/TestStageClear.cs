using ScriptAPI;

public class TestStageClear : Script
{
    private bool fired = false;

    public override void Update()
    {
        if (fired) return;

        // P = 80
        if (GetInput().IsKeyTriggered(80))
        {
            fired = true;
            StageClearElement.StageClear();
        }
    }
}
