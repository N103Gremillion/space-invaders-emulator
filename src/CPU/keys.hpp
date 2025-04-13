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
#define I 105
#define LEFT_ARROW 1073741904
#define RIGHT_ARROW 1073741903

// new left and right arrow indexes to prevent having to make a huge array
#define LEFT_ARROW_INDEX 0
#define RIGHT_ARROW_INDEX 1

#define A_KEY 0x0
#define D_KEY 0x1
#define SPACE_KEY 0x2
#define INSERT_COIN 0x3 // (I)
#define NUM_INPUTS 0x4 // len of inputs

extern bool keys[116];
extern bool inputs[NUM_INPUTS];

void handle_key_press(int keycode);
void handle_key_release(int keycode);

#endif