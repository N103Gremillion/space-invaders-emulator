# Space_Invaders Emulator

A C++ emulator for the original **Space Invaders** arcade game, built using **SDL2**, **SDL2_ttf**, and **CMake**. This project aims to recreate the classic Space Invaders experience as faithfully as possible, complete with keyboard input and original game behavior.

---

## 🕹️ Features

- Emulates the original Space Invaders arcade ROM.
- SDL2-based rendering and input handling.
- Keyboard input support for arcade-style controls.
- Basic TTF font support using SDL2_ttf.
- Clean build system using CMake and a `run.sh` script.
- Passes the two small CPU tests and some of the larger ones.

---

## 🔧 Requirements

- C++17 or higher
- SDL2
- SDL2_ttf
- CMake (version 3.10+ recommended)
- A compatible Space Invaders ROM (not included)

---

## 🚀 Building & Running

Clone the repository and make sure the original **Space Invaders ROM files** (`invaders.e`, `invaders.f`, `invaders.g`, and `invaders.h`) are placed inside a folder named `invaders` in the root directory:

```bash
./run.sh

Space_Invaders_Emulator/
├── src/
├── include/
├── run.sh
├── CMakeLists.txt
├── README.md
└── assets/ (fonts, optional)

🙏 Credits
TheAssembler1 – for the logging library used in this project.
Space Invaders ROM and hardware documentation from various emulator resources.

