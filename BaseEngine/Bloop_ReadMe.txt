================================================================================

              ____  _   _ _     ____  _____ 
             |  _ \| | | | |   / ___|| ____|
             | |_) | | | | |   \___ \|  _|  
             |  __/| |_| | |___ ___) | |___ 
             |_|    \___/|_____|____/|_____|
                                                   
          ____  ____   ___ _____ ___   ____ ___  _     
         |  _ \|  _ \ / _ \_   _/ _ \ / ___/ _ \| |    
         | |_) | |_) | | | || || | | | |  | | | | |    
         |  __/|  _ <| |_| || || |_| | |__| |_| | |___ 
         |_|   |_| \_\\___/ |_| \___/ \____\___/|_____|

                 ~ Rhythm-Action Combat ~

================================================================================


[ 1. GAME CONCEPT ]

Pulse Protocol is a rhythm-action combat game set in a cyberworld.

Players execute attacks by pressing keys in time with the beat. A visual border
pulses on screen every 0.5 seconds (120 BPM), accompanied by a sound effect,
to help players stay on rhythm.

The system evaluates timing accuracy and provides feedback:
- PERFECT : Within 0.05s of the beat
- GOOD    : Within 0.15s of the beat
- MISS    : More than 0.15s off the beat


[ 2. CUSTOM DEMO INPUT & USAGE ]

Gameplay:
---------
- A countdown sequence plays: "Ready" → "3" → "2" → "1" → "GO!"
- Once the countdown finishes, the enemy automatically begins
- Press W then A in rhythm with the pulsing border to attack
- Watch for on-screen feedback showing timing accuracy (PERFECT/GOOD/MISS)

Attack Combo:
W → A (press in rhythm with the beat)

Editor Controls:
----------------
- Play Button    : Starts the game and switches to Game Camera
- Stop Button    : Stops the game and switches back to Editor Camera
- Pause Button   : Pauses the game while in Game Camera
- Project Panel  : Displays all project assets
- Inspector      : Shows all components of selected entity
- Layer Panel    : Located beside the Inspector tab, allows locking/hiding layers

Developer Controls:
-------------------
F1    - Toggle Debug Rectangles
F2    - Toggle Debug Lines
F3    - Toggle Debug Circles
F4    - Toggle Debug Circles Filled
F5    - Toggle Debug Points
F6    - Toggle Debug Grid
F7    - Stress test (spawn 1000 entities)
F8    - Toggle Debug Overlay (FPS counter, performance stats)
F11   - Toggle Fullscreen
Space - Queue delayed combo message (test feature)

[ 3. TEAM ROSTER ]

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
================================================================================