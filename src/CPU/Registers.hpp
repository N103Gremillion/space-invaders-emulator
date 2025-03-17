#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include <cstdint>

using u16 = uint16_t;
using u8 = uint8_t;

class Registers{

  public:
    Registers();
  // b = 0 and c = 1
  union {
    struct {
      u8 b;
      u8 c;
    };
    u16 bc;
  };

  // d = 2 and e = 3
  union {
    struct {
      u8 d;
      u8 e;
    };
    u16 de;
  };

  // h = 4 and l = 5
  union {
    struct {
      u8 h;
      u8 l;
    };
    u16 hl;
  };
  // accumulator (a = 7) describes state fo machine flags
  u8 a;
  u16 pc; // program counter
  u16 sp;
};

#endif

