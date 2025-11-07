═══════════════════════════════════════════════════════════════════════════════
1. GAME CONCEPT
═══════════════════════════════════════════════════════════════════════════════

Pulse Protocol is a rhythm-based combat game and it takes place in a cyberworld.

Players execute attacks and combos by pressing keys in time with the beat.
The system evaluates timing accuracy and provides feedback:

- PERFECT : On-beat input
- GOOD    : Slightly off-beat
- MISS    : Out of sync

The focus of the demo is to showcase:
- Our custom ECS-based engine
- Rhythm-timing input system
- Collision system with monster and hero
- Scene loading and saving
- Working GUI system
- Factory system for creating entities
- Graphics rendering system with sprite support
- Audio system with FMOD integration
- Prefab system for entity templates
- Play/Pause/Stop game state controls

═══════════════════════════════════════════════════════════════════════════════
2. CUSTOM DEMO INPUT & USAGE
═══════════════════════════════════════════════════════════════════════════════

Gameplay Controls:
------------------
W, A, S, D - Attack keys (must be pressed in rhythm with the beat)

Player Movement:
T - Move Up
F - Move Down
G - Move Left
H - Move Right

Enemy Control:
↑ (Up Arrow)    - Move enemy up
↓ (Down Arrow)  - Move enemy down
← (Left Arrow)  - Move enemy left
→ (Right Arrow) - Move enemy right

Timing System:
--------------
- Listen for the background music beat
- Press directional keys synchronized with the beat
- Watch for on-screen feedback showing timing accuracy
- Successful hits build combo meter
- Missed timing breaks combo chain

Developer Controls:
-------------------
F1-F6 - Debug Tools
F7  - Stress test (spawn 1000 entities)

Play/Pause/Stop System:
------------------------
The engine includes runtime game state controls:
- Play: Starts/resumes game systems and gameplay
- Pause: Freezes gameplay while maintaining editor access
- Stop: Resets the scene to its original state
- All entities and components can be edited in real-time
- Scene changes persist when saved to JSON files

How to Run:
-----------
1. Extract BaseEngine.zip
2. Double-click run.bat to generate Visual Studio solution
3. Open build/BaseEngine.sln in Visual Studio 2022
4. Set MyEngine as startup project
5. Press F5 to build and run
6. Control player with TFGH keys in rhythm with the music
7. Control enemy with Arrow keys (for testing/demonstration)

═══════════════════════════════════════════════════════════════════════════════
3. TEAM ROSTER: NAME - ROLE | CHAMPIONING
═══════════════════════════════════════════════════════════════════════════════

Chloe Lau Rey En (IMGD)
- Role: Product Manager
- Champion: Input & Debugging

Reginald Lew Yee Ren (IMGD)
- Role: Programmer
- Champion: Physics & Collision

Leu Jun Yong (IMGD)
- Role: Programmer
- Champion: Gameplay/AI & Level Design

Chia Wei Xuan, Rachael (RTIS)
- Role: Tech Lead
- Champion: Production

Goh Pin Kai (RTIS)
- Role: Programmer
- Champion: Graphics

Ban Kai Wei Benjamin (RTIS)
- Role: Programmer
- Champion: Engine

Carrie Lam Tze Ying (UXGD)
- Role: Design Lead
- Champion: Audio

Chong Sze Ting Fenny (BFA)
- Role: Art Lead
- Champion: Art

═══════════════════════════════════════════════════════════════════════════════