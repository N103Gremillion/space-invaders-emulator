#ifndef KEYS_HPP
#define KEYS_HPP

#include <iostream>

using namespace std;

// opcodes for each key
#define ESC 27
#define SPACE 32
#define _1 49
#define _2 50
#define A 97
#define C 99
#define D 100
#define P 112
#define S 115
#define LEFT_ARROW 1073741904
#define RIGHT_ARROW 1073741903

// new left and right arrow indexes to prevent having to make a huge array
#define LEFT_ARROW_INDEX 0
#define RIGHT_ARROW_INDEX 1

extern bool keys[116];

void handle_key_press(int keycode);
void handle_key_release(int keycode);

#endif