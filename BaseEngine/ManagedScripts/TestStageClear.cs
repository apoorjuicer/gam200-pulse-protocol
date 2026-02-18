using ScriptAPI;

public class TestStageClear : Script
{
    private bool fired = false;

    public override void Update()
    {
        if (fired) return;

        var input = GetInput();

        // P = 80 -> Stage Clear
        if (input.IsKeyTriggered(80))
        {
            fired = true;
            StageClearElement.StageClear();
            return;
        }

        // O = 79 -> Load GameLose.json
        if (input.IsKeyTriggered(79))
        {
            fired = true;

            // you are currently in Level1 when this script runs
            NavigationButtons.SetLastGameplayScene("Level1");

            Scene.LoadScene("GameLose");
            return;
        }

    }
}
