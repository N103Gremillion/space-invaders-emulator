#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#define WHITE 0xFFFFFF
#define BLACK 0x000000
#define CARRY_POS 0
#define AUX_POS 4
#define SIGN_POS 7
#define ZERO_POS 6
#define PARITY_POS 2

using u16 = uint16_t;
using u8 = uint8_t;

class Registers{

  public:
    Registers(); 
    TTF_Font* font = nullptr;
    int window_w = 100;
    int window_h = 330;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    void render_regs();
    std::string get_hex_string(int reg_num);

  // note : 8080 is a little edian processor

  // flags and a register which describes state of flags
  union {  
    struct {
      u8 a;
      // 8-bit f(Flag register)
      union {
        struct {
            u8 ca:1;  // Carry
            u8 :1;
            u8 p:1;  // Parity bit
            u8 :1;
            u8 ac:1; // Auxiliary carry 
            u8 :1;   
            u8 z:1;  // Zero bit
            u8 s:1;  // Sign bit
        };
        u8 f;
      };
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

  // d = 2 and e = 3
  union {
    struct {
      u8 e;
      u8 d;
    };
    u16 de = 0x0000;
  };

  // h = 4 and l = 5
  union {
    struct {
      u8 l;
      u8 h;
    };
    u16 hl = 0x0000;
  };

  u16 pc = 0x0000; // program counter
  u16 sp = 0x2400;
};

#endif

