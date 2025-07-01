#pragma once
#include <cstdint>

uintptr_t GetProcessIdentifier(const wchar_t* exeName);

uintptr_t GetModuleBaseAddress(uintptr_t uint_PID, const wchar_t* modName);