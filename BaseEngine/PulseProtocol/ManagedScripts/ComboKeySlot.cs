/******************************************************************************/
/**
* @file        ComboKeySlot.cs
* @project     Pulse Protocol
* @author      Carrie Lam (70%)
*              Chloe Lau (30%)
* @brief       Represents a single key slot in the combo UI, handling sprite
*              detection, registration, and animations for enemy reveals and
*              player input feedback (correct, wrong, countdown, pop effects).
*
* @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/

using System;
using ScriptAPI;

public class ComboKeySlot : Script
{
    // =========================
    // Configuration
    // =========================
    public int SlotIndex = 0;

    // Enemy row sprites
    public string WSprite = "../../PulseProtocol/Assets/Textures/UI/W.png";
    public string ASprite = "../../PulseProtocol/Assets/Textures/UI/A.png";
    public string SSprite = "../../PulseProtocol/Assets/Textures/UI/S.png";
    public string DSprite = "../../PulseProtocol/Assets/Textures/UI/D.png";

    // Player fill sprites
    public string BlankSprite = "../../PulseProtocol/Assets/Textures/UI/question mark blank key.png";
    public string GreenWSprite = "../../PulseProtocol/Assets/Textures/UI/Player Input GREEN W.png";
    public string GreenASprite = "../../PulseProtocol/Assets/Textures/UI/Player Input GREEN A.png";
    public string GreenSSprite = "../../PulseProtocol/Assets/Textures/UI/Player Input GREEN S.png";
    public string GreenDSprite = "../../PulseProtocol/Assets/Textures/UI/Player Input GREEN D.png";

    public string RedWSprite = "../../PulseProtocol/Assets/Textures/UI/Player Input RED W.png";
    public string RedASprite = "../../PulseProtocol/Assets/Textures/UI/Player Input RED A.png";
    public string RedSSprite = "../../PulseProtocol/Assets/Textures/UI/Player Input RED S.png";
    public string RedDSprite = "../../PulseProtocol/Assets/Textures/UI/Player Input RED D.png";

    public float DisplayDuration = 0.5f;
    public float ScaleAnimDuration = 0.15f;
    public float ResultDisplayDuration = 0.3f;
    public float CorrectHintDuration = 0.3f;

    // =========================
    // Internal State
    // =========================
    private float displayTimer = 0f;
    private bool isShowing = false;
    private bool hasRegistered = false;
    private bool IsEnemySlot = false;

    private bool isScaling = false;
    private float scaleTimer = 0f;
    private float baseScaleX = 1f;
    private float baseScaleY = 1f;
    private bool scaleCached = false;

    // Enemy pop animation
    private bool isPopping = false;
    private float popTimer = 0f;
    public float PopDuration = 0.15f;
    public float PopScale = 1.9f; // this is for enhancing the "pop" animation

    // PLAYER pop animation (when correct input)
    private bool isPoppingCorrect = false;
    private float correctPopTimer = 0f;
    public float CorrectPopDuration = 0.1f;
    public float CorrectPopScale = 1.2f;

    private bool isShowingResult = false;
    private float resultTimer = 0f;

    // Correct hint after wrong press
    private bool isShowingCorrectHint = false;
    private float correctHintTimer = 0f;
    private char correctKey = ' ';

    // Beat-synced scale pulse (triggered per-slot during player turn)
    public  float BeatScaleAmp        = 0.3f;
    private float _scalePulseTimer    = 0f;
    private float _scalePulseTotalDur = 0.5f;

    // Fade-in from dim (0.2) to full (1.0) when player turn starts
    private bool  _inFadeIn       = false;
    private float _fadeInTimer    = 0f;
    private float _fadeInDuration = 0.5f;

    // Fade-out from full (1.0) back to dim (0.2) when enemy turn resets
    private bool  _isFadingToDim    = false;
    private float _fadeToDimTimer   = 0f;
    private float _fadeToDimDuration = 0.5f;

    // =========================
    // Update
    // =========================
    public override void Update()
    {
        if (hasRegistered && !ComboDisplayManager.IsSlotRegistered(SlotIndex, this))
            hasRegistered = false;

        if (!hasRegistered)
        {
            // Cache scale FIRST
            if (!scaleCached)
            {
                var t = GetTransform();
                baseScaleX = t.ScaleX;
                baseScaleY = t.ScaleY;
                scaleCached = true;
            }

            var sprite = GetSprite();
            if (!sprite.HasSprite()) return;

            string texturePath = sprite.Texture;
            if (string.IsNullOrEmpty(texturePath)) return;

            string fileName = System.IO.Path.GetFileName(texturePath.ToLower());

            // Check green sprites FIRST
            if      (fileName == "player input green w.png") { IsEnemySlot = false; SlotIndex = 0; }
            else if (fileName == "player input green a.png") { IsEnemySlot = false; SlotIndex = 1; }
            else if (fileName == "player input green s.png") { IsEnemySlot = false; SlotIndex = 2; }
            else if (fileName == "player input green d.png") { IsEnemySlot = false; SlotIndex = 3; }

            // Then enemy row
            else if (fileName == "ekeyw.png") { IsEnemySlot = true;  SlotIndex = 0; }
            else if (fileName == "ekeya.png") { IsEnemySlot = true;  SlotIndex = 1; }
            else if (fileName == "ekeys.png") { IsEnemySlot = true;  SlotIndex = 2; }
            else if (fileName == "ekeyd.png") { IsEnemySlot = true;  SlotIndex = 3; }

            // Then single letter player row
            else if (fileName == "w.png") { IsEnemySlot = false; SlotIndex = 0; }
            else if (fileName == "a.png") { IsEnemySlot = false; SlotIndex = 1; }
            else if (fileName == "s.png") { IsEnemySlot = false; SlotIndex = 2; }
            else if (fileName == "d.png") { IsEnemySlot = false; SlotIndex = 3; }
            else
            {
                Console.WriteLine($"[ComboKeySlot] ⚠️ Could not detect slot from '{fileName}'");
                IsEnemySlot = false;
                SlotIndex = 0;
            }

            ComboDisplayManager.RegisterKeySlot(SlotIndex, this);
            HideInstant();
            hasRegistered = true;
            Console.WriteLine($"[ComboKeySlot] ✓ Slot {SlotIndex} registered as {(IsEnemySlot ? "Enemy" : "Player")} slot");
        }

        // =========================
        // Scale Animation
        // =========================
        if (isScaling && !IsEnemySlot)
        {
            scaleTimer += Time.DeltaTime;
            float t01 = Math.Min(scaleTimer / ScaleAnimDuration, 1f);
            float eased = 1f - (1f - t01) * (1f - t01);

            var transform = GetTransform();
            transform.ScaleX = baseScaleX * eased;
            transform.ScaleY = baseScaleY * eased;

            if (t01 >= 1f)
            {
                isScaling = false;
                // Console.WriteLine($"[ComboKeySlot] Slot {SlotIndex} scale animation complete");
            }
        }

        // =========================
        // Pop Animation (enemy row)
        // =========================
        if (isPopping)
        {
            popTimer += Time.DeltaTime;
            float t01 = Math.Min(popTimer / PopDuration, 1f);

            // Ease from PopScale back to 1
            float currentScale = PopScale + (1f - PopScale) * t01;

            var transform = GetTransform();
            transform.ScaleX = baseScaleX * currentScale;
            transform.ScaleY = baseScaleY * currentScale;

            if (t01 >= 1f)
                isPopping = false;
        }

        // =========================
        // Correct Pop Animation
        // =========================
        if (isPoppingCorrect)
        {
            correctPopTimer += Time.DeltaTime;
            float t01 = Math.Min(correctPopTimer / CorrectPopDuration, 1f);
            float currentScale = CorrectPopScale + (1f - CorrectPopScale) * t01;

            var transform = GetTransform();
            transform.ScaleX = baseScaleX * currentScale;
            transform.ScaleY = baseScaleY * currentScale;

            if (t01 >= 1f)
                isPoppingCorrect = false;
        }

        // =========================
        // Result Display Timer
        // =========================
        if (isShowingResult)
        {
            resultTimer -= Time.DeltaTime;
            if (resultTimer <= 0f)
            {
                isShowingResult = false;
                if (isShowingCorrectHint)
                    ShowCorrectHint(); // swap to green correct key
            }
        }

        // =========================
        // Correct Hint Timer
        // =========================
        if (isShowingCorrectHint && !isShowingResult)
        {
            correctHintTimer -= Time.DeltaTime;
            if (correctHintTimer <= 0f)
            {
                isShowingCorrectHint = false;
                HideInstant();
            }
        }


        // =========================
        // Fade-in (dim → full on player turn start)
        // =========================
        if (_inFadeIn)
        {
            _fadeInTimer += Time.DeltaTime;
            float t01 = Math.Min(_fadeInTimer / _fadeInDuration, 1f);
            float eased = 1f - (1f - t01) * (1f - t01);
            GetFade().SetAlpha(0.2f + 0.8f * eased);
            if (t01 >= 1f) _inFadeIn = false;
        }

        // =========================
        // Fade-out (full → dim on enemy turn reset)
        // =========================
        if (_isFadingToDim)
        {
            _fadeToDimTimer += Time.DeltaTime;
            float t01 = Math.Min(_fadeToDimTimer / _fadeToDimDuration, 1f);
            // ease-out: fast drop then settles slowly at dim
            float eased = 1f - (1f - t01) * (1f - t01);
            GetFade().SetAlpha(1.0f - 0.8f * eased);
            if (t01 >= 1f) _isFadingToDim = false;
        }

        // =========================
        // Beat scale pulse (per-slot, player turn)
        // =========================
        if (_scalePulseTimer > 0f && !isPopping && !isPoppingCorrect && !isScaling)
        {
            _scalePulseTimer -= Time.DeltaTime;
            if (_scalePulseTimer < 0f) _scalePulseTimer = 0f;
            float ft    = _scalePulseTimer / _scalePulseTotalDur; // always 0..1, no NaN
            float eased = (float)System.Math.Sqrt(ft); // ease-out: holds big, eases back
            var bt = GetTransform();
            bt.ScaleX = baseScaleX * (1f + BeatScaleAmp * eased);
            bt.ScaleY = baseScaleY * (1f + BeatScaleAmp * eased);
        }

        // =========================
        // Display Timer
        // =========================
        if (isShowing)
        {
            displayTimer -= Time.DeltaTime;
            if (displayTimer <= 0)
                Hide();
        }
    }

    // =========================
    // Player Fill Animation
    // =========================
    public void StartCountdown(char expectedKey)
    {
        if (IsEnemySlot) return;

            // DEBUG
        if (Conductor.Instance != null)
        {
            float songTime = Conductor.Instance.GetSongTime();
            float beatDuration = Conductor.Instance.GetBeatDuration();
            float timeUntilBeat = beatDuration - (songTime % beatDuration);
            Console.WriteLine($"[ComboKeySlot] Slot {SlotIndex} StartCountdown | timeUntilNextBeat={timeUntilBeat:F3}");
        }

        correctKey = expectedKey;

        var sprite = GetSprite();
        sprite.Texture = BlankSprite;
        sprite.SetTint(1f, 1f, 1f, 1f); // white tint so blank key is visible

        var transform = GetTransform();
        transform.IsVisible = true;
        transform.ScaleX = 0f;
        transform.ScaleY = 0f;

        isScaling = true;
        scaleTimer = 0f;
        isShowingResult = false;
        isShowingCorrectHint = false;
    }

    public void ShowCorrect(char key)
    {
        if (IsEnemySlot) return;

        isScaling = false;
        isShowingResult = false;
        isShowingCorrectHint = false;
        _inFadeIn = false;
        _isFadingToDim = false;
        GetFade().SetAlpha(1.0f);

        var sprite = GetSprite();
        sprite.Texture = GetGreenSpriteForKey(key);
        sprite.SetTint(1f, 1f, 1f, 1f);

        var transform = GetTransform();
        transform.IsVisible = true;
        transform.ScaleX = baseScaleX;
        transform.ScaleY = baseScaleY;

        isPoppingCorrect = true;
        correctPopTimer = 0f;
    }

    public void ShowWrong(char pressedKey, char expectedKey)
    {
        if (IsEnemySlot) return;

        isScaling = false;
        isShowingResult = false;
        isShowingCorrectHint = false;
        _inFadeIn = false;
        _isFadingToDim = false;
        GetFade().SetAlpha(1.0f);

        var sprite = GetSprite();
        sprite.Texture = GetRedSpriteForKey(pressedKey);
        sprite.SetTint(1f, 1f, 1f, 1f);

        var transform = GetTransform();
        transform.IsVisible = true;
        transform.ScaleX = baseScaleX;
        transform.ScaleY = baseScaleY;
    }   

    private void ShowCorrectHint()
    {
        var sprite = GetSprite();
        sprite.Texture = GetGreenSpriteForKey(correctKey);
        sprite.SetTint(1f, 1f, 1f, 1f);

        var transform = GetTransform();
        transform.IsVisible = true;
        transform.ScaleX = baseScaleX;
        transform.ScaleY = baseScaleY;
    }

    // =========================
    // Display Control
    // =========================
    public void ShowKey(char key)
    {
        var sprite = GetSprite();
        if (!sprite.HasSprite())
        {
            Console.WriteLine("[ComboKeySlot] ERROR: No sprite component!");
            return;
        }

        sprite.Texture = GetSpriteForKey(key);

        var fade = GetFade();
        fade.SetAlpha(1.0f);

        var transform = GetTransform();
        transform.IsVisible = true;

        isShowing = true;
        displayTimer = DisplayDuration;

        Console.WriteLine($"[ComboKeySlot] Slot {SlotIndex} ({(IsEnemySlot ? "Enemy" : "Player")}) showing {key}");
    }

    public void ShowKeyPersistent(char key)
    {
        var sprite = GetSprite();
        if (!sprite.HasSprite())
        {
            Console.WriteLine("[ComboKeySlot] ERROR: No sprite component!");
            return;
        }

        sprite.Texture = GetSpriteForKey(key);

        var fade = GetFade();
        fade.SetAlpha(1.0f);

        var transform = GetTransform();
        transform.IsVisible = true;

        isShowing = false;
        displayTimer = 0f;

        Console.WriteLine($"[ComboKeySlot] Slot {SlotIndex} ({(IsEnemySlot ? "Enemy" : "Player")}) showing {key} (persistent)");
    }

    public void FadeOutAndHide(float duration)
    {
        var fade = GetFade();
        fade.FadeOut(duration);

        isShowing = true;
        displayTimer = duration;

        Console.WriteLine($"[ComboKeySlot] Slot {SlotIndex} ({(IsEnemySlot ? "Enemy" : "Player")}) fading out over {duration}s");
    }

    public void HideInstant()
    {
        var sprite = GetSprite();
        if (sprite.HasSprite())
            sprite.SetTint(1f, 1f, 1f, 1f); // reset tint

        var fade = GetFade();
        fade.SetAlpha(1.0f);

        var transform = GetTransform();
        transform.IsVisible = false;
        transform.ScaleX = baseScaleX > 0f ? baseScaleX : 1f;
        transform.ScaleY = baseScaleY > 0f ? baseScaleY : 1f;

        isShowing = false;
        isScaling = false;
        isShowingResult = false;
        isShowingCorrectHint = false;
        isPoppingCorrect = false;
        _inFadeIn = false;
        _isFadingToDim = false;
    }

    public void Hide()
    {
        var transform = GetTransform();
        transform.IsVisible = false;
        isShowing = false;
    }

    // =========================
    // Helper Funcs
    // =========================
    public bool GetIsEnemySlot() => IsEnemySlot;

    private string GetSpriteForKey(char key)
    {
        switch (key)
        {
            case 'W': return WSprite;
            case 'A': return ASprite;
            case 'S': return SSprite;
            case 'D': return DSprite;
            default:
                Console.WriteLine($"[ComboKeySlot] Unknown key: {key}");
                return WSprite;
        }
    }

    private string GetGreenSpriteForKey(char key)
    {
        switch (key)
        {
            case 'W': return GreenWSprite;
            case 'A': return GreenASprite;
            case 'S': return GreenSSprite;
            case 'D': return GreenDSprite;
            default: return GreenWSprite;
        }
    }

    private string GetRedSpriteForKey(char key)
    {
        switch (key)
        {
            case 'W': return RedWSprite;
            case 'A': return RedASprite;
            case 'S': return RedSSprite;
            case 'D': return RedDSprite;
            default: return RedWSprite;
        }
    }

    // FOR PLAYER SLOTS: snap from dim to full + pop, called at player turn start
    public void ShowBlankPop(char expectedKey)
    {
        if (IsEnemySlot) return;

        correctKey = expectedKey;

        var sprite = GetSprite();
        if (!sprite.HasSprite()) return;

        sprite.Texture = BlankSprite;
        sprite.SetTint(1f, 1f, 1f, 1f);

        var fade = GetFade();
        fade.SetAlpha(0.2f); // start dim, fade-in to full over one beat

        var transform = GetTransform();
        transform.IsVisible = true;
        transform.ScaleX = baseScaleX * PopScale;
        transform.ScaleY = baseScaleY * PopScale;

        isPopping = true;
        popTimer = 0f;
        isScaling = false;
        isShowingResult = false;
        isShowingCorrectHint = false;
        isPoppingCorrect = false;
        _isFadingToDim = false;

        _fadeInDuration = Conductor.Instance != null ? Conductor.Instance.SecondsPerBeat : 0.5f;
        _fadeInTimer = 0f;
        _inFadeIn = true;
    }

    // FOR PLAYER SLOTS: show blank/dim during enemy turn (not yet pressable)
    public void ShowBlankDim(char expectedKey)
    {
        if (IsEnemySlot) return;

        correctKey = expectedKey;

        var sprite = GetSprite();
        if (!sprite.HasSprite()) return;

        sprite.Texture = BlankSprite;
        sprite.SetTint(1f, 1f, 1f, 1f);

        var fade = GetFade();
        fade.SetAlpha(0.2f); // dim — not pressable yet

        var transform = GetTransform();
        transform.IsVisible = true;
        transform.ScaleX = baseScaleX;
        transform.ScaleY = baseScaleY;

        isPopping = false;
        isScaling = false;
        isShowingResult = false;
        isShowingCorrectHint = false;
        isPoppingCorrect = false;
        _inFadeIn = false;
        _isFadingToDim = false;
    }

    // FOR PLAYER SLOTS: smoothly fade from full opacity back to dim when enemy turn resets
    public void ShowBlankFadeToDim(char expectedKey, float duration)
    {
        if (IsEnemySlot) return;

        correctKey = expectedKey;

        var sprite = GetSprite();
        if (!sprite.HasSprite()) return;

        sprite.Texture = BlankSprite;
        sprite.SetTint(1f, 1f, 1f, 1f);

        var transform = GetTransform();
        bool wasVisible = transform.IsVisible;
        transform.IsVisible = true;
        transform.ScaleX = baseScaleX;
        transform.ScaleY = baseScaleY;

        isPopping = false;
        isScaling = false;
        isShowingResult = false;
        isShowingCorrectHint = false;
        isPoppingCorrect = false;
        _inFadeIn = false;

        var fade = GetFade();
        if (wasVisible)
        {
            fade.SetAlpha(1.0f);
            _fadeToDimDuration = duration;
            _fadeToDimTimer    = 0f;
            _isFadingToDim     = true;
        }
        else
        {
            fade.SetAlpha(0.2f);
            _isFadingToDim = false;
        }
    }

    // Trigger a brief scale pulse on this slot (called beat-by-beat during player turn)
    public void TriggerScalePulse(float duration)
    {
        if (IsEnemySlot) return;
        _scalePulseTotalDur = duration;
        _scalePulseTimer    = duration;
    }

    // FOR ENEMY INPUT EFFECT ONLY!!!!
    public void ShowKeyPop(char key)
    {
        var sprite = GetSprite();
        if (!sprite.HasSprite()) return;

        sprite.Texture = GetSpriteForKey(key);

        var fade = GetFade();
        fade.SetAlpha(1.0f);

        var transform = GetTransform();
        transform.IsVisible = true;
        transform.ScaleX = baseScaleX * PopScale;
        transform.ScaleY = baseScaleY * PopScale;

        isPopping = true;
        popTimer = 0f;
        isShowing = false;
    }
}