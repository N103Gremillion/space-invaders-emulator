#ifndef _8080_HPP
#define _8080_HPP

#include <iostream>
#include <SDL2/SDL.h>
#include <array>
#include "keys.hpp"
#include "screen.hpp"

using namespace std;

class _8080 {
    private:
        Screen* screen;
    public:
        void run();
        _8080();
        ~_8080();
};


#endif