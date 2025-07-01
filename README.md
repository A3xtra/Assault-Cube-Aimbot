# External Aimbot for AssaultCube (Windows)

This project is a basic external aimbot written in C++ for the open-source game [AssaultCube](https://assault.cubers.net/). It reads and manipulates memory using Windows APIs to detect enemy entities and automatically calculate aim angles.

---

## How It Works

- Reads memory of the `ac_client.exe` process.
- Extracts the local player and enemy data: position, health, team, etc.
- Calculates the angles (pitch & yaw) needed to aim at the nearest target using trigonometry.
- (Optionally) writes the aim angles to the game memory to lock onto targets.

---

## Code Structure

### Process & Module Access

- `GetProcessIdentifier`: Locates `ac_client.exe` and returns its process ID.
- `GetModuleBaseAddress`: Retrieves the base address of the module in memory.

### Entity Structures

- `struct player`: Stores local player data like position, team, health, name.
- `struct entityList`: Stores enemy entities' data (team, health, position, name).
- `struct targetList`: Prepares target data for aimbot (distance, angles, etc).

## Controls

| Key                  | Function             |
| -------------------- | -------------------- |
| `F1`                 | 9999 health          |
| `F2`                 | 9999 all ammo        |
| `F6`                 | Quit the program     |
| `Numpad 9`           | Toggle debug info    |
| `Right Mouse Button` | Toggle aimbot on/off |

---

## Debug Output

When enabled, prints live data to the console:

- Local player health, position, and username
- Number of detected entities
- Target list sorted by distance with team/health info

---

## Notes & Considerations

- Only works on **Windows**.
- Code assumes target is `ac_client.exe` (AssaultCube).

---

## File Breakdown

| File        | Purpose                                                          |
| ----------- | ---------------------------------------------------------------- |
| `main.cpp`  | Core logic: memory reading, target detection, angle calculation  |
| `utils.h`   | Helper functions and constants (e.g., offsets, math helpers)     |
| `offsets.h` | Contains static offsets used to access in-game memory structures |
