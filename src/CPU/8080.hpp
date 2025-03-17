#ifndef _8080_HPP
#define _8080_HPP

#include <iostream>
#include <SDL2/SDL.h>
#include <array>
#include <fstream>
#include <vector>
#include "keys.hpp"
#include "Screen.hpp"
#include "Registers.hpp"

#define TOTAL_BYTES_OF_MEM 65536
#define PROGRAM_START 0X000
#define RAM_START 0x2000
#define VRAM_START 0X2400
#define MEMORY_END 0x4000

using namespace std;

class _8080 {
    private:
        Screen* screen;
    public:
        Registers* regs;
        u8* memory;
        void load_rom(const string& file_path, u16 start_address);
        void run();
        _8080();
        ~_8080();
};


#endif