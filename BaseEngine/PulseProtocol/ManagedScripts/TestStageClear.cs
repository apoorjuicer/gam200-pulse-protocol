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

        // O = 79 -> Load GameLose
        if (input.IsKeyTriggered(79))
        {
            fired = true;
            NavigationButtons.GoToGameLose();
            return;
        }



    }
}
