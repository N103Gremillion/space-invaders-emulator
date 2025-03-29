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
    XOR, // Logical XOR
    AND, // Logical AND
    OR // Logical OR
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
        void LXI_register(u16* reg); // load next 16 bits in memory into reg specified
        void increment_register(u8* reg, u8* f_reg); // increment given reg and check flags
        void decrement_register(u8* reg, u8* f_reg); // decremtn given reg and check flags
        void DAD_register(u16* hl, u16 reg_pair, u8* flags); // add value in reg to HL reg pair (modifies the carry flag if there is overflow)
        void add_register(u8* a, u8 val, u8* f_reg); // a (accumulator pointer), val (value being added to a) f_reg (flags reg)
        void subtract_register(u8* a, u8 val, u8* f_reg); // a (accumulator pointer), val (value being subtracted from a) f_reg (flags reg)
        void bitwise_AND_register(u8* a, u8 val, u8* f_reg); // a (accumulator pointer), val (value being added to a) f_reg (flags reg)
        void bitwise_XOR_register(u8* a, u8 val, u8* f_reg); // a (accumulator pointer), val (value being added to a) f_reg (flags reg)

    public:
        Registers* regs;
        u8* memory;
        void load_rom(const string& file_path, u16 start_address);
        void run();
        int check_zero_flag(int num); // return value of zero flag 
        int check_sign_flag(u8 num); // for u8 return value of sign flag
        int check_sign_flag(u16 num); // for u16 return value of sign flag
        int check_auxilary_flag(u8 num, u16 res); // return value of aux flag after a given operation
        int check_parity_flag(u16 num); // return value of parity flag 
        int check_carry_flag(u8 num, u8 num2, Operation operation);
        int check_carry_flag(u16 num, u16 num2, Operation operation);
        _8080();
        ~_8080();
};


#endif