using System;
using ScriptAPI;

/*
 * APIs Used:
 * - GetTextComponent()      → ScriptAPIClass.h (TextAPI)
 * - text.SetText()          → TextAPI.h
 * - text.SetColor()         → TextAPI.h
 * - Time.DeltaTime          → DeltaTime.h
 * - Time.GameTime           → DeltaTime.h
 */

public class TextSample : Script
{
    private float timer = 0f;
    private int currentPhase = 0;
    private float lastGameTime = 999999f;
    
    public override void Update()
    {
        var text = GetTextComponent();
        
        float gameTime = Time.GameTime;
        
        // Detect new Play session - GameTime resets to near 0
        if (gameTime < 0.1f && lastGameTime > 1.0f)
        {
            timer = 0f;
            currentPhase = 0;
            Console.WriteLine("[TextSample] NEW SESSION - RESET!");
        }
        lastGameTime = gameTime;
        
        float dt = Time.DeltaTime;
        timer += dt;
        
        switch (currentPhase)
        {
            case 0:
                text.SetText("3");               // TextAPI
                text.SetColor(1.0f, 0.0f, 0.0f); // TextAPI - Red
                if (timer >= 1.0f)
                {
                    timer = 0f;
                    currentPhase = 1;
                }
                break;
                
            case 1:
                text.SetText("2");               // TextAPI
                text.SetColor(1.0f, 0.5f, 0.0f); // TextAPI - Orange
                if (timer >= 1.0f)
                {
                    timer = 0f;
                    currentPhase = 2;
                }
                break;
                
            case 2:
                text.SetText("1");               // TextAPI
                text.SetColor(1.0f, 1.0f, 0.0f); // TextAPI - Yellow
                if (timer >= 1.0f)
                {
                    timer = 0f;
                    currentPhase = 3;
                }
                break;
                
            case 3:
                text.SetText("GO!");             // TextAPI
                text.SetColor(0.0f, 1.0f, 0.0f); // TextAPI - Green
                if (timer >= 1.0f)
                {
                    text.SetText("");            // Just clear text instead of SetVisible
                    currentPhase = 4;
                }
                break;
                
            case 4:
                break;
        }
    }
}