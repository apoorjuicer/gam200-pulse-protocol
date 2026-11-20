# Pulse Protocol: 2D Rhythm Game in a Custom C++ Engine

A 2D rhythm game built on our team's own real-time C++ engine with an
in-house ImGui editor. 8-person team (1 artist, 1 designer, 6 programmers), 2 trimesters, DigiPen Institute of
Technology Singapore.


## My part (engine and tools programmer)
- **Editor undo/redo**: undo manager and snapshot system for the in-house
  ImGui editor, covering entity create/delete (including parent-child),
  gizmo moves, and edits to 9 component types (Transform, Renderable,
  Animation, AABB, Text, Health, Audio, Layer, logic). Added ECS support
  for recreating an entity with a specific ID so undone deletes restore
  the same entity.
- **Object allocator**: wrote the engine's object allocator so allocation
  doesn't rely on the general-purpose heap, plus a benchmark comparing the
  two, and prototyped storing ECS component arrays in it.
- **Asset pipeline**: JSON-driven asset registry for textures, audio and
  fonts; per-scene asset manifests plus a shared-assets set, so each scene
  loads and unloads only what it uses.
- **Audio manager (FMOD)**: audio component, BGM loading and playback
  sync for the rhythm gameplay.
- **Serialization**: template-based JSON save/load for engine objects
  (RapidJSON), prefab and config loading.
- **Graphics manager**: wraps a teammate's graphics API behind one
  interface so the rest of the engine never calls it directly; added the
  Text component.
- **Math library**: Vec2/Mat3 with GLM conversion, exposed to C# scripts
  through a Vector2 scripting API.
- **Gameplay scripts (C#)**: level-start and pause-resume 3-2-1
  countdowns synced to the conductor and music, cutscene manager, turn
  announcer UI.

## Tech
C++, C#, OpenGL, ImGui, RapidJSON, FMOD, Visual Studio

## Team
- Lau Chloe Rey En
- Chia Wei Xuan Rachael
- Ban Kai Wei Benjamin
- Carrie Lam Tze Ying
- Goh Pinkai
- Leu Junyong
- Chong Sze Ting Fenny
- Reginald Lew

Original repository hosted on DigiPen's Git server; this is a mirror for
portfolio purposes.
