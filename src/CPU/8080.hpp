#ifndef _8080_HPP
#define _8080_HPP

#include <iostream>
#include <SDL2/SDL.h>
#include <array>
#include "keys.hpp"
#include "Screen.hpp"
#include "Registers.hpp"

#define TOTAL_BYTES_OF_RAM 65536

using namespace std;

class _8080 {
    private:
        Screen* screen;
        u8* ram;
    public:
        Registers* regs;
        void run();
        _8080();
        ~_8080();
};


#endif