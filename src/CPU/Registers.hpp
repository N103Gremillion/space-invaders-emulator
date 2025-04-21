#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <iomanip>

#define WHITE 0xFFFFFF
#define BLACK 0x000000

#define SIGN_POS 7
#define ZERO_POS 6
#define AUX_POS 4
#define PARITY_POS 2
#define CARRY_POS 0

using u16 = uint16_t;
using u8 = uint8_t;

class Registers {

public:
    Registers(); 

    // SDL-related members
    TTF_Font* font = nullptr;
    int window_w = 100;
    int window_h = 330;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    void render_regs();
    void set_flag(int flag_distance);
    void reset_flag(int flag_distance);
    bool check_flag(int flag_distance); 
    int get_flag(int flag_distance);
    std::string get_hex_string(int reg_num);

    // 8080 Registers (with static anonymous unions)
    union {  
      struct {
        u8 a;
        u8 f;
      };
      u16 PSW = 0x0000;
    };

    union {
      struct {
        u8 c;
        u8 b;
      };
      u16 bc = 0x0000;
    };

    union {
      struct {
        u8 e;
        u8 d;
      };
      u16 de = 0x0000;
    };

    union {
      struct {
        u8 l;
        u8 h;
      };
      u16 hl = 0x0000;
    };

    u16 pc = 0x0000; // program counter
    u16 sp = 0x2400; // stack pointer
};

#endif
