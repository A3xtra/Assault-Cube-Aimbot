#include <math.h>
#include <vector>
#include <algorithm>
#include <TlHelp32.h>
#include <Windows.h>
#include <iomanip>
#include <iostream>
#include "offsets.h"
#include "utils.h"

// key presses, win32 inputdev virtual key codes
#define Number_Pad_9 0x69
#define F6 0x75
#define Right_Mouse 0x02

// Pi for angle calculations
const float PI = 3.14159265358979323846;

// debug mode
bool b_debugEnabled = false;

uintptr_t uint_PID = 0;

uintptr_t uint_ac_client_exe = 0;

HANDLE h_Process = 0;

int entityCount = 0;

struct player
{
	uintptr_t localPlayerPtr;
	int gameState;
	int team; // team ID
	int health; // player health
	float pos[3]; // player pos
	char username[20]; // player username

	// Method for getting player data from the memory
	void ReadMemory()
	{
		// Read memory function that just reads all adresses + offest and stores them in the variable's addresses
		ReadProcessMemory(h_Process, (LPCVOID)(uint_ac_client_exe + playerBaseAddr), &localPlayerPtr, sizeof(localPlayerPtr), 0);
		ReadProcessMemory(h_Process, (LPCVOID)(localPlayerPtr + teamOffset), &team, sizeof(team), 0);
		ReadProcessMemory(h_Process, (LPCVOID)(localPlayerPtr + healthOffset), &health, sizeof(health), 0);
		ReadProcessMemory(h_Process, (LPCVOID)(localPlayerPtr + camPosOffset), &pos, sizeof(pos), 0);
		ReadProcessMemory(h_Process, (LPCVOID)(localPlayerPtr + nameOffset), &username, sizeof(username), 0);

		ReadProcessMemory(h_Process, (LPCVOID)(uint_ac_client_exe + playerCountAddr), &entityCount, sizeof(entityCount), 0);
		// total entities in the game minus 1, player isn't in entity list.
		--entityCount;
	
		ReadProcessMemory(h_Process, (LPCVOID)(uint_ac_client_exe + gameStateAddr), &gameState, sizeof(gameState), 0);
	}
}Player;

int main()
{
	return 0;
}

