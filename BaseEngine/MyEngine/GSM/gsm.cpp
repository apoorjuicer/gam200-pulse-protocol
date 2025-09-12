#include "pch/pch.h"

// ----------------------------------------------------------------------------
//
// Variables to keep track the initial, current, previous and next game state
//
// ----------------------------------------------------------------------------
//GS_STATE	gGameStateInit;
//GS_STATE	gGameStateCurr;
//GS_STATE	gGameStatePrev;
//GS_STATE	gGameStateNext;

// ----------------------------------------------------------------------------
//
// Pointers to functions for game state life cycles functions
//
// ----------------------------------------------------------------------------
void (*GameStateLoad)() = 0;
void (*GameStateInit)() = 0;
void (*GameStateUpdate)() = 0;
void (*GameStateDraw)() = 0;
void (*GameStateFree)() = 0;
void (*GameStateUnload)() = 0;

void GS_init()
{

}

void GS_update()
{

}

void GS_exit()
{

}


// first, load all before application loop (engine_init)
/*
For level, we extract all the data from engine first.
Then in the game loop, when gsm_level changes, we ask resource manager for data and do:

GameStateLoad	= GameStateLoad;
GameStateInit	= GameStateCageInit;
GameStateUpdate	= GameStateCageUpdate;
GameStateDraw	= GameStateCageDraw;
GameStateFree	= GameStateCageFree;
GameStateUnload	= GameStateCageUnload;

*/


