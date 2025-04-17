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
#include "log.hpp"

#define TOTAL_BYTES_OF_MEM 65536
#define PROGRAM_START 0X000
#define RAM_START 0x2000
#define MEMORY_END 0x4000
#define INSTRUCTION_CUTTOFF 0x1A90

// input ports
#define INP0 0x00
#define INP1 0x01
#define INP2 0x02
#define SHFT_IN 0x03

// output ports
#define SHFTAMNT 0X02
#define SOUND1 0x03
#define SHFT_DATA 0x04
#define SOUND2 0x05
#define WATCHDOG 0X06

#define HALF_INTERRUPT 0xCF
#define FULL_INTERRUPT 0xD7

#define OVERFLOW 0xFF

using namespace std;

class Screen;

// size is either 8, 16 or 24 depending on the instruction size
string get_hex_string(int num);

// operations
enum Operation {
    ADD,
    SUBTRACT,
    RLC, // rotate left through carry
    XOR, // Logical XOR
    AND, // Logical AND
    OR, // Logical OR
    COMP // comparison
};

// port types
enum PortType {
    OUT,
    IN
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
        bool interrupt_enabled = false;
        bool halted = false;
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
        void bitwise_OR_register(u8* a, u8 val, u8* f_reg); // a (accumulator pointer), val (value being added to a) f_reg (flags reg)
        void compare_register(u8* a, u8 val, u8* f_reg); // The specified byte is compared to the contents of the accumulator. The comparison is performed by internally subtracting the contents of REG from the ac- cumulator (leaving both unchanged) and setting the condi- tion bits according to the result.
        u16 pop_stack(); // get the next 2 bytes from the current location of the sp
        void push_register(u8* first, u8* second); // load in a reg pair onto the stack it expects first and second to the the frist and second reg of theat pair
        void pop_register(u8* first, u8* second); // given 2 bytes that represent a reg pair they are set to the contents of the next 2 bytes in memory referenfced by the sp
        void RET(); // sets the pc = to the top 2 bytes on the stack
        void CALL(u16 memory_address); // (SP-1)<-PC.hi;(SP-2)<-PC.lo;SP<-SP-2;PC=adr
        void JMP(); // jump to next 16 bytes in memory
        void RST(u16 address); // pushes the contents of the pc on the stack and then jumps to a specific memory location specified by the
        void handle_io(u8 port_num, PortType type, u8* a); // given the port number it can excute appropiate interupt
        void handleCPMCall();
        
    public:
        Registers* regs;
        u8* memory;
        void load_rom(const string& file_path, u16 start_address);
        void run();
        void run_test();
        int check_zero_flag(int num); // return value of zero flag 
        int check_sign_flag(u8 num); // for u8 return value of sign flag
        int check_sign_flag(u16 num); // for u16 return value of sign flag
        int check_auxilary_flag(u8 num, u16 res); // return value of aux flag after a given operation
        int check_parity_flag(u16 num); // return value of parity flag 
        int check_carry_flag(u8 initial, u16 result);
        void execute_interrupt(int interupt_type);
        _8080();
        ~_8080();
};


#endif