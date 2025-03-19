#ifndef _8080_HPP
#define _8080_HPP

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <array>
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

// size is either 8, 16 or 24 depending on the instruction size
string get_hex_string(int num);

class _8080 {
    private:
        // screen is the game screen
        Screen* screen;
        // window holds info about instructions that are running
        int window_w = 150;
        int window_h = 700;
        SDL_Window* window; 
        SDL_Renderer* renderer;
        TTF_Font* font;
        void render();
        void fill_background();
        void draw_instructions();
    public:
        Registers* regs;
        u8* memory;
        void load_rom(const string& file_path, u16 start_address);
        void run();
        _8080();
        ~_8080();
};


#endif