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
bool b_debugEnabled = false;

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

//WriteProcessMemory - [in] HANDLE hProcess, [in] LPVOID lpBaseAddress, [in] LPCVOID lpBuffer, [in] SIZE_T nSize, [out] SIZE_T *lpNumberOfBytesWritten
// hProcess - A handle to the process memory to be modified. The handle must have PROCESS_VM_WRITE and PROCESS_VM_OPERATION access to the process.
// lpBaseAddress - pointer to the base address in the specified process to which data is being written
// nSize - number of bytes being written to the process
// lpNumberOfBytesWritten - a pointer to a variable that receives the number of bytes transferred into the specified process, optional

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

		if (!ReadProcessMemory(h_Process, (LPCVOID)(uint_ac_client_exe + playerCountAddr), &entityCount, sizeof(entityCount), 0)) {
			std::cerr << "[ERROR] Failed to read entity count from memory." << std::endl;
			entityCount = 0;
		}
		else {
			// total entities in the game minus 1, player isn't in entity list.

			--entityCount;
		}

	
		ReadProcessMemory(h_Process, (LPCVOID)(uint_ac_client_exe + gameStateAddr), &gameState, sizeof(gameState), 0);
		//std::cout << localPlayerPtr;

	}
}Player;

struct entityList
{
	uintptr_t entityBasePtr = 0;
	int team = 0; // team ID
	int health = 0; // player health
	float pos[3] = { 0,0,0 }; // player pos
	float aimbotAngle[3] = { 0,0,0 };
	char username[20]; // player username


	// Read entity info from memory, loop through entity list
	void ReadMemory(int entityNum)
	{
		entityNum++;
		// Read memory function that just reads all adresses + offest and stores them in the variable's addresses
		ReadProcessMemory(h_Process, (LPCVOID)(uint_ac_client_exe + entListBaseAddr), &entityBasePtr, sizeof(entityBasePtr), 0);
		ReadProcessMemory(h_Process, (LPCVOID)(entityBasePtr+(entityNum*entStep)), &entityBasePtr, sizeof(entityBasePtr), 0);
		ReadProcessMemory(h_Process, (LPCVOID)(entityBasePtr + teamOffset), &team, sizeof(team), 0);
		ReadProcessMemory(h_Process, (LPCVOID)(entityBasePtr + healthOffset), &health, sizeof(health), 0);
		ReadProcessMemory(h_Process, (LPCVOID)(entityBasePtr + camPosOffset), &pos, sizeof(pos), 0);
		ReadProcessMemory(h_Process, (LPCVOID)(entityBasePtr + nameOffset), &username, sizeof(username), 0);

	}
};


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
	//std::cout << "deltaX = " << deltaX << std::endl;
	//std::cout << "deltaY = " << deltaY << std::endl;
	//std::cout << "deltaZ = " << deltaZ << std::endl;
	//std::cout << "Hypotenuse = " << hypotenuse << std::endl;
	//std::cout << "Yaw = " << Angles[0] << std::endl;
	//std::cout << "Pitch = " << Angles[1] << std::endl;
	//std::cout << "Player pos: (" << Player.pos[0] << ", " << Player.pos[1] << ", " << Player.pos[2] << ")" << std::endl;

	//WriteProcessMemory(h_Process, (BYTE*)(Player.localPlayerPtr + rotationOffset), &Angles, sizeof(Angles), 0); // writes both yaw and pitch
}



void CalculateAngle(float* Player, float* Enemy, float* Angles)
{
	// diff in x,y,z coords between target and player.
	float deltaX = Enemy[0] - Player[0];
	float deltaY = Enemy[1] - Player[1];
	float deltaZ = Enemy[2] - Player[2];

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
}

// list of targets, distance for each, angle for each
struct targetList
{
	float distance = 0; // distance from player to target;
	float aimAngles[3] = { 0,0,0 }; // angles for aiming
	char* name = 0; // name of enemy aimed at
	int health = 0; // enemy target health
	int team = 0; // enemy target team ID
	uintptr_t entityBasePtr = 0; // pointer to target's memory location

	targetList() {} // empty constructor

	// initalising constructor
	targetList(float aimbotAngles[], float playerCoords[], float enemyCoords[], char enemyName[], int enemyHealth, int enemyTeam, uintptr_t enemyEntityBasePtr)
	{
		// calculate distance for target using 3d pythag
		distance = Get3dDistance(playerCoords[0], playerCoords[1], playerCoords[2], enemyCoords[0], enemyCoords[1], enemyCoords[2]);

		aimAngles[0] = aimbotAngles[0];
		aimAngles[1] = aimbotAngles[1];
		aimAngles[2] = aimbotAngles[2];
		name = enemyName;
		health = enemyHealth;
		team = enemyTeam;
		entityBasePtr = enemyEntityBasePtr;
	}

	float Get3dDistance(float myCoordsX, float myCoordsY, float myCoordsZ, float enemyCoordsX, float enemyCoordsY, float enemyCoordsZ)
	{
		return sqrt(pow(float(enemyCoordsX - myCoordsX), 2) + pow(float(enemyCoordsY - myCoordsY), 2) + pow(float(enemyCoordsZ - myCoordsZ), 2));
	}
};


void Aimbot()
{
	Player.ReadMemory();

	// dynamically allocate memory for target list and entity list
	if (entityCount <= 0) 
	{
		std::cout << "Invalid entity count: " << entityCount << std::endl;
		return;
	}
	targetList* targetListPtr = new targetList[entityCount]; // raw pointer, array, when using 'new' delete after using it to prevent memory leaks
	std::vector<entityList> entityList(entityCount); // smart pointer, vector

	int targetLoop = 0;

	for (int i = 0; i < entityCount; i++)
	{
		entityList[i].ReadMemory(i);

		if (entityList[i].health <= 0) continue;

		// gamemode IDs

		if (Player.gameState == 7 && entityList[i].team == Player.team) continue;
		if (Player.gameState == 20 && entityList[i].team == Player.team) continue;
		if (Player.gameState == 21 && entityList[i].team == Player.team) continue;

		CalculateAngle(Player.pos, entityList[i].pos, entityList[i].aimbotAngle);

		targetListPtr[targetLoop] = targetList(
			entityList[i].aimbotAngle,
			Player.pos,
			entityList[i].pos,
			entityList[i].username,
			entityList[i].health,
			entityList[i].team,
			entityList[i].entityBasePtr
		);

		targetLoop++;
	}

	if (targetLoop == 0)
	{
		system("CLS");
		printf("No valid targets found. \n");
	}

	if (targetLoop > 0)
	{
		std::sort(
			targetListPtr, targetListPtr + targetLoop,
			[](const targetList& a, const targetList& b)
			{
				return a.distance < b.distance;
			});

		if (b_debugEnabled)
		{
			printf("Aimbot always on, hold right mouse to disable aimbot, F6 to quit. \n");
			printf("Num pad 9 to toggle debug output. \n");
			std::cout << "Debug: " << (b_debugEnabled == true ? "ON\n" : "OFF\n") << std::endl;

			std::cout << (
				Player.gameState == 0 ? "Default match" :
				Player.gameState == 7 ? "TDM / Survival" :
				Player.gameState == 8 ? "Deathmatch" :
				Player.gameState == 12 ? "One shot, one kill" :
				Player.gameState == 18 ? "Pistol frenzy" :
				Player.gameState == 19 ? "LMS" :
				Player.gameState == 20 ? "Team survivor" :
				Player.gameState == 21 ? "Team one shot, one kill" :
				"Unknown game mode") << " - Number of entities: " << entityCount << " - PIDwwwwww: " << uint_PID << " - targetLoop: " << targetLoop << "\n" << std::endl;

			std::string alignment = "";

			if (Player.health <= -1 && Player.health >= -9) { alignment = "          "; }
			if (Player.health <= -10 && Player.health >= -99) { alignment = "          "; }
			if (Player.health <= 0 && Player.health >= 9) { alignment = "          "; }
			if (Player.health <= 10 && Player.health >= 99) { alignment = "          "; }
			if (Player.health >= 100) { alignment = "          "; }

			// display player info
			std::cout << alignment << "                   " << Player.health << "                   " << Player.team << "     " << Player.username << std::endl;

			std::cout << "Number - Ent address - health - distance - team - name\n";
			int iNum = 0;
			for (int i = 0; i < targetLoop; i++)
			{
				++iNum;

				std::cout << (iNum < 10 ? " " : "") << iNum
					<< "      0x" << std::hex << std::setw(8) << std::setfill('0') << targetListPtr[i].entityBasePtr << std::dec
					<< "     " << (targetListPtr[i].health < 10 ? " " : "") << (targetListPtr[i].health < 100 ? " " : "") << targetListPtr[i].health << "             "
					<< (static_cast<int>(targetListPtr[i].distance) < 10 ? " " : "") << (static_cast<int>(targetListPtr[i].distance) < 100 ? " " : "") << static_cast<int>(targetListPtr[i].distance)
					<< (targetListPtr[i].team < 10 ? " " : "") << targetListPtr[i].team << "   "
					<< targetListPtr[i].name 
					<< std::endl;
			}
		}

		if (!GetAsyncKeyState(Right_Mouse) && targetListPtr[0].distance < 20)
		{

			WriteProcessMemory(h_Process, (BYTE*)(Player.localPlayerPtr + rotationOffset), targetListPtr[0].aimAngles, sizeof(targetListPtr[0].aimAngles), 0);
		}

	}

	//clean up dynamically allocated memory
	targetLoop = 0;
	delete[] targetListPtr;
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
		printf("F1 - health, F2 - ammo, F6 to quit, F9 to debug. \n");
		std::cout << "Debug = " << (b_debugEnabled == true ? "ON" : "OFF") << std::endl;

	
		// F9 KEY
		if(GetAsyncKeyState(0x78) & 0x8000)
		{ 
			b_debugEnabled = !b_debugEnabled;
			Sleep(100);
		}
		// F1 - health
		if (GetAsyncKeyState(0x70) & 0x8000)
		{
			int newHealth = 9999; // amount to be written to the pointer
			//writes to the health address the amount at the address of newHealth
			WriteProcessMemory(h_Process, (BYTE*)(Player.localPlayerPtr + healthOffset), &newHealth, sizeof(newHealth), 0);
		}
		//F2 - ammo
		if (GetAsyncKeyState(0x71) & 0x8000)
		{
			int newAmmo = 9999; // amount to be written to the pointer
			//writes to the ammo addresses the amount at the address of newAmmo

			WriteProcessMemory(h_Process, (BYTE*)(Player.localPlayerPtr + mgAmmoOffset), &newAmmo, sizeof(newAmmo), 0);
			WriteProcessMemory(h_Process, (BYTE*)(Player.localPlayerPtr + mgAmmoOffset), &newAmmo, sizeof(newAmmo), 0);
			WriteProcessMemory(h_Process, (BYTE*)(Player.localPlayerPtr + carbineAmmoOffset), &newAmmo, sizeof(newAmmo), 0);
			WriteProcessMemory(h_Process, (BYTE*)(Player.localPlayerPtr + shotgunAmmoOffset), &newAmmo, sizeof(newAmmo), 0);
			WriteProcessMemory(h_Process, (BYTE*)(Player.localPlayerPtr + sniperAmmoOffset), &newAmmo, sizeof(newAmmo), 0);
			WriteProcessMemory(h_Process, (BYTE*)(Player.localPlayerPtr + arifleAmmoOffset), &newAmmo, sizeof(newAmmo), 0);
			WriteProcessMemory(h_Process, (BYTE*)(Player.localPlayerPtr + pistolAmmoOffset), &newAmmo, sizeof(newAmmo), 0);
		}
		Aimbot();
		Sleep(1);
	}
	CloseHandle(h_Process);
	return 0;
}


