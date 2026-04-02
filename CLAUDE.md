# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Pulse Protocol** is a rhythm-action combat game built on a custom C++ game engine called **PulseEngine**. Players attack enemies by pressing keys in time with a 120 BPM beat. The engine supports a full level editor (PulseEditor) and a C# scripting layer for gameplay logic.

All work happens under `BaseEngine/`. The working directory when running the game/editor is `BaseEngine/build/`.

---

## Build Commands

Run from `BaseEngine/` root:

```bat
run.bat                        # Configure CMake, build, and run PulseEditor (default)
run.bat editor [Debug|Release] # Build and run editor
run.bat game   [Debug|Release] # Build and run game (PulseProtocol)
run.bat all    [Debug|Release] # Build all targets without running
run.bat sln                    # Open build/PulseEngine.sln in Visual Studio
run.bat clean                  # Delete entire build folder (same as clean.bat)
```

**To regenerate the solution** (required when switching active game or after CMake changes):
1. Close Visual Studio
2. Run `clean.bat`
3. Run `run.bat`
4. Reopen `build/PulseEngine.sln`

**Switching between Editor and Game mode** in Visual Studio: right-click `PulseEditor` or `PulseProtocol` in Solution Explorer → "Set as Startup Project" → F5. No CMake regeneration needed.

**Active game** is set by `BaseEngine/ActiveGame.json`. Changing it requires re-running `run.bat`.

---

## Prerequisites

- Visual Studio 2022 with:
  - .NET desktop development workload
  - Desktop development with C++ (C++/CLI support for v143 build tools)
- CMake 3.15+ (added to PATH)
- .NET 8.0 SDK
- .NET 8.0 Runtime DLLs (x64 binaries) placed in `BaseEngine/extern/dotnet/bin/`

---

## Architecture

### Engine Layers

```
PulseProtocol/ManagedScripts/   ← C# gameplay scripts (top layer)
ScriptAPI/                      ← C++/CLI bridge exposing engine to C#
PulseEngine/                    ← Core engine in C++20
PulseEditor/                    ← ImGui-based level editor (C++)
```

### ECS Pattern

The engine uses an Entity-Component-System architecture. The key ECS files are in `PulseEngine/CoreEngine/ECS/`: `Coordinator`, `EntityManager`, `ComponentManager`, `SystemManager`. Entities are IDs; components are plain data structs; systems contain logic and query entities by component type.

### C# Scripting

- Scripts live in `BaseEngine/PulseProtocol/ManagedScripts/*.cs`
- Every script extends `Script` and overrides `Update()`
- The ScriptAPI (`ScriptAPI/*.h`) is the C++/CLI bridge; see `API_Guide.cs` for the full scripting API reference
- To add a new script:
  1. Create `ManagedScripts/MyScript.cs`
  2. Open `ManagedScripts/ManagedScripts.sln` and add the file as an existing item
  3. Reference it in the entity's scene JSON: `"script": {"ScriptName": ["MyScript"]}`

### Scene Data

Scenes are JSON files in `PulseProtocol/JSON/` (e.g., `Level1.json`, `MainMenu.json`). Each scene has a corresponding `.manifest.json` listing which assets to load. Scenes are loaded by stem name: `Scene.LoadScene("Level1")`.

### Rhythm System

- `Conductor.cs` — singleton beat clock; `songTime += Time.DeltaTime` each frame; drives all rhythm logic
- `GameStart.Ready` — boolean gate; all rhythm scripts must check this before ticking
- `BGMMgr.cs` — calls `Conductor.ResetClock()` when audio starts to keep beat in sync
- `VisualBeatSystem.cs` — reads `Conductor.Instance.GetSongTime()` to pulse the screen border
- Beat window: PERFECT ≤0.5s, GOOD ≤1.0s, MISS >1.0s

---

## Key Scripting API Reference

See `ManagedScripts/API_Guide.cs` for runnable examples. Quick reference:

```csharp
// Component access (inside Update())
TransformComponent t = GetTransform();
SpriteComponent s    = GetSprite();
Animation anim       = GetAnimation();
InputComponent input = GetInput();
AudioComponent audio = GetAudio();
HealthComponent hp   = GetHealth();
CollisionComponent c = GetCollision();
var text             = GetTextComponent();

// Scene / Factory
Scene.LoadScene("SceneName");        // name must match JSON filename stem
int id = Factory.Instantiate("PrefabName");
Factory.Destroy(id);                 // destroy OTHER entity, not self

// Time
float dt = Time.DeltaTime;          // returns 0 when paused automatically

// Application
Application.IsPaused();
Application.SetPaused(true/false);
Application.IsPlaying();
Application.Quit();

// Animation
anim.SetAnimation("SpriteName", rows, cols, totalFrames, fps);

// VFX
VFXAPI.BeatPulse();
VFXAPI.ShakeCamera(amplitude, duration);
VFXAPI.SpawnParticles(x, y, count);

// Fade (screen-wide overlay)
FadeAPI.FadeOut(seconds);
FadeAPI.FadeIn(seconds);
FadeAPI.IsFadeDone();

// Input (GLFW key codes, not ASCII)
input.IsKeyTriggered(keyCode);      // pressed this frame
input.IsKeyPressed(keyCode);        // held
input.IsKeyReleased(keyCode);       // released this frame
input.IsMouseButtonTriggered(0);    // 0=left, 1=right, 2=middle
```

### Common Scripting Patterns

```csharp
// Singleton
public static MyScript? Instance { get; private set; }
public override void Update() {
    Instance ??= this;
    if (!GameStart.Ready) return;
    if (Application.IsPaused()) return;
    // ...
}

// Beat-synced animation fps
float fps = totalFrames / Conductor.Instance.SecondsPerBeat;
anim.SetAnimation("sprite", rows, cols, totalFrames, fps);

// Timed state duration
float stateDuration = frameCount / animationSpeed;
```

---

## Asset Registration

Textures **must** be registered in `PulseProtocol/JSON/texture.json` before they can be used. The `"id"` field is the key used everywhere in code and JSON:

```json
{
    "id": "my_sprite",
    "path": "Assets/Textures/VFX/My_Sprite_365x418.png",
    "vert_shader_path": "shader/bg_vs.vert",
    "frag_shader_path": "shader/bg_fs.frag",
    "mdl_ref": 1,
    "shd_ref": 2
}
```

Naming convention: lowercase, underscores, **dimension suffix stripped**. `Sword_Slash_VFX_1099x392.png` → `"sword_slash_vfx"`.

When updating a prefab to use a different texture, three things must change together in the prefab JSON:
- `render.spriteName` → the texture `id`
- `transform.scale` → match the image pixel dimensions
- `animation` grid → `rows`, `columns`, `totalFrames` of the new sheet

---

## Enemy Architecture

`EnemyRhythmController` is the **base class and the Rat enemy** — its field initializers are the rat's sprite settings. `Gorilla`, `BlueGorilla`, and `Bat` are subclasses that reassign inherited fields inside `SetupSprites()`, called during `Initialize()`.

To add a new enemy: subclass `EnemyRhythmController`, override `SetupSprites()`, create a prefab JSON.

Each enemy needs sprite sheets for: `Idle`, `Walk`, `Attack`, `Hurt`, and `Death`. Death animation duration is computed as `DeathFrames / AnimationSpeed` to ensure exactly one play-through.

`EnemySpawner.enemyQueue.Peek()` returns the current `EnemyIconType` and can be read from any script to branch per-enemy.

---

## VFX / Factory Pattern

VFX entities live in `Assets/Prefabs/` and are spawned via `Factory.Instantiate("PrefabName")`. Since `Instantiate` takes no arguments, parameters are passed through `static` fields on the script class **before** calling Instantiate:

```csharp
MyVFX.SpawnX = GetTransform().X;
MyVFX.SpawnDuration = 0.5f;
Factory.Instantiate("MyVFX");
```

The VFX script reads these statics on frame 2 of its `Update()` (frame 1 is the engine init frame — skip it with an `initFrames` counter). Self-destruction is done by setting `GetTransform().IsVisible = false` and `GetHealth().isAlive = false`. Always store `GetTransform()` / `GetHealth()` in a local variable before setting properties — they return structs by value (CS1612 otherwise).

---

## Developer Debug Controls (in-game)

| Key | Action |
|-----|--------|
| F1–F6 | Toggle debug draw overlays (rects, lines, circles, grid) |
| F8 | Toggle FPS/perf overlay |
| F11 | Toggle fullscreen |

---

## Important File Locations

| What | Where |
|------|-------|
| C# scripts | `BaseEngine/PulseProtocol/ManagedScripts/*.cs` |
| Script API reference | `ManagedScripts/API_Guide.cs` |
| Scene JSON files | `BaseEngine/PulseProtocol/JSON/*.json` |
| Prefab JSON files | `BaseEngine/PulseProtocol/Assets/Prefabs/` |
| Engine ECS core | `BaseEngine/PulseEngine/CoreEngine/ECS/` |
| C++/CLI bridge | `BaseEngine/ScriptAPI/` |
| Beat clock | `ManagedScripts/Conductor.cs` |
| Visual beat pulse | `ManagedScripts/VisualBeatSystem.cs` |
| Shaders | `BaseEngine/PulseEngine/Graphics/shader/` |
| Asset config JSONs | `PulseProtocol/JSON/audio.json`, `texture.json`, `animation.json`, `font.json` |
