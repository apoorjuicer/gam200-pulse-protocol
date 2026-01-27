using ScriptAPI;

// ==========================
// KNOBS
// ==========================
public class VolumeKnob : AudioSliderKnobBase { protected override string FixedTag => "Volume"; }
public class MusicKnob  : AudioSliderKnobBase { protected override string FixedTag => "Music"; }
public class SfxKnob    : AudioSliderKnobBase { protected override string FixedTag => "SFX"; }
public class BrightnessKnob : AudioSliderKnobBase { protected override string FixedTag => "Brightness"; }

// ==========================
// FILLS
// ==========================
public class VolumeFill : AudioSliderFillBase { protected override string FixedTag => "Volume"; }
public class MusicFill  : AudioSliderFillBase { protected override string FixedTag => "Music"; }
public class SfxFill    : AudioSliderFillBase { protected override string FixedTag => "SFX"; }
public class BrightnessFill : AudioSliderFillBase { protected override string FixedTag => "Brightness"; }
