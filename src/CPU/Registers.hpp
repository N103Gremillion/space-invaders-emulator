#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include <cstdint>

using u16 = uint16_t;
using u8 = uint8_t;

class Registers{

  public:
    Registers();

  // note : 8080 is a little edian processor

  // flags and a register which describes state of flags
  union {  
    struct {
      u8 a;
      // 8-bit f(Flag register)
      union {
        struct {
            u8 c:1;  // Carry
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
    u16 PSW;
  };
  
  union {
    struct {
      u8 c;
      u8 b;
    };
    u16 bc;
  };

  // d = 2 and e = 3
  union {
    struct {
      u8 e;
      u8 d;
    };
    u16 de;
  };

  // h = 4 and l = 5
  union {
    struct {
      u8 l;
      u8 h;
    };
    u16 hl;
  };

  u16 pc; // program counter
  u16 sp;
};

#endif

