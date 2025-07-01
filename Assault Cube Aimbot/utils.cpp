#include <iostream>
#include "utils.h"
#include <Windows.h>
#include <TlHelp32.h>

// https://learn.microsoft.com/en-us/windows/win32/procthread/processes-and-threads
// Process - A process, in the simplest terms, is an executing program. 

// Handle - Properly, in Windows, (and generally in computing) a handle is an abstraction which hides a real memory address from the API user, allowing the system to reorganize physical memory transparently to the program.
// Resolving a handle into a pointer locks the memory, and releasing the handle invalidates the pointer.

// Snapshot - CreateToolhelp32Snapshot from the tlhelp32.h library, is a read-only, point-in-time copy of system data structures like processes, threads, modules, and heaps. 

// DWORD - Allocates / initalizes a double word (4 bytes) of storage for each initalizer.
// dwSize - Size of the structure in bytes.
// th32ProcessID - Process identifier of the process to be included in the snapshot.

// TH32CS_SNAPPROCESS - all proecesses in the system in the snapshot.

// PROCESSENTRY32  DWORD dwSize, 0, th32ProcessID, dwFlags=0, szExeFile

// Process32First - Retrieves information about the first process encountered in a system snapshot.
// Process32First - Returns TRUE if the first entry of the process list has been copied to the buffer, FALSE otherwise.

// wcscmp - Compares two null-terminated wide strings lexicographically.
// Negative value if lhs appears before rhs in lexicographical order.
// Zero if lhsand rhs compare equal.
// Positive value if lhs appears after rhs in lexicographical order.


// szExeFile[MAXPATH] - name of the exe for the process

uintptr_t GetProcessIdentifier(const wchar_t* exeName)
{
	PROCESSENTRY32 ProcessEntry = { 0 };

	// Creates a snapshot of all processes
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	
	// If the snapshot wasn't created successfully return 0
	if (!Snapshot) return 0;

	// Initalize the dwSize member - If you do not initialize dwSize, Process32First fails.
	ProcessEntry.dwSize = sizeof(ProcessEntry);

	// pointer to PROCESSENTRY32 (containing name of exe, process ID, pr ocess ID of parent process))
	// if first entry of processEntry not been copied to the buffer close the handle
	if (!Process32First(Snapshot, &ProcessEntry)) 
	{
		CloseHandle(Snapshot);
		return 0;
	}

	// loop through processes
	do
	{
		// compares the name of the exe for the process with the name,
		// basically if lhs not after rhs in lexicographical order
		if (!wcscmp(ProcessEntry.szExeFile, exeName))
		{
			CloseHandle(Snapshot);
			// return the process ID
			return ProcessEntry.th32ProcessID;
		}
	} while (Process32Next(Snapshot, &ProcessEntry));

	// Clean up handle if no match found
	CloseHandle(Snapshot);
	return 0;
}

// TH32CS_SNAPMODULE - all modules of the process specified in th32ProcessID. Includes 64 bit if in a 64 bit module
// TH32CS_SNAPMODULE32 - to include the 32 bit modules of the process from a 64 bit process use TH32CS_SNAPMODULE32 flag


uintptr_t GetModuleBaseAddress(uintptr_t uint_PID, const wchar_t* modName)
{
	// var for the module base addr to be stored in
	uintptr_t modBaseAddr = 0;

	// Take snapshot (all processes, etc.) 
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, uint_PID);

	// if the CreateToolhelp32Snapshot didn't fail
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry); // set size of modEntry

		// if the first entry of the module list has been copied to the buffer
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				// compare module name with target module name case-insensitive
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					// if match is found store the module's base addr
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} 
			// move to next module in hSnap
			while (Module32Next(hSnap, &modEntry));
		}
	}

	// close handle 
	CloseHandle(hSnap);
	return modBaseAddr;
}

