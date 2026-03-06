/******************************************************************************/
/**
* @file        GlowOutlineScript.cs
* @project     Pulse Protocol
* @author      Goh Pin Kai
* @brief       make key outline glow
*
* @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
/******************************************************************************/
using System;
using ScriptAPI;

public class GlowOutlineScript : Script
{
    private int SlotIndex = 0; // will be auto-detected from sprite

    bool applied = false;

    public override void Start()
    {
        // optional: try once here, but don't rely on it
        TryApplyGlow();
    }

    public override void Update()
    {
        if (applied) return;

        // Keep trying for a few frames until engine systems are ready
        if (TryApplyGlow())
            applied = true;
    }

    private bool TryApplyGlow()
    {
        var glow = GetGlow();
        glow.SetColor(1.0f, 0.9f, 0.0f);
        glow.SetEnabled(false);
        glow.SetIntensity(0f);

        var sprite = GetSprite();
        if (!sprite.HasSprite()) return false;

        string fileName = System.IO.Path.GetFileName(sprite.Texture.ToLower());

        if      (fileName == "player input border 0.png") SlotIndex = 0;
        else if (fileName == "player input border 1.png") SlotIndex = 1;
        else if (fileName == "player input border 2.png") SlotIndex = 2;
        else if (fileName == "player input border 3.png") SlotIndex = 3;
        else return false; // unknown sprite

        ComboDisplayManager.RegisterOutlineSlot(SlotIndex, this);
        return true;
    }

    public void TriggerGlow()
    {
        var glow = GetGlow();
        glow.SetEnabled(true);
        glow.SetIntensity(0f);
        glow.GlowTo(6.0f, 0.15f); // fade in
    }

    public void ResetGlow()
    {
        var glow = GetGlow();
        glow.GlowTo(0f, 0.3f); // fade out
    }
}
