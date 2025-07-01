#include <math.h>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include <TlHelp32.h>
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
bool b_debugEnabled = true;

uintptr_t uint_PID = 0;

uintptr_t uint_ac_client_exe = 0;

HANDLE h_Process = 0;

int entityCount = 0;


//ReadProcessMemory -   [in]  HANDLE  hProcess, [in]  LPCVOID lpBaseAddress, [out] LPVOID  lpBuffer, [in]  SIZE_T  nSize, [out] SIZE_T* lpNumberOfBytesRead
// hProcess - handle to process with memory being read, 
// LPCVOID lpBaseAddress - long pointer to base addr in a specified process to be read,
// lpBuffer - buffer variable to store the value in
// nSize - number of bytes to read from specified process
// lpNumberOfBytesRead - A pointer to a variable that receives the number of bytes transferred into the specified buffer. If lpNumberOfBytesRead is NULL, the parameter is ignored.

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
		//std::cout << localPlayerPtr;

	}
}Player;

void DebugCalculateAngle()
{
	Player.ReadMemory();

	float Enemy[3] = { 120,132,11.5 };
	float Angles[2] = { 0,0 };

	// diff in x,y,z coords between target and player.
	float deltaX = Enemy[0] - Player.pos[0];
	float deltaY = Enemy[1] - Player.pos[1];
	float deltaZ = Enemy[2] - Player.pos[2];

	// 3d hypotenuse
	float hypotenuse = sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);

	// trig: tan o/a, sin o/h, cos a/h

	// Calculate the yaw (vertical angle, left and right) using arctangent2 function
	//atan2f(y,x) represents an angle in a right triangle, y is the opposite side.
	// x is the adjacent side.
	// so function is atan2f(opposite,adjacent)
	Angles[0] = (atan2f(deltaY, deltaX));

	Angles[0] *= (180 / PI); // convert radians to degrees

	Angles[0] += 90.0f; // Add 90 degrees to yaw, because of AC's coord system

	// calculate pitch using arcsin
	// pitch angle is arcsin(opp/hyp)
	Angles[1] = (asinf(deltaZ / hypotenuse));
	Angles[1] *= (180 / PI);

	// clearing console for continuous updates
	system("CLS");
	std::cout << "deltaX = " << deltaX << std::endl;
	std::cout << "deltaY = " << deltaY << std::endl;
	std::cout << "deltaZ = " << deltaZ << std::endl;
	std::cout << "Hypotenuse = " << hypotenuse << std::endl;
	std::cout << "Yaw = " << Angles[0] << std::endl;
	std::cout << "Pitch = " << Angles[1] << std::endl;
	std::cout << "Player pos: (" << Player.pos[0] << ", " << Player.pos[1] << ", " << Player.pos[2] << ")" << std::endl;

	WriteProcessMemory(h_Process, (BYTE*)(Player.localPlayerPtr + rotationOffset), &Angles, sizeof(Angles), 0); // writes both yaw and pitch
}

int main()
{
	// get pID of "ac_client.exe"
	uint_PID = GetProcessIdentifier(L"ac_client.exe");

	// get base addr of ac_client in memory
	uint_ac_client_exe = GetModuleBaseAddress(uint_PID, L"ac_client.exe");

	std::cout << uint_ac_client_exe << std::endl;
	// open handle to the game process, full access
	h_Process = OpenProcess(PROCESS_ALL_ACCESS, NULL, uint_PID);

	// bring game to foreground
	SetForegroundWindow(FindWindow(0, L"AssaultCube"));


	while (!GetAsyncKeyState(F6))
	{
		system("CLS");
		printf("F6 to quit, num pad 9 to debug. \n");
		std::cout << "Debug = " << (b_debugEnabled == true ? "ON" : "OFF") << std::endl;

	
		if(GetAsyncKeyState(Number_Pad_9) & 0x8000)
		{ 
			b_debugEnabled = !b_debugEnabled;
			Sleep(100);
		}
		if (b_debugEnabled)
		{
			DebugCalculateAngle();
		}

	}
	CloseHandle(h_Process);
	return 0;
}

