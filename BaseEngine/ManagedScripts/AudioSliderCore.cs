using System;
using System.Collections.Generic;
using System.Reflection;
using ScriptAPI;

// =========================================================
// Base (ABSTRACT) scripts: engine should NOT list abstract
// =========================================================

public abstract class AudioSliderKnobBase : Script
{
    // Must be provided by wrappers (Volume/Music/Sfx/Brightness)
    protected abstract string FixedTag { get; }

    // Empty bar width
    public float BarWidth = 1140.0f * 0.98f;

    // 2.0 => drag 2x farther, value changes 2x slower
    public float DragDistanceScale = 2.0f;

    public float TrackInset = 0.0f;
    public bool LockKnobY = true;

    // Start knob at MAX by default (100%)
    public bool StartAtMax = true;

    // Bigger clickable area (diamond -> bigger AABB)
    public float KnobHitboxScaleX = 2.2f;
    public float KnobHitboxScaleY = 2.2f;
    public float KnobHitboxPadX = 0.0f;
    public float KnobHitboxPadY = 0.0f;

    // ---- Published per tag ----
    public struct SliderState
    {
        public float Value01;

        public float TrackLeftX;
        public float TrackRightX;
        public float TrackY;

        public float FillLeftX;
        public float FillWidth;
    }

    public static Dictionary<string, SliderState> States = new Dictionary<string, SliderState>();

    // ---- runtime ----
    private bool initialized = false;
    private float lockedY;

    private float barLeftOuterX;
    private float knobMinX, knobMaxX;

    private bool dragging = false;
    private bool dragJustStarted = false;   // ✅ FIX: prevents instant cancel on click frame
    private float dragOffsetX = 0.0f;

    // robust input binding
    private object cachedInputObj;

    private MethodInfo heldMI = null;
    private Type heldParamType = null;

    private MethodInfo releasedMI = null;
    private Type releasedParamType = null;

    private bool toggleFallback = false;

    public override void Update()
    {
        InputComponent input = GetInput();
        TransformComponent t = GetTransform();

        BindInputIfNeeded(input);

        if (!initialized)
        {
            lockedY = t.Y;

            // SAVE the value BEFORE anything else
            float savedValue = -1f;
            if (States.ContainsKey(FixedTag))
            {
                savedValue = States[FixedTag].Value01;
            }

            float knobHalfW = t.ScaleX * 0.5f;

            if (StartAtMax)
            {
                // Assume knob starts at MAX
                float knobRightEdge = t.X + knobHalfW;
                float trackRightOuterX = knobRightEdge + TrackInset;
                barLeftOuterX = trackRightOuterX - (BarWidth * DragDistanceScale);
            }
            else
            {
                // Assume knob starts at MIN
                float knobLeftEdge = t.X - knobHalfW;
                barLeftOuterX = knobLeftEdge - TrackInset;
            }

            RecomputeTrack(t);

            // RESTORE knob position from saved value
            if (savedValue >= 0f)
            {
                t.X = knobMinX + (savedValue * (knobMaxX - knobMinX));
            }
            else
            {
                t.X = Clamp(t.X, knobMinX, knobMaxX);
            }
            
            if (LockKnobY) t.Y = lockedY;

            PublishState(ToValue01(t.X));
            initialized = true;
        }
        else
        {
            RecomputeTrack(t);

            if (States.ContainsKey(FixedTag) && States[FixedTag].Value01 > 0.001f)
            {
                float savedValue = States[FixedTag].Value01;
                t.X = knobMinX + (savedValue * (knobMaxX - knobMinX));
            }
            else
            {
                t.X = Clamp(t.X, knobMinX, knobMaxX);
            }
            
            if (LockKnobY) t.Y = lockedY;

            PublishState(ToValue01(t.X));
            initialized = true;
        }

        float mx = input.WorldMouseX;
        float my = input.WorldMouseY;

        // Enlarged knob hitbox (AABB)
        float hitW = (t.ScaleX * KnobHitboxScaleX) + (KnobHitboxPadX * 2.0f);
        float hitH = (t.ScaleY * KnobHitboxScaleY) + (KnobHitboxPadY * 2.0f);
        bool knobHovered = PointInRect(mx, my, t.X, t.Y, hitW, hitH);

        // Bar hover (for click-to-jump)
        var st = GetState();
        float hoverHalfH = Math.Max(20.0f, t.ScaleY * 1.5f);
        bool barHovered =
            (mx >= st.TrackLeftX && mx <= st.TrackRightX &&
             my >= lockedY - hoverHalfH && my <= lockedY + hoverHalfH);

        bool click = input.IsMouseButtonTriggered(0);
        bool held = MouseHeld(input, 0);
        bool released = MouseReleased(input, 0);

        // Start drag
        if (!dragging && click && (knobHovered || barHovered))
        {
            dragging = true;
            dragJustStarted = true; 
            dragOffsetX = knobHovered ? (t.X - mx) : 0.0f;

            // Click bar (not knob) -> jump immediately
            if (barHovered && !knobHovered)
            {
                t.X = Clamp(mx, knobMinX, knobMaxX);
                if (LockKnobY) t.Y = lockedY;
                PublishState(ToValue01(t.X));
            }
        }

        // Stop drag (AFTER grace frame)
        if (dragging && !dragJustStarted)
        {
            if (!toggleFallback)
            {
                // stop if mouse not held OR released
                if (!held || released) dragging = false;
            }
            else
            {
                // fallback: stop on next click (toggle behavior)
                if (click && !(knobHovered || barHovered)) dragging = false;
            }
        }

        // Drag movement (ONLY when held, except fallback)
        if (dragging && (toggleFallback || held))
        {
            t.X = Clamp(mx + dragOffsetX, knobMinX, knobMaxX);
            if (LockKnobY) t.Y = lockedY;
            PublishState(ToValue01(t.X));
        }


        // End grace frame
        dragJustStarted = false;

        // Auto-apply to audio mixer (Brightness does nothing)
        ApplyToAudioMixer(GetState().Value01);
    }

    private void ApplyToAudioMixer(float v01)
    {
        string tag = FixedTag ?? "";

        if (tag.Equals("Volume", StringComparison.OrdinalIgnoreCase))
            GetAudio().SetMasterVolume(v01);
        else if (tag.Equals("Music", StringComparison.OrdinalIgnoreCase))
            GetAudio().SetMusicVolume(v01);
        else if (tag.Equals("SFX", StringComparison.OrdinalIgnoreCase) || tag.Equals("Sfx", StringComparison.OrdinalIgnoreCase))
            GetAudio().SetSfxVolume(v01);
    }

    private void RecomputeTrack(TransformComponent knobT)
    {
        float knobHalfW = knobT.ScaleX * 0.5f;

        // Visual bar (fill range) stays BarWidth
        float visualRightOuterX = barLeftOuterX + BarWidth;

        float fillLeft = barLeftOuterX + TrackInset;
        float fillRight = visualRightOuterX - TrackInset;
        if (fillRight < fillLeft) fillRight = fillLeft;

        float fillWidth = fillRight - fillLeft;

        // Drag track is longer
        float effectiveBarWidth = BarWidth * DragDistanceScale;
        float trackRightOuterX = barLeftOuterX + effectiveBarWidth;

        float trackLeft = barLeftOuterX + TrackInset;
        float trackRight = trackRightOuterX - TrackInset;
        if (trackRight < trackLeft) trackRight = trackLeft;

        knobMinX = trackLeft + knobHalfW;
        knobMaxX = trackRight - knobHalfW;

        if (knobMaxX < knobMinX)
        {
            knobMinX = knobT.X;
            knobMaxX = knobT.X;
        }

        SliderState st = GetState();
        st.TrackLeftX = trackLeft;
        st.TrackRightX = trackRight;
        st.TrackY = lockedY;

        st.FillLeftX = fillLeft;
        st.FillWidth = fillWidth;

        States[FixedTag] = st;
    }

    private void PublishState(float value01)
    {
        SliderState st = GetState();
        st.Value01 = Clamp(value01, 0.0f, 1.0f);
        st.TrackY = lockedY;
        States[FixedTag] = st;
    }

    private SliderState GetState()
    {
        if (!States.ContainsKey(FixedTag))
            States[FixedTag] = new SliderState();
        return States[FixedTag];
    }

    private float ToValue01(float knobX)
    {
        float denom = (knobMaxX - knobMinX);
        if (denom <= 0.0001f) return 0.0f;
        return Clamp((knobX - knobMinX) / denom, 0.0f, 1.0f);
    }

    private bool PointInRect(float px, float py, float cx, float cy, float w, float h)
    {
        float halfW = w * 0.5f;
        float halfH = h * 0.5f;
        return px >= cx - halfW && px <= cx + halfW &&
               py >= cy - halfH && py <= cy + halfH;
    }

    private float Clamp(float v, float lo, float hi)
    {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }

    // =========================
    // Robust mouse binding
    // =========================
    private void BindInputIfNeeded(InputComponent input)
    {
        if (cachedInputObj == (object)input) return;
        cachedInputObj = input;

        //  include Pressed as a fallback too (some engines misname these)
        heldMI = FindBool1ParamMethod(input, new string[] { "IsMouseButtonDown", "IsMouseButtonHeld", "IsMouseButtonPressed" }, out heldParamType);
        releasedMI = FindBool1ParamMethod(input, new string[] { "IsMouseButtonReleased", "IsMouseButtonUp" }, out releasedParamType);

        toggleFallback = (heldMI == null && releasedMI == null);
    }

    private MethodInfo FindBool1ParamMethod(object obj, string[] names, out Type paramType)
    {
        paramType = null;
        Type tp = obj.GetType();

        foreach (string name in names)
        {
            MethodInfo best = null;
            Type bestParam = null;

            foreach (var mi in tp.GetMethods(BindingFlags.Instance | BindingFlags.Public))
            {
                if (mi.Name != name) continue;
                if (mi.ReturnType != typeof(bool)) continue;

                var ps = mi.GetParameters();
                if (ps == null || ps.Length != 1) continue;

                var pt = ps[0].ParameterType;

                if (best == null)
                {
                    best = mi; bestParam = pt;
                }
                else
                {
                    if (pt == typeof(int) && bestParam != typeof(int)) { best = mi; bestParam = pt; }
                    else if (pt == typeof(uint) && bestParam != typeof(int) && bestParam != typeof(uint)) { best = mi; bestParam = pt; }
                    else if (pt.IsEnum && !(bestParam == typeof(int) || bestParam == typeof(uint) || bestParam.IsEnum)) { best = mi; bestParam = pt; }
                }
            }

            if (best != null)
            {
                paramType = bestParam;
                return best;
            }
        }

        return null;
    }

    private bool MouseHeld(InputComponent input, int button)
    {
        if (heldMI == null || heldParamType == null) return dragging;
        object arg = ConvertButtonArg(heldParamType, button);

        try { return (bool)heldMI.Invoke(input, new object[] { arg }); }
        catch { return dragging; }
    }

    private bool MouseReleased(InputComponent input, int button)
    {
        if (releasedMI == null || releasedParamType == null) return false;
        object arg = ConvertButtonArg(releasedParamType, button);

        try { return (bool)releasedMI.Invoke(input, new object[] { arg }); }
        catch { return false; }
    }

    private object ConvertButtonArg(Type pt, int button)
    {
        if (pt == typeof(int)) return button;
        if (pt == typeof(uint)) return (uint)button;
        if (pt.IsEnum) return Enum.ToObject(pt, button);
        return button;
    }
}

public abstract class AudioSliderFillBase : Script
{
    protected abstract string FixedTag { get; }

    // 0=center, 1=left, 2=right
    public int PivotMode = 2;

    public bool SnapToKnobY = true;
    public float YOffset = 0.0f;

    // Uniform thickness
    public bool ForceUniformHeight = true;
    public float UniformHeight = 20.0f;

    private bool initialized = false;
    private float baseY;
    private float baseH;

    public override void Update()
    {
        TransformComponent t = GetTransform();

        if (!initialized)
        {
            baseY = t.Y;
            baseH = t.ScaleY;
            initialized = true;
        }

        if (!AudioSliderKnobBase.States.ContainsKey(FixedTag))
        {
            t.ScaleX = 0.0f;
            t.ScaleY = ForceUniformHeight ? UniformHeight : baseH;
            t.Y = baseY;
            return;
        }

        var st = AudioSliderKnobBase.States[FixedTag];

        float maxW = st.FillWidth;
        float v = st.Value01;

        float newW = (maxW <= 0.0001f) ? 0.0f : (maxW * v);

        t.ScaleX = newW;
        t.ScaleY = ForceUniformHeight ? UniformHeight : baseH;

        float left = st.FillLeftX;

        if (PivotMode == 0)      t.X = left + (newW * 0.5f);
        else if (PivotMode == 1) t.X = left;
        else                     t.X = left + newW;

        t.Y = SnapToKnobY ? (st.TrackY + YOffset) : baseY;
    }
}
