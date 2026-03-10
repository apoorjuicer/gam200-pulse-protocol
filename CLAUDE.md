# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

All commands run from `BaseEngine/` using `run.bat`:

```bat
run.bat editor Debug          # build + launch PulseEditor (default)
run.bat protocol Release      # build + launch PulseProtocol (game)
run.bat build Debug --norun   # build only, no launch
run.bat sln                   # open MyEngine.sln in Visual Studio
run.bat all Debug --reconfigure  # force CMake reconfigure + build all
```

**Important:** Run `clean.bat` when switching between editor and game mode — the two share incompatible CMake configurations.

- Requires: Visual Studio 2022, CMake 3.20+, .NET 8 SDK
- Solution: `build/MyEngine.sln` — startup project is `PulseEditor`
- Third-party deps (glfw, glew, glm, freetype, RapidJSON) are fetched via CMake FetchContent into `build/libraries/`

## Project Structure

```
BaseEngine/
├── PulseEngine/          # C++ engine core (compiled as DLL)
│   ├── CoreEngine/
│   │   ├── Core/         # CoreEngine.cpp — main loop, .NET bridge, pause/play
│   │   ├── ECS/          # Entity-Component-System (Coordinator, ComponentManager, SystemManager)
│   │   ├── Scene/        # Scene loading/unloading
│   │   ├── Factory/      # Prefab instantiation/destruction
│   │   └── Asset/        # AssetsManager
│   ├── Audio/            # FMOD integration
│   ├── Graphics/         # OpenGL renderer, post-process, VFX, particles
│   ├── Input/            # GLFW input polling
│   ├── Physics/          # AABB / distance collision
│   └── Components/       # Health, EntityType, Transform, etc.
├── ScriptAPI/            # C++/CLI bridge (compiled as CLR DLL, .NET 8)
│   └── *.h / *.cpp       # Thin wrappers: TransformComponent, SpriteComponent, AudioAPI, VFXAPI, etc.
├── ManagedScripts/       # C# game scripts (.NET 8)
│   ├── API_Guide.cs      # Full ScriptAPI reference — read before writing scripts
│   ├── Conductor.cs      # Beat clock singleton (foundational — all rhythm scripts depend on this)
│   └── *.cs              # All other game scripts
├── PulseEditor/          # ImGui-based editor (Hierarchy, Inspector, ProjectPanel, etc.)
├── PulseProtocol/        # Game entry point
│   ├── Assets/Prefabs/   # Enemy + object prefabs (*.json)
│   └── JSON/             # Scene files (Level1.json, MainMenu.json, GameLose.json, etc.)
└── imgui/                # ImGui source (built inline)
```

## Architecture Overview

**Engine ↔ Script Bridge:**
`PulseEngine` (native C++) exposes subsystems through `ScriptAPI` (C++/CLI), which wraps them as managed types callable from C# `ManagedScripts`. At runtime, `CoreEngine` loads the .NET 8 runtime via CoreCLR, compiles `ManagedScripts` into an assembly, and calls `executeUpdateFunc()` each frame to drive all script `Update()` calls.

**ECS:**
Standard archetype-style ECS in `CoreEngine/ECS/`. `Coordinator` is the central hub — it owns `EntityManager`, `ComponentManager`, and `SystemManager`. Entities are plain integer IDs. Components are stored in typed `ComponentArray<T>` pools.

**Two Engine Variants:**
`PulseEngine` builds as two CMake targets:
- `PulseEngine_Editor` — linked by `PulseEditor`; editor play/pause/stop controls the game loop
- `PulseEngine_Game` — linked by `PulseProtocol`; runs the game standalone

## Game Loop & Turn System

The core gameplay is turn-based and rhythm-driven. `TurnManager.cs` drives four phases each turn:

```
Countdown (4 beats) → EnemyDisplay (4 beats) → PlayerInput (5 beats) → Resolution
    ↑_______________________________________________________________|
```

- **Countdown**: 3-2-1-GO! pre-game sequence run by `LevelCountdown.cs`. Sets `GameStart.Ready = true` when done.
- **EnemyDisplay**: Enemy's weakness combo is revealed one key per beat via `ComboDisplayManager.cs`. Player input is locked (`TurnManager.IsInputLocked`). Enemy steps toward the player on each beat.
- **PlayerInput**: Player enters W/A/S/D combo judged PERFECT/GREAT/MISS by `ComboSystem.cs`. Damage is applied **immediately** in `InputBridge.OnKeyPressed()` when the combo completes — not in Resolution.
- **Resolution**: If player failed, `EnemyRhythmController.ExecuteFailurePunishment()` moves remaining steps and attacks. Waits for death animation (2.5s) before cycling to next enemy.

Both `EnemyDisplay` and `PlayerInput` phases backdate `phaseStartBeat` by `-1` to absorb the transition beat, giving the effective beat counts described above.

`TurnManager` uses `Conductor.GetCurrentBeat()` (integer) to drive transitions. Check `TurnManager.IsInputLocked` before accepting player input.

## Rhythm & Beat Clock

`Conductor.cs` is the single source of truth for timing. It syncs to **FMOD audio position** (not DeltaTime) to avoid drift:

```csharp
// Conductor internally: songTime = audioTimeAccumulator + fmodAudioPos
float adjustedTime = songTime - MusicLatencyOffset - BeatStartOffset;
int beat = (int)Math.Floor(adjustedTime / secondsPerBeat);
```

- `MusicLatencyOffset` (default 0.125s) — compensates for hardware audio buffer lag
- `BeatStartOffset` (default 0.75s) — aligns beat 0 to the actual first beat in the audio file
- Both are tunable fields on the `Conductor` entity in the editor
- `LevelBGM` class in `BGMMgr.cs` calls `Conductor.ResetClock()` when level audio starts to sync beat tracking to t=0
- On BGM loop, `audioTimeAccumulator += prevAudioPos` so the beat counter never resets mid-session

**Key Conductor API:**
```csharp
Conductor.Instance.GetCurrentBeat()             // integer beat index
Conductor.Instance.GetSongTime()                // raw FMOD time (seconds)
Conductor.Instance.GetAdjustedSongTime()        // - MusicLatencyOffset - BeatStartOffset: use for combo judgment
Conductor.Instance.GetVisualAdjustedSongTime()  // + DisplayLatencyOffset on top: use for all visual beat queries
Conductor.Instance.SecondsPerBeat               // 60 / BPM
```

**Two adjusted clocks — use the right one:**
- `GetAdjustedSongTime()` → combo timing, TurnManager phase math (audio-aligned)
- `GetVisualAdjustedSongTime()` → VisualBeatSystem, any sprite that pulses on the beat (display-aligned)

**`VisualBeatSystem.cs`** drives the border pulse using `GetVisualAdjustedSongTime()`. **`ComboSystem.RegisterInput()`** uses `GetAdjustedSongTime()` for PERFECT/GREAT judgment.

## Singleton Chain

All major systems expose a static `Instance`. The initialization order at level start is:

```
Conductor → TurnManager → ComboSystem → EnemyRhythmController → EnemySpawner
```

- `GameStart.Ready` is a static bool gating all rhythm scripts. **Only `TurnManager.Initialize()`** resets it to `false`; only `LevelCountdown` sets it to `true`. Never reset it elsewhere.
- `EnemyRhythmController` sets the weakness on `ComboSystem` via `TrySetWeakness()`, retrying up to 100 frames until `ComboSystem.Instance` is non-null.
- `EnemySpawner` calls `OnEnemyDefeated()` after the 2.5s death animation, which spawns the next enemy prefab from its queue.
- `PlayerHealth.Instance.PlayerEntityID` is how `EnemyRhythmController` discovers the player entity for collision checks.

## Script Session-Reuse Gotcha

The engine may **not** recreate C# Script instances on editor Stop→Play. Every script that holds state must handle this. Two patterns in use:

1. **GameStart.Ready flip** (TurnManager): detect `initialized && prevGameReady && !GameStart.Ready` → re-initialize.
2. **entityID stamp** (EnemyRhythmController): compare `entityID != initializedForEntityID` → re-initialize.

When writing new stateful scripts, use one of these patterns.

## Input Flow

```
InputBridge.Update()
  → edge-detects W/A/S/D taps (prev/cur frame state)
  → checks TurnManager.IsInputLocked (blocks during EnemyDisplay/Countdown)
  → OnKeyPressed(char) → ComboSystem.RegisterInput(char)
      → judges timing vs GetAdjustedSongTime() % beatDuration
      → on combo complete → InputBridge applies damage: EnemyRhythmController.TakeDamage(10f)
                                                        PlayerAnimationController.PlayAttack()
```

PERFECT window: ≤ 0.08s from beat boundary. GREAT: ≤ 0.15s. MISS: frees the beat slot for retry on the same beat.

## Enemy System

`EnemyRhythmController.cs` is the base class. Add a new enemy type by subclassing it and overriding `SetupSprites()`:

```csharp
public class MyEnemy : EnemyRhythmController
{
    protected override void SetupSprites()
    {
        IdleSpriteSheet = "../Enemies/MyEnemy_Idle_spritesheet_WxH";
        WeaknessCombo = "WASD";
        // set frame counts, columns, rows...
    }
}
```

Enemy prefabs live in `PulseProtocol/Assets/Prefabs/` (e.g. `Rat_Enemy.json`, `Gorilla_Enemy.json`). `EnemySpawner` reads a queue of prefab names and calls `Factory.Instantiate()` for each. `CollisionMode = "Manual"` with `ManualAttackRange` is the most reliable collision approach.

## Scenes

All scenes are in `PulseProtocol/JSON/`. Scene name passed to `Scene.LoadScene()` must match the JSON filename stem:

| Scene | Purpose |
|---|---|
| `MainMenu` | Title screen with `GeneralBGM` |
| `Level1` | Main gameplay level with `LevelBGM` |
| `Level2` | Second level |
| `LatencyCalibration` | Audio latency calibration (`LatencyCalibrator.cs`); result saved to `Conductor.CalibratedOffset` |
| `VisualLatencyCalibration` | Visual A/V sync calibration (`VisualLatencyCalibrator.cs`); result saved to `Conductor.CalibratedDisplayOffset` |
| `Cutscene` | Intro cutscene (`CutscenePanel.cs`) |
| `GameWin` / `GameLose` | End screens |
| `Settings` | Settings overlay |
| `PauseMenu` | In-game pause overlay |

`audio.json`, `texture.json`, `font.json`, `layers.json` are asset manifests (not loadable scenes).

## Writing C# Scripts

All scripts extend `Script` and override `Update()`. See `ManagedScripts/API_Guide.cs` for the full API surface.

```csharp
public class MyScript : Script
{
    public static MyScript? Instance { get; private set; }

    public override void Update()
    {
        Instance ??= this;
        if (!GameStart.Ready) return;      // wait for countdown (defined in LevelCountdown.cs)
        if (Application.IsPaused()) return;

        TransformComponent t = GetTransform();
        t.X += 5f * Time.DeltaTime;
    }
}
```

**Key APIs (all via ScriptAPI wrappers):**
- `GetTransform()`, `GetSprite()`, `GetAnimation()`, `GetAudio()`, `GetHealth()`, `GetCollision()`, `GetInput()`, `GetFSM()`, `GetTextComponent()`, `GetFade()`, `GetGlow()`
- `Scene.LoadScene("Name")` — name must match the JSON stem in `PulseProtocol/JSON/`
- `Factory.Instantiate("PrefabName")` / `Factory.Destroy(id)` — prefab name matches JSON stem in `Assets/Prefabs/`
- `Time.DeltaTime` — returns 0 automatically when paused
- `Application.SetPaused(bool)` / `IsPaused()`
- `VFXAPI.BeatPulse()`, `VFXAPI.ShakeCamera(amplitude, freq)`, `VFXAPI.SpawnParticles(...)`
- `FadeAPI` — screen-wide overlay fade; `FadeComponent` — per-entity sprite fade
- `PostProcessAPI` — bloom (screen-wide); `GlowComponent` — per-entity glow halo

**Animation:**
```csharp
anim.SetAnimation("SpritePath", rows, cols, totalFrames, fps);
// Beat-synced fps: fps = totalFrames / SecondsPerBeat
```

**Script masterlist:** `ManagedScripts/C# Scripts Masterlist.md` — lists all active, superseded, and debug scripts. Keep it updated when adding/removing scripts.
