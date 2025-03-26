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
#include "instruction_list.hpp"

#define TOTAL_BYTES_OF_MEM 65536
#define PROGRAM_START 0X000
#define RAM_START 0x2000
#define VRAM_START 0X2400
#define MEMORY_END 0x4000

using namespace std;

// size is either 8, 16 or 24 depending on the instruction size
string get_hex_string(int num);

// operations
enum Operation {
    ADD,
    SUBTRACT,
    RLC, // rotate left through carry
    AND, // Logical AND
    OR, // Logical OR
    XOR // Logical XOR
};

class _8080 {
    private:
        // screen is the game screen
        Screen* screen;
        // window holds info about instructions that are running
        int window_w = 150;
        int window_h = 700;
        int cycles = 0;
        SDL_Window* window; 
        SDL_Renderer* renderer;
        TTF_Font* font;
        void render();
        void fill_background();
        void draw_instructions();
        u8 fetch_byte(); // fetch bytes
        u16 fetch_bytes(); // fetch next 2 bytes
        void execute_instruction(u8 opcode);

    public:
        Registers* regs;
        u8* memory;
        void load_rom(const string& file_path, u16 start_address);
        void run();
        int check_zero_flag(int num); // return value of zero flag 
        int check_sign_flag(u8 num); // for u8 return value of sign flag
        int check_sign_flag(u16 num); // for u16 return value of sign flag
        int check_auxilary_flag(u8 num, u8 num2, Operation operation); // return value of aux flag after a given operation
        int check_parity_flag(u16 num); // return value of parity flag 
        int check_carry_flag(u8 num, u8 num2, Operation operation);
        int check_carry_flag(u16 num, u16 num2, Operation operation);
        _8080();
        ~_8080();
};


#endif