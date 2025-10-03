Bloop_ReadMe
=======================

1. Game Concept
---------------
Pulse Protocol is a rhythm-based combat game developed on a custom C++ engine. 
Players execute attacks and combos by pressing keys in time with the beat. 
The system evaluates timing accuracy and provides feedback:
- PERFECT : On-beat input
- GOOD    : Slightly off-beat
- MISS    : Out of sync

The focus of the demo is to showcase:
- Our custom ECS-based engine
- Rhythm-timing input system
- Collision system with monster and hero
- Visual feedback integrated with ImGui overlays

2. Custom Demo Input & Usage
----------------------------
- Movement/Combo Input: **W, A, S, D** keys in timed sequences
- Rhythm detection: Inputs must match the beat cycle
- Feedback system: On-screen entity appears for a short second
- Debug overlay: **Toggle ImGui windows with key `O`**

Steps to run:
- Open the solution (`BaseEngine.sln`) in Visual Studio 2022
- Build and run the **MyEngine** project
- Play the demo using keyboard inputs synchronized with the beat

3. Team Roster: Name - Role | Championing
----------------------------------------
- Chloe Lau Rey En (IMGD) – Product Manager | Input & Debugging Champion
- Reginald Lew Yee Ren (IMGD) – Designer | Physics & Collision Champion
- Leu Jun Yong (IMGD) – Programmer | Gameplay/AI/Level Design Champion
- Chia Wei Xuan, Rachael (RTIS) – Tech Lead | Production Champion
- Goh Pin Kai (RTIS) – Programmer | Graphics Champion
- Ban Kai Wei Benjamin (RTIS) – Programmer | Engine Champion
- Carrie Lam Tze Ying (UXGD) – Design Lead | Audio Champion
- Chong Sze Ting Fenny (BFA) – Art Lead | Art Champion
